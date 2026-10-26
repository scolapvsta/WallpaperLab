// Renderer entry point derived from Almamu/linux-wallpaperengine src/main.cpp (GPL-3.0).
// Wallpaper Lab embeds the engine library and reports readiness after rendering.
#include "WallpaperEngine/Application/ApplicationContext.h"
#include "WallpaperEngine/Application/WallpaperApplication.h"
#include "WallpaperEngine/Logging/Log.h"
#include <csignal>
#include <fstream>
#include <iostream>
#include <unistd.h>
static volatile sig_atomic_t stopRequested=0;
static void stopHandler(int) {stopRequested=1;}
int engineMain(int argc,char**argv,const std::string& ready) {
    try {
        sLog.addOutput(new std::ostream(std::cout.rdbuf()));
        sLog.addError(new std::ostream(std::cerr.rdbuf()));
        WallpaperEngine::Application::ApplicationContext context(argc,argv);
        context.loadSettingsFromArgv();
        WallpaperEngine::Application::WallpaperApplication app(context);
        if(context.settings.general.onlyListProperties) return 0;
        std::signal(SIGTERM,stopHandler);std::signal(SIGINT,stopHandler);
        app.setup();
        int frames=0;
        while(context.state.general.keepRunning && !stopRequested) {
            app.render();
            if(++frames==3 && context.state.general.keepRunning && !stopRequested && !ready.empty()) { std::ofstream f(ready);f<<getpid()<<'\n'; }
        }
        app.cleanup();
        return 0;
    } catch(const std::exception& e) { std::cerr<<"Renderer: "<<e.what()<<'\n'; return 1; }
}
