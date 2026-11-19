// Presets contain configuration only; wallpaper assets and authentication state stay local.
static const std::array<const char*,4> uiPresetKeys={"widgets","grid","font","accent"};
static const std::array<const char*,11> scenePresetKeys={"source","mode","output","ui_outputs","ui_primary","fps","profiles","wallpapers","output_profiles","variants","project_profiles"};
json makePreset(const json& document,const std::string& name,bool full){
    if(name.empty()||name.size()>96||name.find_first_not_of(" \t\r\n")==std::string::npos||std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32;}))throw std::invalid_argument("Invalid preset name");
    auto d=validate(document);json data=json::object();for(auto key:uiPresetKeys)data[key]=d[key];if(full)for(auto key:scenePresetKeys)data[key]=d[key];
    return {{"format","wallpaper-lab-login-preset"},{"version",1},{"kind",full?"complete":"ui"},{"name",name},{"data",data}};
}
json applyPreset(json d,const json& preset,bool full,const std::vector<Item>& library){
    if(preset.value("format",std::string())!="wallpaper-lab-login-preset"||preset.value("version",0)!=1||preset.value("kind",std::string())!=(full?"complete":"ui")||!preset.at("data").is_object())throw std::invalid_argument("Invalid login preset");
    auto& data=preset.at("data");for(auto key:uiPresetKeys)d[key]=data.at(key);if(full)for(auto key:scenePresetKeys)d[key]=std::string(key)=="project_profiles"?data.value(key,json::object()):data.at(key);d=validate(d);
    if(full){
        std::set<std::string> sources;auto source=sourceFor(d);if(!source.empty())sources.insert(source);
        if(d["mode"]=="single")for(auto& [output,path]:d["wallpapers"].items())sources.insert(path.get<std::string>());
        if(sources.empty())throw std::invalid_argument("Preset wallpaper is unavailable");
        for(auto& path:sources)if(std::none_of(library.begin(),library.end(),[&](const Item& item){return item.path==path&&supported(item);}))throw std::invalid_argument("Preset wallpaper is unavailable");
    }
    return d;
}
fs::path presetDirectory(bool full){return configDir()/"login-presets"/(full?"complete":"ui");}
std::vector<json> listPresets(bool full){
    std::vector<json> result;std::vector<fs::path> directories;
    if(!full)directories.push_back(astronautAssets()/"presets");directories.push_back(presetDirectory(full));
    for(auto& directory:directories){if(!fs::is_directory(directory))continue;
    for(auto& entry:fs::directory_iterator(directory))try{
        if(entry.is_symlink()||!entry.is_regular_file()||entry.path().extension()!=".json"||entry.file_size()>1024*1024)continue;
        auto p=readJson(entry.path());auto checked=makePreset(p.at("data"),p.at("name"),full);
        if(p.value("format",std::string())!=checked["format"].get<std::string>()||p.value("version",0)!=1||p.value("kind",std::string())!=checked["kind"].get<std::string>())continue;
        p["file"]=entry.path().string();p["bundled"]=directory!=presetDirectory(full);
        auto existing=std::find_if(result.begin(),result.end(),[&](auto& item){return item["name"]==p["name"];});
        if(existing==result.end())result.push_back(p);else *existing=p;
    }catch(...){/* A damaged file must not hide the remaining presets. */}}

    std::sort(result.begin(),result.end(),[](auto& a,auto& b){return a["name"]<b["name"];});return result;
}
void savePreset(const json& document,const std::string& name,bool full){
    auto preset=makePreset(document,name,full);ControllerLock lock;auto path=presetDirectory(full)/(std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+".json");
    for(auto& existing:listPresets(full))if(existing["name"]==name&&!existing.value("bundled",false)){path=existing["file"].get<std::string>();writeJson(stateDir()/"backups"/("login-preset-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/path.filename(),readJson(path));break;}
    writeJson(path,preset);
}
