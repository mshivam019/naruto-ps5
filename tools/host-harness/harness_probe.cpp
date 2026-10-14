// Linux profiling harness only. Drives a virtual gamepad, logs guest present
// cadence plus the engine timer delta, and captures the guest output.
#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

#include <rex/hook.h>
#include <rex/logging.h>
#include <rex/runtime.h>
#include <rex/system/kernel_state.h>
#include <rex/ui/presenter.h>

#include "narutorise_init.h"

namespace {
using Clock = std::chrono::steady_clock;

double EnvSeconds(const char* name, double fallback) {
  const char* value = std::getenv(name);
  return value ? std::atof(value) : fallback;
}

void SetButton(SDL_Joystick* j, SDL_GamepadButton b, bool down) {
  SDL_SetJoystickVirtualButton(j, b, down);
}

void Sleep(double seconds) {
  std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}

// Phase 1: tap A to pass menus and cutscenes. Phase 2: walk and turn the camera,
// tapping A every 8 seconds to dismiss tutorial pop-ups.
void ScriptThread() {
  const double start_delay = EnvSeconds("NARUTO_HARNESS_DELAY", 8.0);
  const double mash = EnvSeconds("NARUTO_HARNESS_MASH", 150.0);
  const double walk = EnvSeconds("NARUTO_HARNESS_WALK", 600.0);
  Sleep(start_delay);
  while (!SDL_WasInit(SDL_INIT_GAMEPAD)) Sleep(0.5);
  SDL_VirtualJoystickDesc desc{};
  SDL_INIT_INTERFACE(&desc);
  desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
  desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
  desc.nbuttons = 15;
  desc.axis_mask = (1u << desc.naxes) - 1;
  desc.button_mask = (1u << desc.nbuttons) - 1;
  desc.name = "Harness Gamepad";
  SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
  SDL_Joystick* j = id ? SDL_OpenJoystick(id) : nullptr;
  if (!j) {
    REXLOG_ERROR("[harness] virtual gamepad failed: {}", SDL_GetError());
    return;
  }
  REXLOG_INFO("[harness] virtual gamepad attached");
  for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) {
    SDL_SetJoystickVirtualAxis(j, axis, axis >= SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? -32768 : 0);
  }
  const auto mash_end = Clock::now() + std::chrono::duration<double>(mash);
  int taps = 0;
  while (Clock::now() < mash_end) {
    const bool start = (++taps % 6) == 0 && taps < 20;
    const SDL_GamepadButton b = start ? SDL_GAMEPAD_BUTTON_START : SDL_GAMEPAD_BUTTON_SOUTH;
    SetButton(j, b, true);
    Sleep(0.15);
    SetButton(j, b, false);
    Sleep(1.1);
  }
  REXLOG_INFO("[harness] walking phase");
  const auto walk_end = Clock::now() + std::chrono::duration<double>(walk);
  int step = 0;
  while (Clock::now() < walk_end) {
    SDL_SetJoystickVirtualAxis(j, SDL_GAMEPAD_AXIS_LEFTY, -32767);
    SDL_SetJoystickVirtualAxis(j, SDL_GAMEPAD_AXIS_RIGHTX, (step % 4 == 1) ? 26000 : 0);
    if (step % 4 == 3) {
      SetButton(j, SDL_GAMEPAD_BUTTON_SOUTH, true);
      Sleep(0.15);
      SetButton(j, SDL_GAMEPAD_BUTTON_SOUTH, false);
      Sleep(1.85);
    } else {
      Sleep(2.0);
    }
    ++step;
  }
  SDL_SetJoystickVirtualAxis(j, SDL_GAMEPAD_AXIS_LEFTY, 0);
  SDL_SetJoystickVirtualAxis(j, SDL_GAMEPAD_AXIS_RIGHTX, 0);
}

struct ScriptStarter {
  ScriptStarter() {
    if (std::getenv("NARUTO_HARNESS_SCRIPT")) std::thread(ScriptThread).detach();
  }
} script_starter;

uint32_t LoadBE32(uint8_t* base, uint32_t address) {
  uint32_t raw;
  std::memcpy(&raw, base + address, 4);
  return std::byteswap(raw);
}

// Writes the guest output (not the desktop) to NARUTO_HARNESS_SHOTS/shot-N.ppm.
void CaptureThread(std::string dir) {
  for (int n = 0;; ++n) {
    Sleep(EnvSeconds("NARUTO_HARNESS_SHOT_INTERVAL", 30.0));
    auto* kernel = REX_KERNEL_STATE();
    auto* graphics = kernel ? kernel->emulator()->graphics_system() : nullptr;
    auto* presenter = graphics ? graphics->presenter() : nullptr;
    rex::ui::RawImage image;
    if (!presenter || !presenter->CaptureGuestOutput(image)) continue;
    const std::string path = dir + "/shot-" + std::to_string(n) + ".ppm";
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) continue;
    std::fprintf(f, "P6\n%u %u\n255\n", image.width, image.height);
    for (uint32_t y = 0; y < image.height; ++y) {
      const uint8_t* row = image.data.data() + y * image.stride;
      for (uint32_t x = 0; x < image.width; ++x) std::fwrite(row + x * 4, 1, 3, f);
    }
    std::fclose(f);
    REXLOG_INFO("[harness] captured {}", path);
  }
}

struct CaptureStarter {
  CaptureStarter() {
    if (const char* dir = std::getenv("NARUTO_HARNESS_SHOTS")) {
      std::thread(CaptureThread, std::string(dir)).detach();
    }
  }
} capture_starter;

struct Window {
  Clock::time_point begin = Clock::now();
  uint32_t frames = 0;
  double dt_sum = 0, dt_min = 1e9, dt_max = 0;
  uint32_t fixed = 0;
  float step = 0, max_step = 0;
} window;
}  // namespace

extern "C" void __imp__sub_820EED48(PPCContext& __restrict, uint8_t*);

// Game present wrapper; runs once per guest frame on the render thread.
REX_HOOK_RAW(sub_820EED48) {
  const uint32_t timer = LoadBE32(base, 0x82C0F080);
  if (timer) {
    const float dt = std::bit_cast<float>(LoadBE32(base, timer + 64));
    window.dt_sum += dt;
    window.dt_min = std::min<double>(window.dt_min, dt);
    window.dt_max = std::max<double>(window.dt_max, dt);
    window.fixed = LoadBE32(base, timer + 72);
    window.step = std::bit_cast<float>(LoadBE32(base, timer + 76));
    window.max_step = std::bit_cast<float>(LoadBE32(base, timer + 80));
  }
  ++window.frames;
  const double elapsed = std::chrono::duration<double>(Clock::now() - window.begin).count();
  if (elapsed >= 5.0) {
    REXLOG_INFO("[harness] presents={} wall_fps={:.2f} engine_dt avg={:.4f} min={:.4f} max={:.4f} "
                "engine_fps={:.2f} fixed={} step={:.4f} max={:.4f}",
                window.frames, window.frames / elapsed, window.dt_sum / window.frames,
                window.dt_min, window.dt_max, window.dt_sum > 0 ? window.frames / window.dt_sum : 0.0, window.fixed, window.step, window.max_step);
    window = Window{};
  }
  __imp__sub_820EED48(ctx, base);
}

