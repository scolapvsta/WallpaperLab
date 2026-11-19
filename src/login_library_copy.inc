// Copy only this project's visual preferences. Desktop state is never modified.
json copyLibrarySettings(json d,const Item& item,const json& libraryProfiles){
    d=validate(d);if(!supported(item)||sourceFor(d)!=item.path)throw std::invalid_argument("Select a login wallpaper first");
    auto original=libraryProfiles.value(item.path,json::object());if(!original.is_object())throw std::invalid_argument("Invalid wallpaper profile");
    auto clean=validate(defaults());auto profile=imageProfile(clean);auto position=wallpaperPosition(libraryProfiles,item.path);
    for(auto key:{"x","y","zoom"})profile[key]=position[key];
    auto image=original.value("image",json::object());for(auto& [key,value]:profile.items())if(key!="x"&&key!="y"&&key!="zoom"&&image.contains(key))value=image[key];
    // The desktop renderer fills each output; its X/Y/zoom express the crop.
    profile["crop"]=1.;
    if(d["mode"]=="single")d["output_profiles"][d["output"].get<std::string>()][item.path]=profile;
    else d["profiles"][item.path]=profile;
    d["variants"].erase(item.path);
    if(original.contains("variant"))for(auto& v:item.variants)if(original["variant"].value("property",std::string())==v.property&&original["variant"].value("value",std::string())==v.value){d["variants"][item.path]={{"property",v.property},{"value",v.value}};break;}
    json properties=json::object();auto overrides=original.value("properties",json::object());
    for(auto& [key,value]:overrides.items())if(item.properties.contains(key)){try{properties[key]=validateProperty(item.properties.at(key),value);}catch(...){/* Match the main renderer's handling of stale overrides. */}}
    json snapshot={{"properties",properties}};
    d["project_profiles"][item.path]=snapshot;return validate(d);
}
json projectProfile(const json& d,const std::string& source,const json& libraryProfiles){
    auto saved=d.value("project_profiles",json::object());
    return saved.contains(source)?saved.at(source):libraryProfiles.value(source,json::object());
}
