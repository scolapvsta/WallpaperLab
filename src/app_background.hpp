#pragma once
#include "local_library.hpp"
namespace lab {
// Decode to a bounded PNG kept in XDG data, never in the wallpaper library.
inline fs::path importAppBackground(const fs::path& input){
 auto file=fs::canonical(input);if(!fs::is_regular_file(file)||!localMediaImage(mediaExtension(file)))throw std::runtime_error("Scegli un’immagine per lo sfondo dell’app");
 auto probe=capture({"ffprobe","-v","error","-protocol_whitelist","file","-select_streams","v:0","-show_entries","stream=width,height","-of","json",file.string()});
 int w=0,h=0;try{auto stream=json::parse(probe.at(0)).at("streams").at(0);w=stream.at("width");h=stream.at("height");}catch(...){}
 if(w<1||h<1||w>32768||h>32768)throw std::runtime_error("Immagine non valida: sfondo precedente conservato");
 const char* data=getenv("XDG_DATA_HOME"),*home=getenv("HOME");auto dir=(data?fs::path(data):fs::path(home?home:".")/".local/share")/"wallpaper-lab/app-backgrounds";fs::create_directories(dir);
 auto id=std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());auto stage=dir/(".import-"+id+".png"),dest=dir/(id+".png");
 try{
  auto converted=capture({"ffmpeg","-v","error","-nostdin","-protocol_whitelist","file","-i",file.string(),"-frames:v","1","-vf","scale=2048:2048:force_original_aspect_ratio=decrease","-y",stage.string()});
  if(converted.empty()||!fs::is_regular_file(stage)||fs::file_size(stage)<24)throw std::runtime_error("Impossibile importare lo sfondo: immagine precedente conservata");
  fs::rename(stage,dest);return dest;
 }catch(...){std::error_code ec;fs::remove(stage,ec);throw;}
}
}
