#include "WallpaperEngine/Render/Helpers/VisibleFullscreen.h"
#include <iostream>
#include <stdexcept>
using nlohmann::json;
using WallpaperEngine::Render::Helpers::visibleFullscreen;
void check(bool value){if(!value)throw std::runtime_error("fullscreen visibility policy failed");}
int main(){
 auto monitors=json::parse(R"([{"id":0,"name":"DP-1","activeWorkspace":{"name":"1"},"specialWorkspace":{"name":""}},{"id":1,"name":"DP-2","activeWorkspace":{"name":"2"},"specialWorkspace":{"name":"special:test"}}])");
 auto clients=json::parse(R"([{"monitor":0,"workspace":{"name":"1"},"fullscreen":2,"mapped":true,"hidden":false,"class":"game","focusHistoryID":1},{"monitor":1,"workspace":{"name":"2"},"fullscreen":2,"mapped":true,"hidden":false,"class":"other","focusHistoryID":0}])");
 auto any=[](bool,const std::string&){return true;};
 auto active=[](bool focused,const std::string&){return focused;};
 auto filtered=[](bool,const std::string& app){return app!="game";};
 check(visibleFullscreen(monitors,clients,{"DP-1"},any));
 check(!visibleFullscreen(monitors,clients,{"DP-1"},active));
 check(visibleFullscreen(monitors,clients,{"DP-2"},active));
 check(!visibleFullscreen(monitors,clients,{"DP-1"},filtered));
 check(visibleFullscreen(monitors,clients,{"DP-1","DP-2"},any));
 clients[0]["workspace"]["name"]="hidden";
 check(!visibleFullscreen(monitors,clients,{"DP-1"},any));
 check(!visibleFullscreen(monitors,clients,{"DP-1","DP-2"},any));
 check(visibleFullscreen(monitors,clients,{},any));
 clients[1]["workspace"]["name"]="special:test";
 check(visibleFullscreen(monitors,clients,{"DP-2"},any));
 monitors[1]["specialWorkspace"]["name"]="";
 check(!visibleFullscreen(monitors,clients,{"DP-2"},any));
 clients[0]["workspace"]["name"]="1";clients[0]["fullscreen"]=1;
 check(!visibleFullscreen(monitors,clients,{"DP-1"},any));
 clients[0]["fullscreen"]=2;clients[0]["hidden"]=true;
 check(!visibleFullscreen(monitors,clients,{"DP-1"},any));
 clients[0]["hidden"]=false;clients[0]["mapped"]=false;
 check(!visibleFullscreen(monitors,clients,{"DP-1"},any));
 std::cout<<"Visible/hidden workspaces, output coverage, focus and exclusions passed\n";
}
