// Save-file pictures for title 555307E5.
//
// When saving, the game resolves a 320x180 RGB565 picture of the screen and
// reads it on the CPU into the save; the Load Game menu shows it. Reading every
// resolve back each frame is far too slow on PS5 (about 2 FPS), so only this
// resolve is read back, synchronously, once per save.
//
// naruto_log_resolves logs each distinct resolve shape once, for finding it.
#include <cstdint>
#include <mutex>
#include <set>
#include <tuple>

#include <rex/cvar.h>
#include <rex/graphics/command_processor.h>
#include <rex/logging.h>

REXCVAR_DEFINE_BOOL(naruto_log_resolves, false, "narutorise",
                    "Debug: log each distinct resolve destination shape once");

namespace {
using rex::graphics::CommandProcessor;

std::mutex g_mutex;
std::set<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> g_seen;

bool OnResolve(const CommandProcessor::ResolveReadbackRequest& request) {
  if (REXCVAR_GET(naruto_log_resolves)) {
    std::lock_guard lock(g_mutex);
    if (g_seen.emplace(uint32_t(request.format), request.pitch, request.height, request.length)
            .second) {
      REXLOG_INFO("[narutorise] resolve format={} pitch={} height={} length={} at {:08X}",
                  uint32_t(request.format), request.pitch, request.height, request.length,
                  request.address);
    }
  }
  return request.format == rex::graphics::xenos::ColorFormat::k_5_6_5 && request.pitch == 320 &&
         request.height == 180;
}

const bool g_registered = [] {
  CommandProcessor::SetResolveReadbackFilter(OnResolve);
  return true;
}();
}  // namespace
