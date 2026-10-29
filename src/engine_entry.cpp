// Renderer entry point derived from Almamu/linux-wallpaperengine src/main.cpp (GPL-3.0).
// Wallpaper Lab embeds the engine library and reports readiness after rendering.
#include "WallpaperEngine/Application/ApplicationContext.h"
#include "WallpaperEngine/Application/WallpaperApplication.h"
#include "WallpaperEngine/Logging/Log.h"
#include <csignal>
#include "desktop_surface.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "WallpaperEngine/Render/Helpers/SpanGeometry.h"
#include "WallpaperEngine/Render/Helpers/WallpaperPlacement.h"
static volatile sig_atomic_t stopRequested=0;
static void stopHandler(int) {stopRequested=1;}
int engineMain(int argc,char**argv,const std::string& ready) {
    try {
        sLog.addOutput(new std::ostream(std::cout.rdbuf()));
        sLog.addError(new std::ostream(std::cerr.rdbuf()));
        // The layout is an immutable snapshot passed only to this renderer.
        std::vector<char*> engineArgs={argv[0]};std::string layout,position,profileFile,liveFile,project,feed;
        for(int i=1;i<argc;i++){
            if(std::string(argv[i])=="--lab-transition"){if(++i>=argc)throw std::runtime_error("Transizione mancante");setenv("WALLPAPER_LAB_TRANSITION",argv[i],1);
            }else if(std::string(argv[i])=="--lab-layout"){
                if(++i>=argc)throw std::runtime_error("Geometria del panorama mancante");
                layout=argv[i];WallpaperEngine::Render::Helpers::SpanGeometry::parse(layout);
            }else if(std::string(argv[i])=="--lab-position"){
                if(++i>=argc)throw std::runtime_error("Posizione dello sfondo mancante");
                position=argv[i];WallpaperEngine::Render::Helpers::WallpaperPlacement::parse(position);
            }else if(std::string(argv[i])=="--lab-profile-file"||std::string(argv[i])=="--lab-live-file"||std::string(argv[i])=="--lab-project"||std::string(argv[i])=="--lab-feed-dir"){
                std::string key=argv[i];if(++i>=argc)throw std::runtime_error("Parametro live mancante");
                if(key=="--lab-profile-file")profileFile=argv[i];else if(key=="--lab-live-file")liveFile=argv[i];else if(key=="--lab-project")project=argv[i];else feed=argv[i];
            }else engineArgs.push_back(argv[i]);
        }
        if(layout.empty())unsetenv("WALLPAPER_LAB_SPAN_LAYOUT");else setenv("WALLPAPER_LAB_SPAN_LAYOUT",layout.c_str(),1);
        if(position.empty())unsetenv("WALLPAPER_LAB_POSITION");else setenv("WALLPAPER_LAB_POSITION",position.c_str(),1);
        for(auto [name,value]:std::vector<std::pair<const char*,std::string>>{{"WALLPAPER_LAB_PROFILE_FILE",profileFile},{"WALLPAPER_LAB_LIVE_FILE",liveFile},{"WALLPAPER_LAB_PROJECT",project},{"WALLPAPER_LAB_FEED_DIR",feed}}){if(value.empty())unsetenv(name);else setenv(name,value.c_str(),1);}
        int engineCount=engineArgs.size();engineArgs.push_back(nullptr);
        WallpaperEngine::Application::ApplicationContext context(engineCount,engineArgs.data());
        context.loadSettingsFromArgv();
        WallpaperEngine::Application::WallpaperApplication app(context);
        if(context.settings.general.onlyListProperties) return 0;
        std::signal(SIGTERM,stopHandler);std::signal(SIGINT,stopHandler);
        lab::prepareDesktopSurface();
        app.setup();
        int frames=0;bool reportedReady=false;
        while(context.state.general.keepRunning && !stopRequested) {
            app.render();
            if(++frames>=3 && !reportedReady && app.isReady() && context.state.general.keepRunning && !stopRequested && !ready.empty()) { std::ofstream f(ready);f<<getpid()<<'\n';reportedReady=true; }
        }
        app.cleanup();
        return 0;
    } catch(const std::exception& e) { std::cerr<<"Renderer: "<<e.what()<<'\n'; return 1; }
}
