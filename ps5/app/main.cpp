// Native startup follows the platform contract used by the working PS5 ports.
#include "native/platform.h"
#include "narutorise_init.h"
#include <SDL3/SDL.h>
#include <rex/cvar.h>
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

int main() {
    platform_init("Naruto: Rise of a Ninja");
    mkdir("/app0/user", 0777);
    mkdir("/app0/user/cache", 0777);
    SDL_Environment* environment = SDL_GetEnvironment();
    SDL_SetEnvironmentVariable(environment, "SDL_VIDEODRIVER", "dummy", true);
    SDL_SetEnvironmentVariable(environment, "SDL_AUDIODRIVER", "ps5", true);
    char executable[] = "/app0/eboot.bin";
    char gpu[] = "--gpu_plugin=xenos";
    char video[] = "--video_driver=dummy";
    char* arguments[] = {executable, gpu, video, nullptr};
    rex::cvar::Init(3, arguments);
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
