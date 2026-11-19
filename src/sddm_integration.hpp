#pragma once
#include "catalog.hpp"
#include <chrono>
#include <unistd.h>
namespace lab {
inline json sddmRequest(const std::string& action, const std::string& value, const std::vector<fs::path>& roots) {
    auto helper=fs::read_symlink("/proc/self/exe").parent_path()/"sddm_integration.py";
    if(!fs::is_regular_file(helper))throw std::runtime_error("SDDM integration helper unavailable");
    auto file=stateDir()/"sddm"/("request-"+std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".json");
    json directories=json::array();for(const auto& root:roots)directories.push_back(root.string());
    writeJson(file,{{"action",action},{"value",value},{"roots",directories},{"config",configDir().string()},{"state",stateDir().string()}});
    try {auto output=capture({"python3",helper.string(),"--request",file.string()});fs::remove(file);if(output.empty())throw std::runtime_error("SDDM integration helper failed");return json::parse(output.front());}
    catch(...){std::error_code ec;fs::remove(file,ec);throw;}
}
}
