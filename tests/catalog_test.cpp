#include "catalog.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
using namespace lab;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
    auto root=fs::temp_directory_path()/("wallpaper-lab-test-"+std::to_string(getpid()));
    try{
        writeJson(root/"100/project.json",{{"title","Scene with spaces; $(noop)"},{"type","Scene"},{"preview","preview.gif"},{"file","scene.json"}});
        std::ofstream(root/"100/scene.pkg")<<"fixture";std::ofstream(root/"100/preview.gif")<<"fixture";
        writeJson(root/"200/project.json",{{"title","Preset"},{"dependency","100"}});
        writeJson(root/"300/project.json",{{"title","Missing video"},{"type","Video"},{"file","missing.mp4"},{"preview","../../outside.png"}});
        fs::create_directories(root/"400");std::ofstream(root/"400/project.json")<<"{invalid";
        auto rows=scan({root,root});require(rows.size()==4,"deduplication or malformed record handling failed");
        for(auto&i:rows){
            if(i.id=="100"){require(i.type=="scene","case normalization failed");require(i.reason.empty(),"valid packed scene rejected");require(!i.preview.empty(),"preview missing");}
            if(i.id=="200")require(i.type=="preset"&&!i.reason.empty(),"dependency preset incorrectly accepted");
            if(i.id=="300"){require(!i.reason.empty(),"missing video accepted");require(i.preview.empty(),"escaping preview accepted");}
            if(i.id=="400")require(i.type=="invalid","invalid JSON not isolated");
        }
        fs::remove_all(root);std::cout<<"Catalog fixtures passed\n";return 0;
    }catch(const std::exception&e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
