#include "catalog.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
namespace lab {
static fs::path home() { const char* p = getenv("HOME"); if (!p) throw std::runtime_error("HOME is not set"); return p; }
fs::path configDir() { auto p = getenv("XDG_CONFIG_HOME"); return (p ? fs::path(p) : home()/".config")/"wallpaper-lab"; }
fs::path stateDir() { auto p = getenv("XDG_STATE_HOME"); return (p ? fs::path(p) : home()/".local/state")/"wallpaper-lab"; }
json readJson(const fs::path& path) {
    if (fs::file_size(path) > 4*1024*1024) throw std::runtime_error("JSON exceeds 4 MiB: " + path.string());
    std::ifstream f(path); return json::parse(f);
}
void writeJson(const fs::path& path, const json& value) {
    fs::create_directories(path.parent_path());
    auto tmp = path.string()+".tmp."+std::to_string(getpid());
    { std::ofstream f(tmp); f << value.dump(2) << '\n'; if (!f) throw std::runtime_error("Cannot write " + tmp); }
    fs::rename(tmp,path);
}
std::vector<std::string> capture(const std::vector<std::string>& args) {
    int pipes[2]; if (pipe(pipes)) throw std::runtime_error("pipe failed");
    posix_spawn_file_actions_t fa; posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa,pipes[1],STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&fa,pipes[0]); posix_spawn_file_actions_addclose(&fa,pipes[1]);
    std::vector<char*> av; for (const auto& s:args) av.push_back(const_cast<char*>(s.c_str())); av.push_back(nullptr);
    pid_t pid; int err=posix_spawnp(&pid,av[0],&fa,nullptr,av.data(),environ);
    posix_spawn_file_actions_destroy(&fa); close(pipes[1]);
    if (err) { close(pipes[0]); return {}; }
    std::string out; char buf[4096]; ssize_t n;
    while ((n=read(pipes[0],buf,sizeof(buf)))>0) out.append(buf,n);
    close(pipes[0]); int status; while(waitpid(pid,&status,0)<0 && errno==EINTR) {}
    if (!WIFEXITED(status) || WEXITSTATUS(status)) return {};
    return {out};
}
std::vector<fs::path> steamLibraries() {
    std::vector<fs::path> seeds={home()/".local/share/Steam",home()/".steam/steam",home()/".var/app/com.valvesoftware.Steam/.local/share/Steam"};
    std::set<fs::path> paths;
    for (auto p:seeds) if(fs::is_directory(p)) paths.insert(fs::canonical(p));
    const std::regex re("\"path\"\\s*\"([^\"]+)\"");
    for (auto p:seeds) {
        std::ifstream f(p/"steamapps/libraryfolders.vdf");
        std::string text((std::istreambuf_iterator<char>(f)),{});
        for (auto i=std::sregex_iterator(text.begin(),text.end(),re); i!=std::sregex_iterator(); ++i)
            if(fs::is_directory((*i)[1].str())) paths.insert(fs::canonical((*i)[1].str()));
    }
    return {paths.begin(),paths.end()};
}
std::string assetsPath() {
    for (auto p:steamLibraries()) if(fs::is_directory(p/"steamapps/common/wallpaper_engine/assets")) return (p/"steamapps/common/wallpaper_engine/assets").string();
    return {};
}
static std::string lower(std::string s) { std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return std::tolower(c);});return s; }
static std::string stringValue(const json& j,const char* key,const std::string& fallback="") { auto i=j.find(key); return i!=j.end()&&i->is_string()?i->get<std::string>():fallback; }
std::vector<Item> scan(const std::vector<fs::path>& roots) {
    std::vector<Item> out; std::set<fs::path> seen;
    for(auto root:roots) {
        if(!fs::is_directory(root)) continue;
        for(const auto& entry:fs::directory_iterator(root,fs::directory_options::skip_permission_denied)) {
            if(!entry.is_directory() || !fs::is_regular_file(entry.path()/"project.json")) continue;
            auto path=fs::canonical(entry.path()); if(!seen.insert(path).second) continue;
            Item item; item.path=path.string(); item.id=path.filename().string(); item.title=item.id;
            try {
                auto j=readJson(path/"project.json"); item.title=stringValue(j,"title",item.id); item.type=lower(stringValue(j,"type"));
                auto preview=stringValue(j,"preview");
                if(!preview.empty()) {
                    auto target=fs::weakly_canonical(path/preview);
                    auto rel=target.lexically_relative(path);
                    if(!rel.empty() && *rel.begin()!=".." && fs::is_regular_file(target)) item.preview=target.string();
                }
                if(item.preview.empty()) for(auto name:{"preview.jpg","preview.gif","preview.png"}) if(fs::is_regular_file(path/name)) { item.preview=(path/name).string();break; }
                if(j.contains("dependency")) { item.type="preset"; item.reason="Preset con dipendenza Workshop: supporto non ancora implementato"; }
                else if(item.type.empty() && fs::exists(path/"scene.pkg")) item.type="scene";
                if(item.type!="scene" && item.type!="video" && item.type!="web" && item.reason.empty()) item.reason="Tipo di progetto non supportato";
                if(item.type=="scene" && !fs::exists(path/"scene.pkg") && !fs::exists(path/stringValue(j,"file","scene.json"))) item.reason="File della scena mancante";
                if(item.type=="video" || item.type=="web") {
                    auto f=stringValue(j,"file");
                    if(f.empty() || !fs::is_regular_file(path/f)) item.reason="File del progetto mancante";
                }
            } catch(const std::exception& e) {item.type="invalid"; item.reason=e.what();}
            out.push_back(std::move(item));
        }
    }
    std::sort(out.begin(),out.end(),[](const Item&a,const Item&b){return lower(a.title)<lower(b.title);});return out;
}
}
