// The first private test title links the base-game engine module statically.
#include <rex/image_info.h>
#include <rex/system/function_dispatcher.h>
#include <cstring>

extern "C" void NarutoAI2C_Register(rex::runtime::IModuleRegistrar*);
extern "C" const rex::PPCImageInfo* NarutoAI2C_GetImageInfo();

extern "C" bool rex_ps5_resolve_builtin_module(
    const char* name,
    rex::runtime::FunctionDispatcher::RegisterFn* register_fn,
    const rex::PPCImageInfo* (**image_info_fn)()) {
    if (!name || !register_fn || !image_info_fn) return false;
    *register_fn = nullptr;
    *image_info_fn = nullptr;
    if (std::strcmp(name, "narutorise_AI2C") != 0) return false;
    *register_fn = NarutoAI2C_Register;
    *image_info_fn = NarutoAI2C_GetImageInfo;
    return true;
}
