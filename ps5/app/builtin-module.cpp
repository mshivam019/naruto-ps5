// The engine's ai2c.dll module is part of the private game module; the runtime
// asks for it by name when the game loads the DLL.
#include <cstring>

#include <rex/image_info.h>
#include <rex/system/function_dispatcher.h>

#include "../module/game_module.h"

extern "C" bool rex_ps5_resolve_builtin_module(
    const char* name,
    rex::runtime::FunctionDispatcher::RegisterFn* register_fn,
    const rex::PPCImageInfo* (**image_info_fn)()) {
    if (!name || !register_fn || !image_info_fn) return false;
    *register_fn = nullptr;
    *image_info_fn = nullptr;
    if (std::strcmp(name, "narutorise_AI2C") != 0) return false;
    *register_fn = naruto_module::Ai2cRegister();
    *image_info_fn = naruto_module::Ai2cImageInfo();
    return *register_fn && *image_info_fn;
}
