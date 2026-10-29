#pragma once
#include "catalog.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
namespace lab {
inline fs::path localLibrary(const json& prefs){
 if(auto p=prefs.value("local_path",std::string());!p.empty())return fs::path(p);
 const char* data=getenv("XDG_DATA_HOME"),*home=getenv("HOME");return (data?fs::path(data):fs::path(home?home:".")/".local/share")/"wallpaper-lab/wallpapers";
}
inline bool localMediaImage(std::string ext){std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);return ext==".png"||ext==".jpg"||ext==".jpeg"||ext==".webp"||ext==".gif"||ext==".bmp"||ext==".tif"||ext==".tiff"||ext==".avif";}
inline std::string mediaExtension(const fs::path& file){auto ext=file.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);return ext;}
inline fs::path importLocalMedia(const fs::path& input,const fs::path& directory){
 auto file=fs::canonical(input);if(!fs::is_regular_file(file))throw std::runtime_error("Seleziona un file immagine o video");auto ext=mediaExtension(file);bool image=localMediaImage(ext);
 if(!image&&ext!=".mp4"&&ext!=".webm"&&ext!=".mkv"&&ext!=".mov"&&ext!=".avi"&&ext!=".m4v")throw std::runtime_error("Formato locale non supportato");
 auto probe=capture({"ffprobe","-v","error","-protocol_whitelist","file","-select_streams","v:0","-show_entries","stream=width,height","-of","json",file.string()});
 int w=0,h=0;try{auto stream=json::parse(probe.at(0)).at("streams").at(0);w=stream.at("width");h=stream.at("height");}catch(...){}
 if(w<1||h<1||w>32768||h>32768)throw std::runtime_error("Impossibile leggere la risoluzione del file");
 auto stamp=file.string()+":"+std::to_string(fs::file_size(file))+":"+std::to_string(fs::last_write_time(file).time_since_epoch().count());uint64_t hash=14695981039346656037ull;for(unsigned char c:stamp){hash^=c;hash*=1099511628211ull;}
 std::ostringstream name;name<<"local-"<<std::hex<<hash;fs::create_directories(directory);auto root=fs::canonical(directory),dest=root/name.str();if(fs::is_regular_file(dest/"project.json"))return dest;
 auto stage=root/(".import-"+name.str()+"-"+std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));if(fs::exists(stage))throw std::runtime_error("Importazione locale già in corso");fs::create_directory(stage);
 try{
  auto media="media"+ext;fs::copy_file(file,stage/media);auto preview=stage/"preview.png";
  capture({"ffmpeg","-v","error","-nostdin","-protocol_whitelist","file","-i",(stage/media).string(),"-frames:v","1","-vf","scale=640:360:force_original_aspect_ratio=decrease","-y",preview.string()});
  if(!fs::is_regular_file(preview))throw std::runtime_error("Impossibile creare l’anteprima del file");
  if(image){std::ofstream html(stage/"index.html");html<<"<!doctype html><meta charset='utf-8'><style>html,body{margin:0;width:100%;height:100%;overflow:hidden;background:#000}img{display:block;width:100%;height:100%;object-fit:cover}</style><img draggable='false' src='"<<media<<"'>";if(!html)throw std::runtime_error("Impossibile creare il progetto locale");}
  writeJson(stage/"project.json",{{"title",file.stem().string()},{"type",image?"web":"video"},{"file",image?"index.html":media},{"preview","preview.png"},{"lab_origin","local"},{"lab_media_kind",image?"image":"video"},{"lab_width",w},{"lab_height",h},{"general",{{"supportsaudioprocessing",false}}}});
  fs::rename(stage,dest);return dest;
 }catch(...){std::error_code ec;fs::remove_all(stage,ec);throw;}
}
inline bool executableAvailable(const std::string& name){const char* paths=getenv("PATH");if(!paths)return false;std::istringstream input(paths);std::string path;while(std::getline(input,path,':'))if(!path.empty()&&access((fs::path(path)/name).c_str(),X_OK)==0)return true;return false;}
inline std::vector<fs::path> chooseLocalFiles(bool folder,const std::string& title){
 std::vector<std::string> args;
 if(executableAvailable("zenity")){args={"zenity","--file-selection","--title="+title};if(folder)args.push_back("--directory");else{args.insert(args.end(),{"--multiple","--separator=\n","--file-filter=Media | *.png *.jpg *.jpeg *.webp *.gif *.bmp *.tif *.tiff *.avif *.mp4 *.webm *.mkv *.mov *.avi *.m4v","--file-filter=Tutti i file | *"});}}
 else if(executableAvailable("kdialog")){args=folder?std::vector<std::string>{"kdialog","--getexistingdirectory"}:std::vector<std::string>{"kdialog","--getopenfilename",".","*.png *.jpg *.jpeg *.webp *.gif *.bmp *.tif *.tiff *.avif *.mp4 *.webm *.mkv *.mov *.avi *.m4v","--multiple","--separate-output"};args.insert(args.end(),{"--title",title});}
 else throw std::runtime_error("Per scegliere i file serve zenity o kdialog");
 auto result=capture(args);std::vector<fs::path> files;if(result.empty())return files;std::istringstream lines(result[0]);std::string line;while(std::getline(lines,line))if(!line.empty())files.emplace_back(line);return files;
}
}
