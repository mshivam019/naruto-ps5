// Native startup follows the platform contract used by the working PS5 ports.
#include "native/platform.h"
#include "narutorise_init.h"
#include <SDL3/SDL.h>
#include <rex/cvar.h>
#include <rex/chrono/clock.h>
#include <time.h>
#include <rex/thread/fiber.h>
#include <rex/logging.h>
#include <rex/rex_app.h>
#include <rex/ui/windowed_app_context_sdl.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sys/stat.h>

extern "C" void ps5_input_initialize();
extern "C" void ps5_input_shutdown();
extern "C" void catchReturnFromMain(int);
extern "C" [[noreturn]] void __wrap_exit(int status) {
    catchReturnFromMain(status);
    for (;;) {}
}
extern "C" [[noreturn]] void __wrap__Exit(int status) { __wrap_exit(status); }
extern "C" [[noreturn]] void __wrap__exit(int status) { __wrap_exit(status); }
extern "C" [[noreturn]] void __wrap_abort() {
    say("Naruto fatal error; inspect the runtime log");
    __wrap_exit(1);
}

class NarutoPS5App final : public rex::ReXApp {
public:
    explicit NarutoPS5App(rex::ui::WindowedAppContext& context)
        : rex::ReXApp(context, "narutorise", PPCImageConfig) {}
    void OnConfigurePaths(rex::PathConfig& paths) override {
        paths.game_data_root = "/app0/game";
        paths.user_data_root = "/app0/user";
        paths.cache_root = "/app0/user/cache";
        paths.metadata_root = "/app0/metadata";
        paths.config_path = "/app0/user/narutorise.toml";
        paths.update_data_root.clear();
    }
};

struct FiberCheck {
    rex::thread::Fiber* parent;
    int step;
};

static void check_fiber_entry(void* arg) {
    auto* check = static_cast<FiberCheck*>(arg);
    for (int i = 1; i <= 3; i++) {
        check->step = i;
        rex::thread::Fiber::SwitchTo(check->parent);
    }
    std::abort();
}

static bool check_fibers() {
    auto* parent = rex::thread::Fiber::ConvertCurrentThread();
    if (!parent) return false;
    FiberCheck check{parent, 0};
    auto* child = rex::thread::Fiber::Create(256 * 1024, check_fiber_entry, &check);
    if (!child) {
        parent->Destroy();
        return false;
    }
    bool passed = true;
    for (int i = 1; i <= 3; i++) {
        rex::thread::Fiber::SwitchTo(child);
        if (check.step != i) passed = false;
    }
    child->Destroy();
    parent->Destroy();
    say("native fiber round-trip check: %s", passed ? "passed" : "failed");
    return passed;
}

int main() {
    platform_init("Naruto: Rise of a Ninja");
    if (!check_fibers()) return 1;
    timespec clock_resolution{};
    clock_getres(CLOCK_MONOTONIC, &clock_resolution);
    const auto tick_start = rex::chrono::Clock::host_tick_count_platform();
    const timespec clock_check_delay{0, 100000000};
    nanosleep(&clock_check_delay, nullptr);
    const auto tick_elapsed = rex::chrono::Clock::host_tick_count_platform() - tick_start;
    const auto tick_frequency = rex::chrono::Clock::host_tick_frequency_platform();
    say("clock check: resolution=%ld ns frequency=%llu elapsed=%.6f seconds",
        clock_resolution.tv_nsec, static_cast<unsigned long long>(tick_frequency),
        double(tick_elapsed) / double(tick_frequency));
    mkdir("/app0/user", 0777);
    mkdir("/app0/user/cache", 0777);
    SDL_Environment* environment = SDL_GetEnvironment();
    SDL_SetEnvironmentVariable(environment, "SDL_VIDEODRIVER", "dummy", true);
    SDL_SetEnvironmentVariable(environment, "SDL_AUDIODRIVER", "ps5", true);
    char executable[] = "/app0/eboot.bin";
    char gpu[] = "--gpu_plugin=xenos";
    char video[] = "--video_driver=dummy";
    // Native-title sandboxes reject chdir; optional files use absolute paths.
    char mappings[] = "--hid_mappings_file=/app0/gamecontrollerdb.txt";
    char* arguments[] = {executable, gpu, video, mappings, nullptr};
    rex::cvar::Init(4, arguments);
    rex::cvar::ApplyEnvironment();
    rex::InitLoggingEarly();
    try {
        rex::ui::SDLWindowedAppContext context;
        if (!context.Initialize()) return 1;
        if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
            say("Controller initialization failed: %s", SDL_GetError());
            return 1;
        }
        ps5_input_initialize();
        int result = 1;
        {
            NarutoPS5App app(context);
            if (static_cast<rex::ui::WindowedApp&>(app).OnInitialize()) result = context.RunMainMessageLoop();
            app.InvokeOnDestroy();
        }
        ps5_input_shutdown();
        return result;
    } catch (const std::exception& error) {
        say("Naruto startup failed: %s", error.what());
        return 1;
    }
}
