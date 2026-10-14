// Guest frame-rate selection for Naruto: Rise of a Ninja (title 555307E5).
//
// The game chooses its presentation rate through engine command dispatcher
// sub_820EACE0 (r3 = command, r4 = argument):
//   45: request a rate; 68: lock a rate (0 unlocks and reapplies the request).
// The handler maps 60 to D3D PresentInterval 1 and sets the engine 60 FPS flag
// at 0x82C0F050, -1 to an immediate present, and any other value to
// PresentInterval 2. Battles request 60; the open world requests 30.
// The open world also calls engine timer SetFixedStep (sub_825B6148,
// r3 = enabled, f1 = step) with a 1/30 second step, so every frame advances
// gameplay by 1/30 second. At 60 FPS that doubles game speed. With naruto_fps60
// enabled, that call keeps the timer in its measured mode (sub_826ACFC0), and
// the timer's maximum step (+80) becomes the requested 1/30 second. Gameplay then
// follows wall time between 30 and 60 FPS. Below 30 FPS it slows exactly as the
// original fixed step does.
#include <bit>
#include <cstdint>
#include <cstring>

#include <rex/cvar.h>
#include <rex/hook.h>
#include <rex/logging.h>

#include "narutorise_init.h"

REXCVAR_DEFINE_BOOL(naruto_fps60, true, "narutorise",
                    "Use the game's 60 FPS mode where it requests 30 FPS");

namespace {
constexpr uint32_t kRequestFrameRate = 45;
constexpr uint32_t kLockFrameRate = 68;
constexpr uint32_t kImmediate = 0xFFFFFFFFu;
constexpr uint32_t kSixty = 60;
constexpr uint32_t kTimerPointer = 0x82C0F080;
constexpr uint32_t kTimerMaximumStep = 80;
// Fixed steps longer than this are 30 FPS-style steps (1/30 s); 1/60 s steps stay.
constexpr double kLongestSixtyStep = 1.0 / 45.0;

uint32_t LoadGuest(uint8_t* base, uint32_t address) {
  uint32_t raw;
  std::memcpy(&raw, base + address, sizeof(raw));
  return std::byteswap(raw);
}

void StoreGuest(uint8_t* base, uint32_t address, uint32_t value) {
  const uint32_t raw = std::byteswap(value);
  std::memcpy(base + address, &raw, sizeof(raw));
}

// Maximum measured step installed by the timer reset (sub_826ACE38), 0.166 s.
uint32_t g_original_maximum_step = 0;
}  // namespace

extern "C" void __imp__sub_825B6148(PPCContext& __restrict, uint8_t*);

REX_HOOK_RAW(sub_825B6148) {
  const uint32_t timer = LoadGuest(base, kTimerPointer);
  const bool enabled = ctx.r3.u32 != 0;
  const double step = ctx.f1.f64;
  if (timer && REXCVAR_GET(naruto_fps60)) {
    const uint32_t maximum_address = timer + kTimerMaximumStep;
    if (enabled && step > kLongestSixtyStep) {
      if (!g_original_maximum_step) {
        g_original_maximum_step = LoadGuest(base, maximum_address);
      }
      StoreGuest(base, maximum_address, std::bit_cast<uint32_t>(static_cast<float>(step)));
      ctx.r3.u64 = 0;
      REXLOG_INFO("[narutorise] engine step {:.4f} s fixed -> measured, max {:.4f} s", step, step);
    } else if (g_original_maximum_step) {
      StoreGuest(base, maximum_address, g_original_maximum_step);
      g_original_maximum_step = 0;
      REXLOG_INFO("[narutorise] engine step restored (enabled={} step={:.4f} s)", enabled, step);
    }
  }
  __imp__sub_825B6148(ctx, base);
}

extern "C" void __imp__sub_820EACE0(PPCContext& __restrict, uint8_t*);

REX_HOOK_RAW(sub_820EACE0) {
  const uint32_t command = ctx.r3.u32;
  if (command == kRequestFrameRate || command == kLockFrameRate) {
    const uint32_t requested = ctx.r4.u32;
    // Lock value 0 releases the lock; the stored request is reapplied.
    const bool unlocks = command == kLockFrameRate && requested == 0;
    const bool rewrite = REXCVAR_GET(naruto_fps60) && !unlocks &&
                         requested != kImmediate && requested != kSixty;
    if (rewrite) {
      ctx.r4.u64 = kSixty;
    }
    REXLOG_INFO("[narutorise] frame rate {} {} -> {}",
                command == kRequestFrameRate ? "request" : "lock",
                static_cast<int32_t>(requested), static_cast<int32_t>(ctx.r4.u32));
  }
  __imp__sub_820EACE0(ctx, base);
}
