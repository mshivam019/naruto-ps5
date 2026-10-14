// Race fix for the engine's "cLoader" worker thread (title 555307E5).
//
// sub_8265BDE0 allocates the 248-byte thread object, starts its thread through
// vtable slot 7 and only afterwards stores the entry function (+236) and its
// argument (+240). The thread body, sub_8265BB68, immediately calls the entry
// at +236. On the Xbox 360 the creator wins this race; on PS5 the new host
// thread can run first and calls uninitialized memory (logged as
// "Call to invalid or unregistered function", e.g. 0xE464C0C8, about 18 s after
// boot when ai2c.dll loads). The thread body now waits until its creator has
// returned from sub_8265BDE0.
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_set>
#include <bit>

#include <rex/hook.h>
#include <rex/logging.h>

#include "narutorise_init.h"

namespace {
std::mutex g_mutex;
std::condition_variable g_ready_changed;
std::unordered_set<uint32_t> g_ready_threads;

uint32_t LoadGuest(uint8_t* base, uint32_t address) {
  uint32_t raw;
  std::memcpy(&raw, base + address, sizeof(raw));
  return std::byteswap(raw);
}
}  // namespace

extern "C" void __imp__sub_8265BDE0(PPCContext& __restrict, uint8_t*);
extern "C" void __imp__sub_8265BB68(PPCContext& __restrict, uint8_t*);

// Creates the loader thread object (stored at owner +4) and starts it.
REX_HOOK_RAW(sub_8265BDE0) {
  const uint32_t owner = ctx.r3.u32;
  __imp__sub_8265BDE0(ctx, base);
  const uint32_t thread_object = LoadGuest(base, owner + 4);
  {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ready_threads.insert(thread_object);
  }
  g_ready_changed.notify_all();
}

// Loader thread body: calls the entry function stored at +236 with +240.
REX_HOOK_RAW(sub_8265BB68) {
  const uint32_t thread_object = ctx.r3.u32;
  {
    std::unique_lock<std::mutex> lock(g_mutex);
    if (!g_ready_changed.wait_for(lock, std::chrono::seconds(5), [&] {
          return g_ready_threads.count(thread_object) != 0;
        })) {
      REXLOG_WARN("[narutorise] loader thread {:08X} started without its creator", thread_object);
    }
    g_ready_threads.erase(thread_object);
  }
  __imp__sub_8265BB68(ctx, base);
}
