// The game code lives in a private module that setup builds from the player's
// own disc; the app (eboot.bin) contains only the runtime.
#pragma once

#include <string>

#include <rex/image_info.h>
#include <rex/system/function_dispatcher.h>

namespace naruto_module {
inline constexpr char kPath[] = "/app0/game/naruto.module";

// Loads and binds the module once. On failure, error describes the problem.
bool Load(const char* path, std::string& error);

// Valid after Load succeeded.
const rex::PPCImageInfo& MainImage();
rex::runtime::FunctionDispatcher::RegisterFn Ai2cRegister();
const rex::PPCImageInfo* (*Ai2cImageInfo())();
}  // namespace naruto_module
