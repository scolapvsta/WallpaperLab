#include "shortcuts.hpp"
#include "preferences.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <regex>
#include <sstream>
namespace lab {
namespace {
const std::string description="Wallpaper Lab carousel";
std::string lower(std::string s){std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return std::tolower(c);});return s;}
std::string shellQuote(const std::string& s){std::string out="'";for(char c:s)out+=c=='\''?"'\\''":std::string(1,c);return out+"'";}
std::string textFile(const fs::path& file){std::ifstream f(file);return {std::istreambuf_iterator<char>(f),{}};}
void writeText(const fs::path& path,const std::string& s){fs::create_directories(path.parent_path());auto tmp=path.string()+".tmp";{std::ofstream f(tmp);f<<s;if(!f)throw std::runtime_error("Scrittura fallita: "+path.string());}fs::rename(tmp,path);}
}
Shortcut parseShortcut(const std::string& input){
    Shortcut s;std::istringstream stream(input);std::string part;
    while(std::getline(stream,part,'+')) {
        part.erase(std::remove_if(part.begin(),part.end(),[](unsigned char c){return std::isspace(c);}),part.end());
        auto token=lower(part);int bit=token=="super"?64:token=="ctrl"?4:token=="alt"?8:token=="shift"?1:0;
        if(bit){if(s.mask&bit)throw std::runtime_error("Modificatore ripetuto");s.mask|=bit;}
        else {if(!s.key.empty()||!std::regex_match(part,std::regex("([a-zA-Z0-9]|F([1-9]|1[0-2])|Tab|Left|Right|Up|Down|Home|End|Page_Up|Page_Down|space)")))throw std::runtime_error("Scorciatoia non valida: usa modificatori e un tasto, ad esempio SUPER+Tab");s.key=part;}
    }
    if(!s.mask||s.key.empty())throw std::runtime_error("Scegli almeno un modificatore e un tasto");
    for(auto [bit,name]:{std::pair{64,"SUPER"},{4,"CTRL"},{8,"ALT"},{1,"SHIFT"}})if(s.mask&bit){if(!s.mods.empty())s.mods+=' ';s.mods+=name;if(!s.combo.empty())s.combo+='+';s.combo+=name;}
    s.combo+='+'+s.key;return s;
}
std::string shortcutConfig(const Shortcut& s,const std::string& launcher,bool lua){
    if(!lua)throw std::runtime_error("Il popup carousel richiede Hyprland con configurazione Lua");
    auto prefix=shellQuote(launcher)+" --carousel-event ";
    // Lua numbers all events before launching clients. Absolute step counts make
    // delivery order irrelevant, including release arriving during cold startup.
    std::string out="-- Managed by Wallpaper Lab: hold modifiers, tap key, release to confirm.\n";
    out+="local epoch = tostring(os.time()) .. '-' .. tostring({}):gsub('[^%w]', '')\nlocal session, steps, held = 0, 0, false\n";
    out+="local function emit(action)\n  hl.dispatch(hl.dsp.exec_cmd("+json(prefix).dump()+" .. epoch .. ' ' .. session .. ' ' .. steps .. ' ' .. action))\nend\n";
    out+="hl.bind("+json(s.combo).dump()+", function()\n  if not held then session = session + 1; steps = 0; held = true end\n  steps = steps + 1\n  emit('step')\nend, { description = \"Wallpaper Lab carousel\" })\n";
    out+="local function finish()\n  if held then held = false; emit('commit') end\nend\n";
    for(auto [bit,key]:{std::pair{64,"Super"},{4,"Control"},{8,"Alt"},{1,"Shift"}})if(s.mask&bit)for(auto side:{"_L","_R"}) {
        std::string name=std::string(key)+side;
        out+="hl.bind("+json(name).dump()+", finish, { release = true, ignore_mods = true, non_consuming = true, transparent = true, description = \"Wallpaper Lab carousel release "+name+"\" })\n";
    }
    out+="hl.window_rule({ name = 'wallpaper-lab-carousel', match = { class = '^wallpaper-lab$', title = '^Wallpaper Lab Carousel$' }, float = true, center = true, no_anim = true, no_blur = true, no_shadow = true, no_dim = true, border_size = 0, rounding = 0, opacity = '1.0 override' })\n";
    return out;
}
json shortcutSettings(){auto file=configDir()/"shortcut.json";return fs::exists(file)?readJson(file):json{{"combo","SUPER+Tab"},{"enabled",false}};}
void configureShortcut(const std::string& combo,bool enabled){
    ControllerLock lock;auto shortcut=parseShortcut(combo);
    auto system=capture({"hyprctl","systeminfo"});
    if(system.empty())throw std::runtime_error("Scorciatoia globale disponibile in una sessione Hyprland");
    bool lua=system[0].find("configProvider: lua")!=std::string::npos;
    auto configHome=configDir().parent_path();auto main=configHome/"hypr"/(lua?"hyprland.lua":"hyprland.conf");
    if(!fs::is_regular_file(main))throw std::runtime_error("Configurazione Hyprland standard non trovata");
    main=fs::canonical(main);
    auto bindings=capture({"hyprctl","binds","-j"});if(bindings.empty())throw std::runtime_error("Impossibile verificare le scorciatoie esistenti");
    if(enabled)for(auto& b:json::parse(bindings[0]))if(b.value("modmask",0)==shortcut.mask&&lower(b.value("key",std::string()))==lower(shortcut.key)&&b.value("description",std::string())!=description)
        throw std::runtime_error("Scorciatoia gia utilizzata da Hyprland. Scegli un'altra combinazione.");
    const char* home=getenv("HOME");if(!home)throw std::runtime_error("HOME mancante");
    auto launcher=fs::path(home)/".local/bin/wallpaper-lab";
    if(!fs::is_regular_file(launcher))throw std::runtime_error("Installa Wallpaper Lab prima di attivare la scorciatoia");
    auto generated=configDir()/(lua?"shortcut.lua":"shortcut.conf");
    auto original=textFile(main), previous=textFile(generated);bool existed=fs::exists(generated);
    auto marker=lua?"-- Wallpaper Lab shortcut":"# Wallpaper Lab shortcut";
    auto changed=original;
    if(changed.find(marker)==std::string::npos)changed+="\n"+std::string(marker)+"\n"+(lua?"dofile("+json(generated.string()).dump()+")\n":"source = "+generated.string()+"\n");
    auto backup=stateDir()/"backups"/std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(backup);fs::copy_file(main,backup/main.filename());
    if(existed)fs::copy_file(generated,backup/generated.filename());
    try {
        writeText(generated,enabled?shortcutConfig(shortcut,launcher.string(),lua):(lua?"-- Shortcut disabled\n":"# Shortcut disabled\n"));
        if(changed!=original)writeText(main,changed);
        auto result=capture({"hyprctl","reload","config-only"});
        if(result.empty()||result[0].find("ok")==std::string::npos)throw std::runtime_error("Hyprland non ha ricaricato la configurazione");
        auto active=capture({"hyprctl","binds","-j"});bool found=false;
        if(!active.empty())for(auto& b:json::parse(active[0]))if(b.value("description",std::string())==description&&b.value("modmask",0)==shortcut.mask&&lower(b.value("key",std::string()))==lower(shortcut.key))found=true;
        if(enabled&&!active.empty()) {
            for(auto [bit,key]:{std::pair{64,"Super"},{4,"Control"},{8,"Alt"},{1,"Shift"}})if(shortcut.mask&bit)for(auto side:{"_L","_R"}) {
                bool releaseFound=false;std::string name=std::string(key)+side;
                for(auto& b:json::parse(active[0]))if(b.value("description",std::string())==description+" release "+name&&b.value("release",false)&&b.value("non_consuming",false))releaseFound=true;
                if(!releaseFound)throw std::runtime_error("Rilascio della scorciatoia non registrato");
            }
        }
        if(found!=enabled)throw std::runtime_error("Scorciatoia non verificata; ripristinata la configurazione precedente");
        writeJson(configDir()/"shortcut.json",{{"combo",shortcut.combo},{"enabled",enabled}});
    }catch(...){
        writeText(main,original);if(existed)writeText(generated,previous);else fs::remove(generated);
        capture({"hyprctl","reload","config-only"});throw;
    }
}
}
