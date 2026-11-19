#include "login_config.hpp"
#include "preferences.hpp"
#include "project_source.hpp"
#include "local_library.hpp"
#include <cmath>
#include <chrono>
#include <unistd.h>
#include <pwd.h>
namespace lab::login {
json defaults(){
    json d={{"schema",2},{"enabled",false},{"source",""},{"mode","span"},{"output",""},{"ui_outputs","primary"},{"ui_primary",""},{"fps",60},{"muted",true},{"profiles",json::object()},{"wallpapers",json::object()},{"output_profiles",json::object()},{"message","Wallpaper Lab"},{"font","Oxanium"},{"accent","#87cfdf"},{"light",false},{"panel_opacity",.65},{"radius",20},{"field_opacity",.10},{"border_width",1.0},{"field_radius",10},{"show_welcome",true},{"show_labels",true}};
    auto widget=[&](const char* id,bool enabled,double x,double y,double w,double h,int size){d["widgets"][id]={{"enabled",enabled},{"x",x},{"y",y},{"width",w},{"height",h},{"size",size}};};
    widget("clock",true,.36,.12,.28,.12,86);widget("date",true,.32,.25,.36,.05,22);widget("media",false,.72,.8,.24,.13,18);widget("message",true,.04,.05,.4,.05,16);widget("login",true,.36,.39,.28,.43,20);return d;
}
std::string sourceFor(const json& d,const std::string& output){
    auto connector=output.empty()?d.value("output",std::string()):output;
    return d.value("mode",std::string())=="single"?d.value("wallpapers",json::object()).value(connector,d.value("source",std::string())):d.value("source",std::string());
}
json validate(json d){
    if(!d.is_object()||(d.value("schema",1)!=1&&d.value("schema",1)!=2))throw std::invalid_argument("Unsupported login document");
    auto base=defaults();base.merge_patch(d);d=base;d["schema"]=2;d.erase("template");d["widgets"].erase("calendar");
    auto choice=[&](const char* key,std::initializer_list<const char*> values){auto v=d.at(key).get<std::string>();for(auto item:values)if(v==item)return;throw std::invalid_argument(std::string("Invalid login option: ")+key);};
    choice("mode",{"span","mirror","single"});choice("ui_outputs",{"primary","secondary","all"});
    for(auto key:{"enabled","muted","light","show_welcome","show_labels"})if(!d[key].is_boolean())throw std::invalid_argument("Invalid login boolean");
    for(auto key:{"source","output","ui_primary","font","message","accent"})if(!d[key].is_string()||d[key].get_ref<const std::string&>().size()>4096)throw std::invalid_argument("Invalid login text");
    if(d["font"].get<std::string>().size()>96||d["message"].get<std::string>().size()>256)throw std::invalid_argument("Login text is too long");
    auto color=d["accent"].get<std::string>();if(color.size()!=7||color[0]!='#'||color.find_first_not_of("0123456789abcdefABCDEF",1)!=std::string::npos)throw std::invalid_argument("Invalid login colour");
    auto number=[](const json& value,double low,double high){if(!value.is_number()||!std::isfinite(value.get<double>())||value.get<double>()<low||value.get<double>()>high)throw std::invalid_argument("Invalid login number");};
    number(d["field_opacity"],0,1);number(d["field_radius"],0,40);number(d["border_width"],0,4);number(d["fps"],15,120);number(d["panel_opacity"],0,1);number(d["radius"],0,60);
    if(!d["widgets"].is_object()||!d["profiles"].is_object()||!d["wallpapers"].is_object()||!d["output_profiles"].is_object())throw std::invalid_argument("Invalid login collections");
    for(auto id:widgets){auto& w=d["widgets"].at(id);if(!w.at("enabled").is_boolean())throw std::invalid_argument("Invalid widget visibility");for(auto key:{"x","y"})number(w.at(key),0,1);for(auto key:{"width","height"})number(w.at(key),.02,1);number(w.at("size"),10,140);w["x"]=std::min(w["x"].get<double>(),1-w["width"].get<double>());w["y"]=std::min(w["y"].get<double>(),1-w["height"].get<double>());}
    d["widgets"]["login"]["enabled"]=true;
    auto& form=d["widgets"]["login"];form["width"]=std::max(.20,form["width"].get<double>());form["height"]=std::max(.32,form["height"].get<double>());form["x"]=std::min(form["x"].get<double>(),1-form["width"].get<double>());form["y"]=std::min(form["y"].get<double>(),1-form["height"].get<double>());
    const std::map<std::string,std::pair<double,double>> ranges={{"x",{-1,1}},{"y",{-1,1}},{"zoom",{.25,3}},{"brightness",{-1,1}},{"contrast",{0,2}},{"saturation",{0,2}},{"temperature",{-1,1}},{"hue",{-180,180}},{"gamma",{.25,3}},{"vignette",{0,1}},{"rotation",{-180,180}},{"flip_x",{0,1}},{"flip_y",{0,1}},{"crop",{0,1}}};
    auto checkProfiles=[&](const json& profiles){if(!profiles.is_object())throw std::invalid_argument("Invalid login image profile");for(auto& [path,p]:profiles.items()){if(!p.is_object())throw std::invalid_argument("Invalid login image profile");for(auto& [key,value]:p.items()){auto it=ranges.find(key);if(it==ranges.end())throw std::invalid_argument("Unknown login image control");number(value,it->second.first,it->second.second);}}};
    checkProfiles(d["profiles"]);for(auto& [output,profiles]:d["output_profiles"].items())checkProfiles(profiles);
    for(auto& [output,path]:d["wallpapers"].items())if(output.empty()||output.size()>128||!path.is_string()||path.get_ref<const std::string&>().size()>4096)throw std::invalid_argument("Invalid login text");
    return d;
}
json load(){auto p=configDir()/"login-screen.json";return fs::exists(p)?validate(readJson(p)):defaults();}
void save(const json& d){ControllerLock lock;writeJson(configDir()/"login-screen.json",validate(d));}
json imageProfile(const json& d,const std::string& output){
    json p={{"x",0},{"y",0},{"zoom",1},{"brightness",0},{"contrast",1},{"saturation",1},{"temperature",0},{"hue",0},{"gamma",1},{"vignette",0},{"rotation",0},{"flip_x",0},{"flip_y",0},{"crop",1}};
    auto source=sourceFor(d,output);p.merge_patch(d.value("profiles",json::object()).value(source,json::object()));
    if(d.value("mode",std::string())=="single")p.merge_patch(d.value("output_profiles",json::object()).value(output.empty()?d.value("output",std::string()):output,json::object()).value(source,json::object()));return p;
}
bool supported(const Item& item){
    if(!item.reason.empty())return false;
    if(item.type=="scene"||item.type=="video")return true;
    try{return item.origin=="local"&&readJson(fs::path(item.path)/"project.json").contains("lab_media_file");}catch(...){return false;}
}
fs::path prepare(const json& document,const Item& item,const json& desktop,const std::vector<std::string>& outputs,const std::vector<Item>& library){
    auto d=validate(document);if(!supported(item))throw std::invalid_argument("Wallpaper is unsupported for native login");
    auto dir=stateDir()/"login-screen"/("preview-"+std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directories(dir);
    try{
        auto project=item.source.is_null()?fs::path(item.path):materializeProject(item.path,item.source);
        auto manifest=readJson(project/"project.json");
        // Imported image/GIF media uses native mpv, never the desktop HTML wrapper.
        if(manifest.contains("lab_media_file")){
            auto media=fs::weakly_canonical(project/manifest["lab_media_file"].get<std::string>());
            if(media.lexically_relative(fs::canonical(project)).empty()||media.lexically_relative(fs::canonical(project)).string().starts_with("..")||!fs::is_regular_file(media))throw std::invalid_argument("Invalid local login media");
            fs::create_directory(dir/"project");std::error_code linkError;fs::create_hard_link(media,dir/"project"/media.filename(),linkError);if(linkError)fs::copy_file(media,dir/"project"/media.filename());
            writeJson(dir/"project/project.json",{{"type","video"},{"file",media.filename().string()},{"title",item.title}});project=dir/"project";
        }
        d["project"]=project.string();d["assets"]=item.origin=="local"&&item.type!="scene"?project.string():assetsPath();
        if(!fs::is_directory(d["assets"].get<std::string>()))throw std::invalid_argument("Wallpaper Engine assets are unavailable");
        try{auto rows=capture({"hyprctl","monitors","-j"});if(!rows.empty()){d["monitors"]=json::array();for(auto& row:json::parse(rows.front())){json screen;for(auto key:{"name","width","height","x","y","scale","transform"})if(row.contains(key))screen[key]=row[key];d["monitors"].push_back(screen);}}}catch(...){}
        d["language"]=desktop.value("language",std::string("en"));d["outputs"]=outputs;d["title"]=item.title;d["renderer"]=fs::read_symlink("/proc/self/exe").string();
        // Seed the first login with the publishing desktop's session. The last
        // successful greetd login takes precedence on subsequent boots.
        if(auto user=getpwuid(getuid()))d["username"]=user->pw_name;
        if(const char* session=getenv("DESKTOP_SESSION")){std::string name=session;if(!name.empty()&&name.size()<128&&fs::path(name).filename()==name)d["session"]=name.ends_with(".desktop")?name:name+".desktop";}
        if(desktop.contains("display_layout"))d["display_layout"]=desktop["display_layout"];
        auto profile=imageProfile(d);d["image_profile"]=profile;profile.erase("crop");
        // Use the renderer's exact profile shape, including independently saved creator properties.
        auto original=loadPlacements().value(item.path,json::object());for(auto& [key,value]:profile.items())if(key=="x"||key=="y"||key=="zoom")original["position"][key]=value;else original["image"][key]=value;
        writeJson(dir/"wallpaper_profiles.json",{{item.path,original}});
        d["profile_file"]=(dir/"wallpaper_profiles.json").string();writeJson(dir/"settings.json",{{"muted",d["muted"]}});
        if(d.value("mode",std::string())=="single"){
            d["prepared_wallpapers"]=json::object();
            for(const auto& output:outputs){
                auto source=sourceFor(d,output);auto found=std::find_if(library.begin(),library.end(),[&](const Item& candidate){return candidate.path==source;});
                const Item* selected=source==item.path?&item:found==library.end()?nullptr:&*found;if(!selected||!supported(*selected))throw std::invalid_argument("Wallpaper is unsupported for native login");
                auto child=d;child["source"]=source;auto image=imageProfile(d,output);child["mode"]="mirror";child["profiles"][source]=image;child.erase("prepared_wallpapers");
                auto file=prepare(child,*selected,desktop,{output},{});auto record=readJson(file);d["prepared_wallpapers"][output]={{"source",source},{"project",record["project"]},{"assets",record["assets"]},{"profile_file",record["profile_file"]},{"image_profile",image},{"title",selected->title}};
                if(selected->type=="scene")d["assets"]=record["assets"];
            }
        }
        d["source"]=item.path;writeJson(dir/"login.json",d);return dir/"login.json";
    }catch(...){std::error_code ec;fs::remove_all(dir,ec);throw;}
}
std::vector<std::string> renderArguments(const json& d,bool windowed){
    std::vector<std::string> a={d.at("renderer"),"--assets-dir",d.at("assets"),"--fps",std::to_string(d.value("fps",60)),"--volume",d.value("muted",true)?"0":"100","--no-audio-processing","--disable-parallax","--no-fullscreen-pause","--lab-profile-file",d.at("profile_file"),"--lab-project",d.at("source")};
    if(d.contains("owner"))a.insert(a.end(),{"--lab-owner",d.at("owner").dump()});
    auto image=d.value("image_profile",imageProfile(d));a.insert(a.end(),{"--lab-position",image.dump()});
    auto project=d.at("project").get<std::string>();auto profile=readJson(d.at("profile_file").get<std::string>()).value(d.value("source",std::string()),json::object());
    auto properties=profile.value("properties",json::object());
    for(auto& [key,value]:properties.items())a.insert(a.end(),{"--set-property",key+"="+(value.is_string()?value.get<std::string>():value.dump())});
    auto scale=image.value("crop",1.)>.5?"fill":"fit";
    if(windowed){a.insert(a.end(),{"--window","0x0x1280x720","--scaling",scale,"--clamp","border",project});return a;}
    auto outputs=d.at("outputs").get<std::vector<std::string>>();if(outputs.empty())throw std::invalid_argument("No monitors available for native login");
    a.insert(a.end(),{"--layer","overlay"});
    if(d.value("mode",std::string())=="span"&&outputs.size()>1){std::string joined;for(auto& o:outputs){if(!joined.empty())joined+=",";joined+=o;}a.insert(a.end(),{"--screen-span",joined,"--scaling",scale,"--bg",project});}
    else for(auto& o:outputs){auto chosen=project;auto scaling=scale;if(d.contains("prepared_wallpapers")&&d["prepared_wallpapers"].contains(o)){const auto& record=d["prepared_wallpapers"][o];chosen=record["project"];scaling=record["image_profile"].value("crop",1.)>.5?"fill":"fit";}a.insert(a.end(),{"--screen-root",o,"--scaling",scaling,"--bg",chosen});}
    return a;
}
}
