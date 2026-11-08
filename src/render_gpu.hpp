#pragma once
// Render GPU selection: lists DRM render nodes and maps a saved choice to a DRI_PRIME tag.
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace lab {
struct RenderGpu {
    std::string id;    // DRI_PRIME tag, e.g. pci-0000_12_00_0
    std::string label;
};

inline std::string readFirstLine(const std::filesystem::path& path) {
    std::ifstream file(path);std::string line;std::getline(file,line);
    while(!line.empty()&&std::isspace((unsigned char)line.back()))line.pop_back();
    return line;
}

// Device name from pci.ids, preferring the marketing name in brackets.
inline std::string pciDeviceName(const std::filesystem::path& ids,std::string vendor,std::string device) {
    auto hex=[](std::string v){if(v.starts_with("0x"))v=v.substr(2);std::transform(v.begin(),v.end(),v.begin(),::tolower);return v;};
    vendor=hex(vendor);device=hex(device);
    std::ifstream file(ids);std::string line;bool inVendor=false;
    while(std::getline(file,line)){
        if(line.empty()||line[0]=='#')continue;
        if(line[0]!='\t'){inVendor=line.starts_with(vendor+"  ");continue;}
        if(!inVendor||line.size()<2||line[1]=='\t'||!line.substr(1).starts_with(device+"  "))continue;
        auto name=line.substr(1+device.size()+2);
        auto open=name.find('['),close=name.rfind(']');
        return open!=std::string::npos&&close>open+1?name.substr(open+1,close-open-1):name;
    }
    return {};
}

inline std::vector<RenderGpu> renderGpus(const std::filesystem::path& byPath="/dev/dri/by-path",
    const std::filesystem::path& pciDevices="/sys/bus/pci/devices",
    const std::filesystem::path& pciIds="/usr/share/hwdata/pci.ids") {
    std::vector<RenderGpu> gpus;std::error_code ec;
    for(const auto& entry:std::filesystem::directory_iterator(byPath,ec)){
        auto name=entry.path().filename().string();
        if(!name.starts_with("pci-")||!name.ends_with("-render"))continue;
        auto slot=name.substr(4,name.size()-4-7);
        auto id="pci-"+slot;std::replace(id.begin(),id.end(),':','_');std::replace(id.begin(),id.end(),'.','_');
        auto vendor=readFirstLine(pciDevices/slot/"vendor"),device=readFirstLine(pciDevices/slot/"device");
        auto label=vendor.empty()?std::string():pciDeviceName(pciIds,vendor,device);
        if(label.empty())label=vendor.empty()?slot:vendor.substr(vendor.starts_with("0x")?2:0)+":"+device.substr(device.starts_with("0x")?2:0);
        gpus.push_back({id,label+" ("+slot+")"});
    }
    std::sort(gpus.begin(),gpus.end(),[](const auto& a,const auto& b){return a.id<b.id;});
    return gpus;
}

// Empty means automatic: the renderer keeps the inherited environment.
inline std::string driPrimeFor(const std::string& choice,const std::vector<RenderGpu>& gpus) {
    for(const auto& gpu:gpus)if(gpu.id==choice)return choice;
    return {};
}
}
