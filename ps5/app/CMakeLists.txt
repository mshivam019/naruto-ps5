set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-march=x86-64-v2)

# Private generated sources are supplied locally, never committed to the port repo.
set(NARUTO_GENERATED "${CMAKE_CURRENT_LIST_DIR}/../../private/codegen/generated" CACHE PATH "Private codegen output")
set(NARUTO_UPSTREAM "${CMAKE_CURRENT_LIST_DIR}/../../upstream/naruto-rise-recomp")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/../tools/prepare-static-module.py"
    --generated "${NARUTO_GENERATED}" --output "${CMAKE_CURRENT_BINARY_DIR}/private-generated"
    COMMAND_ERROR_IS_FATAL ANY)
foreach(module default ai2c)
    set(module_dir "${CMAKE_CURRENT_BINARY_DIR}/private-generated/${module}")
    if(NOT EXISTS "${module_dir}/sources.cmake")
        message(FATAL_ERROR "Generate private Naruto ${module} code before configuring")
    endif()
    include("${module_dir}/sources.cmake")
    add_library(naruto_${module} STATIC ${GENERATED_SOURCES})
    target_link_libraries(naruto_${module} PRIVATE rex::runtime)
    target_include_directories(naruto_${module} PRIVATE "${module_dir}")
    target_compile_options(naruto_${module} PRIVATE -O2 -g0 -fno-strict-aliasing -ffp-model=strict)
    target_precompile_headers(naruto_${module} PRIVATE "${module_dir}/narutorise_pch.h")
endforeach()
target_compile_definitions(naruto_ai2c PRIVATE
    ReXModule_Register=NarutoAI2C_Register
    ReXModule_GetImageInfo=NarutoAI2C_GetImageInfo
    PPCImageConfig=NarutoAI2C_ImageConfig
    PPCFuncMappings=NarutoAI2C_FuncMappings)
add_library(naruto_app STATIC main.cpp builtin-module.cpp input.cpp native/platform.c
    "${NARUTO_UPSTREAM}/sdk/src/ui/rex_app.cpp")
target_link_libraries(naruto_app PRIVATE naruto_ps5_runtime imgui)
target_include_directories(naruto_app PRIVATE "${NARUTO_GENERATED}/default")
target_compile_definitions(naruto_app PRIVATE REXGLUE_BUILD_CONFIG="Release")
target_compile_options(naruto_app PRIVATE -O2 -g0)
