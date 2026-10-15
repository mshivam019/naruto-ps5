#include "game_module.h"

#include <cstring>
#include <stdexcept>

#include "loader.h"
#include "module_abi.h"  // generated: NARUTO_MODULE_ABI

// Generated from ps5/module/host-imports.txt: the runtime symbols the module
// imports, by name.
void* naruto_host_symbol(const char* name);

namespace naruto_module {
namespace {
game_module::Image g_image;
const rex::PPCImageInfo* g_main_image = nullptr;
rex::runtime::FunctionDispatcher::RegisterFn g_ai2c_register = nullptr;
const rex::PPCImageInfo* (*g_ai2c_image_info)() = nullptr;

void* Required(const char* name, size_t size = 1) {
  void* result = g_image.symbol(name, size);
  if (!result) throw std::runtime_error(std::string("Missing game module symbol: ") + name);
  return result;
}
}  // namespace

bool Load(const char* path, std::string& error) {
  if (g_main_image) return true;
  if (!g_image.load(path, naruto_host_symbol, error)) return false;
  try {
    if (std::memcmp(Required("naruto_module_abi", sizeof(NARUTO_MODULE_ABI)), NARUTO_MODULE_ABI,
                    sizeof(NARUTO_MODULE_ABI))) {
      throw std::runtime_error(
          "Game module was built for a different app version; rerun setup from the matching "
          "release");
    }
    auto* image = static_cast<const rex::PPCImageInfo*>(
        Required("PPCImageConfig", sizeof(rex::PPCImageInfo)));
    g_ai2c_register = reinterpret_cast<rex::runtime::FunctionDispatcher::RegisterFn>(
        Required("NarutoAI2C_Register"));
    g_ai2c_image_info =
        reinterpret_cast<const rex::PPCImageInfo* (*)()>(Required("NarutoAI2C_GetImageInfo"));
    g_main_image = image;
    return true;
  } catch (const std::exception& ex) {
    error = ex.what();
    return false;
  }
}

const rex::PPCImageInfo& MainImage() { return *g_main_image; }
rex::runtime::FunctionDispatcher::RegisterFn Ai2cRegister() { return g_ai2c_register; }
const rex::PPCImageInfo* (*Ai2cImageInfo())() { return g_ai2c_image_info; }
}  // namespace naruto_module
