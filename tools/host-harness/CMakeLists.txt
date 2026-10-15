# Linux profiling harness for the Naruto PS5 port. Builds the patched SDK and the
# private generated game code for the host (never reruns codegen) together with
# the PS5 app's guest hooks. Outputs contain recompiled game code: never publish.
#
#   cmake -S tools/host-harness -B build/host-harness -G Ninja \
#       -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang \
#       -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_LINKER_TYPE=LLD
#   cmake --build build/host-harness
cmake_minimum_required(VERSION 3.25)
project(narutorise LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(SDK "${ROOT}/upstream/naruto-rise-recomp/sdk")
set(APP_SRC "${ROOT}/upstream/naruto-rise-recomp/app/src")
set(GENERATED "${ROOT}/build/setup/codegen/generated" CACHE PATH "Generated code from ps5/tools/setup.py")

set(REXGLUE_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(REXGLUE_ENABLE_TRACY OFF CACHE BOOL "" FORCE)
# The PS5 clang target defaults to Zen 2; match its instruction set and -O3.
# Frame pointers keep perf call graphs usable.
add_compile_options(-march=znver2 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer)
# Periodic GPU-thread stage timings, as logged on PS5.
add_compile_definitions(REX_GPU_STAGE_STATS_HOST=1)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG" CACHE STRING "" FORCE)
add_subdirectory("${SDK}" rexglue-sdk)

# The SDK writes to sdk/out/<platform>; keep harness outputs in this build tree.
set(HARNESS_OUT "${CMAKE_BINARY_DIR}/out")
function(redirect_outputs dir)
    get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(t IN LISTS targets)
        get_target_property(type ${t} TYPE)
        if(NOT type STREQUAL "INTERFACE_LIBRARY" AND NOT type STREQUAL "UTILITY")
            set_target_properties(${t} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${HARNESS_OUT}"
                LIBRARY_OUTPUT_DIRECTORY "${HARNESS_OUT}"
                ARCHIVE_OUTPUT_DIRECTORY "${HARNESS_OUT}")
        endif()
    endforeach()
    get_property(children DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(child IN LISTS children)
        redirect_outputs("${child}")
    endforeach()
endfunction()
redirect_outputs("${SDK}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${HARNESS_OUT}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${HARNESS_OUT}")
set(REXGLUE_HOST_TARGET narutorise)
set(RECOMP_OPTIONS -O2 -gline-tables-only -fno-strict-aliasing -ffp-model=strict)

if(NOT EXISTS "${GENERATED}/default/sources.cmake")
    message(FATAL_ERROR "Generate the private Naruto code first (see README)")
endif()
function(generated_sources module out_var)
    include("${GENERATED}/${module}/sources.cmake")
    set(sources "")
    foreach(source IN LISTS GENERATED_SOURCES)
        if(IS_ABSOLUTE "${source}")
            list(APPEND sources "${source}")
        else()
            list(APPEND sources "${GENERATED}/${source}")
        endif()
    endforeach()
    set(${out_var} ${sources} PARENT_SCOPE)
endfunction()
generated_sources(default DEFAULT_SOURCES)
generated_sources(ai2c AI2C_SOURCES)

add_executable(narutorise
    "${APP_SRC}/main.cpp" "${APP_SRC}/intro_skip.cpp" "${APP_SRC}/ultrawide.cpp"
    "${ROOT}/ps5/app/button_prompts.cpp" "${ROOT}/ps5/app/save_picture.cpp"
    "${ROOT}/ps5/app/loader_thread.cpp"
    harness_probe.cpp)
target_link_libraries(narutorise PRIVATE imgui)
add_library(narutorise_recomp OBJECT ${DEFAULT_SOURCES})
target_include_directories(narutorise_recomp PRIVATE "${GENERATED}/.." "${GENERATED}/default" "${APP_SRC}")
target_link_libraries(narutorise_recomp PRIVATE rex::runtime)
rexglue_apply_target_settings(narutorise_recomp)
target_precompile_headers(narutorise_recomp PRIVATE "${GENERATED}/default/narutorise_pch.h")
target_compile_options(narutorise_recomp PRIVATE ${RECOMP_OPTIONS})
# button_prompts.cpp registers a texture hook in the GPU library (a plugin on Linux).
target_link_libraries(narutorise PRIVATE narutorise_recomp rex::runtime rex::gpu-xenos)
target_include_directories(narutorise PRIVATE "${GENERATED}/.." "${GENERATED}/default" "${APP_SRC}")
rexglue_configure_target(narutorise GPU_PLUGINS xenos)

add_library(narutorise_AI2C SHARED ${AI2C_SOURCES})
target_include_directories(narutorise_AI2C PRIVATE "${GENERATED}/.." "${GENERATED}/ai2c")
target_link_libraries(narutorise_AI2C PRIVATE rex::runtime)
target_precompile_headers(narutorise_AI2C PRIVATE "${GENERATED}/ai2c/narutorise_pch.h")
target_compile_options(narutorise_AI2C PRIVATE ${RECOMP_OPTIONS})
set_target_properties(narutorise_AI2C PROPERTIES CXX_VISIBILITY_PRESET hidden)
rexglue_configure_module_target(narutorise_AI2C HOST narutorise)

# The runtime dlopens guest modules by their bare shared_lib_name.
add_custom_command(TARGET narutorise_AI2C POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink libnarutorise_AI2C.so "${HARNESS_OUT}/narutorise_AI2C"
    VERBATIM)
configure_file(narutorise.toml "${HARNESS_OUT}/narutorise.toml" COPYONLY)
