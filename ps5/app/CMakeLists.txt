set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-march=x86-64-v2)

# The app contains no game code. Game code is a private module that setup
# compiles from the player's disc (ps5/tools/build-game-module.py); the app
# loads it and binds the runtime symbols listed in ps5/module/host-imports.txt.
set(NARUTO_UPSTREAM "${CMAKE_CURRENT_LIST_DIR}/../../upstream/naruto-rise-recomp")
set(NARUTO_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/../module")
set(NARUTO_MODULE_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/module-generated")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${NARUTO_MODULE_DIR}/host-imports.txt" "${NARUTO_MODULE_DIR}/compile-flags.txt"
    "${CMAKE_CURRENT_LIST_DIR}/../patches/rexglue-ps5.patch")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/../tools/module-support.py"
    --output "${NARUTO_MODULE_GENERATED}"
    COMMAND_ERROR_IS_FATAL ANY)

add_library(naruto_app STATIC main.cpp builtin-module.cpp input.cpp button_prompts.cpp save_picture.cpp
    native/platform.c
    "${NARUTO_MODULE_DIR}/loader.cpp" "${NARUTO_MODULE_DIR}/game_module.cpp"
    "${NARUTO_MODULE_GENERATED}/module_exports.cpp"
    "${NARUTO_UPSTREAM}/sdk/src/ui/rex_app.cpp")
target_link_libraries(naruto_app PRIVATE naruto_ps5_runtime imgui)
target_include_directories(naruto_app PRIVATE "${NARUTO_MODULE_GENERATED}"
    "${NARUTO_UPSTREAM}/sdk/thirdparty/xxHash")
target_compile_definitions(naruto_app PRIVATE REXGLUE_BUILD_CONFIG="Release")
target_compile_options(naruto_app PRIVATE -O2 -g0)
