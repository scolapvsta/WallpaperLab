#pragma once
#include <fontconfig/fontconfig.h>
#include <filesystem>
#include <string>
namespace lab {
inline fs::path bundledFonts(){auto path=fs::read_symlink("/proc/self/exe").parent_path()/"fonts";if(fs::is_directory(path))return path;return fs::path(WALLPAPER_LAB_SOURCE_ASSETS)/"fonts";}
inline std::string bundledFont(const std::string& family){
 const char* relative=family=="Orbitron"?"orbitron/Orbitron[wght].ttf":family=="Oxanium"?"oxanium/Oxanium[wght].ttf":family=="Silkscreen"?"silkscreen/Silkscreen-Regular.ttf":family=="Rajdhani"?"rajdhani/Rajdhani-SemiBold.ttf":"";
 if(!*relative)return {};auto p=bundledFonts()/relative;return fs::is_regular_file(p)?p.string():"";
}
inline void registerThemeFonts(){static bool registered=false;if(registered)return;registered=true;for(auto family:{"Orbitron","Oxanium","Rajdhani","Silkscreen"}){auto file=bundledFont(family);if(!file.empty())FcConfigAppFontAddFile(FcConfigGetCurrent(),reinterpret_cast<const FcChar8*>(file.c_str()));}FcConfigBuildFonts(FcConfigGetCurrent());}
}
