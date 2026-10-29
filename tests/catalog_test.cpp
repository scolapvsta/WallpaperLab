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
        writeJson(root/"500/project.json",{{"title","Metadata fixture"},{"type","Scene"},{"file","scene.json"},{"general",{{"supportsaudioprocessing",true}}}});
        std::string packed;
        auto u32=[&](uint32_t n){for(int b=0;b<4;b++)packed.push_back((char)(n>>(b*8)));};
        auto str=[&](const std::string& text){u32(text.size());packed+=text;};
        auto scene=json({{"general",{{"orthogonalprojection",{{"width",1920},{"height",1080}}}}}}).dump();
        str("PKGV0001");u32(1);str("scene.json");u32(0);u32(scene.size());packed+=scene;
        {std::ofstream file(root/"500/scene.pkg",std::ios::binary);file.write(packed.data(),packed.size());}
        auto rows=scan({root,root});require(rows.size()==5,"deduplication or malformed record handling failed");
        auto favorites=scanProjects({root/"100",root/"100",root/"missing",root/"400"});
        require(favorites.size()==2,"favorite-only scan must skip missing paths and duplicates");
        for(auto& item:favorites)require(item.id=="100"||item.id=="400","favorite scan included an unrelated project");
        for(auto&i:rows){
            if(i.id=="100"){require(i.type=="scene","case normalization failed");require(i.reason.empty(),"valid packed scene rejected");require(!i.preview.empty(),"preview missing");}
            if(i.id=="200")require(i.type=="scene"&&i.reason.empty(),"installed dependency preset not resolved");
            if(i.id=="300"){require(!i.reason.empty(),"missing video accepted");require(i.preview.empty(),"escaping preview accepted");}
            if(i.id=="400")require(i.type=="invalid","invalid JSON not isolated");
        }
        auto meta=scanProjects({root/"500"}).front();
        require(meta.width==1920&&meta.height==1080&&meta.audio==1,"packed scene metadata was not read");
        require(matches(meta,1,1,1)&&!matches(meta,2,0,0)&&!matches(meta,0,2,0)&&!matches(meta,0,0,2),"combined filters failed");
        meta.width=3440;meta.height=1440;require(matches(meta,0,4,0)&&!matches(meta,0,2,0),"ultrawide classified as exact QHD");
        meta.width=1080;meta.height=1920;require(matches(meta,0,5,0)&&!matches(meta,0,1,0),"portrait resolution filter failed");
        meta.width=meta.height=0;meta.audio=-1;
        require(matches(meta,0,6,3)&&!matches(meta,0,1,0)&&!matches(meta,0,0,2),"unknown metadata guessed from previews");
        std::vector<Item> formats;
        for(auto [w,h]:std::vector<std::pair<int,int>>{{3840,1080},{5120,1440},{7680,2160},{3440,1440},{1130,665},{1920,1080},{0,0}}){auto item=meta;item.width=w;item.height=h;formats.push_back(item);}
        auto filters=resolutionFilters(formats);
        for(int code:{7,8,9}){auto f=std::find_if(filters.begin(),filters.end(),[&](auto& f){return f.code==code;});require(f!=filters.end()&&f->count==1,"dual resolution count failed");require(!resolutionMatches(formats[5],*f),"single display leaked into dual filter");}
        require(std::none_of(filters.begin(),filters.end(),[](auto& f){return f.width==1130;}),"uncommon library dimensions leaked into curated filters");
        require(std::count_if(filters.begin(),filters.end(),[](auto& f){return f.width==1920&&f.height==1080;})==1,"standard resolution duplicated");
        writeJson(root/"600/project.json",{{"title","Multi-resolution"},{"type","scene"},{"file","scene.json"},{"tags",json::array({"1920 x 1080","2560 x 1440"})},{"general",{{"properties",{{"resolution",{{"type","combo"},{"text","Resolution"},{"options",json::array({{{"label","1920 x 1080"},{"value","hd"}},{{"label","3840 × 2160"},{"value","uhd"}}})}}}}}}}});
        writeJson(root/"600/scene.json",{{"general",{{"orthogonalprojection",{{"width",1920},{"height",1080}}}}}});
        auto multi=scanProjects({root/"600"}).front();
        require(multi.supported.size()==3&&multi.variants.size()==2,"declared resolutions or selectable property variants missing");
        require(resolutionAll(multi,filters,{1,2,3}),"AND filter did not accept all declared resolutions");
        require(!resolutionAll(multi,filters,{1,8}),"AND filter accepted an unsupported resolution");
        require(!resolutionAll(formats[5],filters,{1,2}),"single native size falsely treated as multiple supported sizes");
        require(matchesCriteria(multi,filters,0,0,{},true)&&matchesCriteria(multi,filters,0,0,{},false),"empty filter query excluded items");
        require(matchesCriteria(multi,filters,0,0,{1,8},true)&&!matchesCriteria(multi,filters,0,0,{1,8},false),"resolution OR/AND semantics failed");
        require(matchesCriteria(multi,filters,2,0,{1},true)&&!matchesCriteria(multi,filters,2,0,{1},false),"type/resolution OR/AND semantics failed");
        require(matchesCriteria(formats[5],filters,0,0,{1,2,3},false,1),"whole single-monitor group must match any resolution inside that group");
        require(!matchesCriteria(formats[5],filters,0,0,{1,2,3,7,8},false,3),"two selected groups must both match in AND mode");
        require(matchesCriteria(formats[5],filters,0,0,{1,2,3,7,8},true,3),"two selected groups must be alternatives in OR mode");
        require(!matchesCriteria(multi,filters,1,0,{},false,0,2)&&matchesCriteria(multi,filters,1,0,{},true,0,2),"origin AND/OR did not combine with wallpaper type");
        require(!matchesCriteria(multi,filters,0,0,{999},true),"unknown criterion matched");
        packed.resize(packed.size()-6);{std::ofstream file(root/"500/scene.pkg",std::ios::binary);file.write(packed.data(),packed.size());}
        auto truncated=scanProjects({root/"500"}).front();
        require(truncated.width==0&&truncated.reason.empty(),"optional malformed metadata rejected a project or read past package");
        fs::remove_all(root);std::cout<<"Catalog fixtures passed\n";return 0;
    }catch(const std::exception&e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
