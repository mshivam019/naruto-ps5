# Private generated sources are supplied locally, never committed to the port repo.
set(NARUTO_GENERATED "${CMAKE_CURRENT_LIST_DIR}/../../private/codegen/generated" CACHE PATH "Private codegen output")
set(NARUTO_UPSTREAM "${CMAKE_CURRENT_LIST_DIR}/../../upstream/naruto-rise-recomp")
foreach(module default ai2c)
    if(NOT EXISTS "${NARUTO_GENERATED}/${module}/sources.cmake")
        message(FATAL_ERROR "Generate private Naruto ${module} code before configuring")
    endif()
    include("${NARUTO_GENERATED}/${module}/sources.cmake")
    add_library(naruto_${module} STATIC ${GENERATED_SOURCES})
    target_link_libraries(naruto_${module} PRIVATE rex::runtime)
    target_include_directories(naruto_${module} PRIVATE "${NARUTO_GENERATED}/${module}")
    target_compile_options(naruto_${module} PRIVATE -O2 -g0 -fno-strict-aliasing -ffp-model=strict)
    target_precompile_headers(naruto_${module} PRIVATE "${NARUTO_GENERATED}/${module}/narutorise_pch.h")
endforeach()
target_compile_definitions(naruto_ai2c PRIVATE
    ReXModule_Register=NarutoAI2C_Register
    ReXModule_GetImageInfo=NarutoAI2C_GetImageInfo
    PPCImageConfig=NarutoAI2C_ImageConfig
    PPCFuncMappings=NarutoAI2C_FuncMappings)
add_library(naruto_app STATIC main.cpp builtin-module.cpp input.cpp native/platform.c)
target_link_libraries(naruto_app PRIVATE naruto_ps5_runtime)
target_include_directories(naruto_app PRIVATE "${NARUTO_GENERATED}/default")
target_compile_options(naruto_app PRIVATE -O2 -g0)
