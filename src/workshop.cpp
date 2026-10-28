#include "workshop.hpp"
#include "steam_abi.hpp"
#include <curl/curl.h>
#include <dlfcn.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <chrono>
#include <regex>
#include <fstream>
#include <cstring>
#include <functional>
#include <future>
#include <set>
using namespace std::chrono_literals;
namespace lab::workshop {
namespace {
using namespace steamabi;
std::mutex mutex;std::condition_variable wake;std::thread worker;
std::deque<json> commands;bool quitting=false;std::atomic<bool> installed{false};
json state={{"connected",false},{"busy",false},{"error",""},{"items",json::array()},{"downloads",json::object()},{"total",0},{"page",1},{"revision",0}};
void update(const std::function<void(json&)>& f){std::lock_guard lock(mutex);f(state);}
void* library=nullptr;void* ugc=nullptr;void* friends=nullptr;int pipe=0;bool initialized=false;
json knownAuthors=json::object();bool authorsLoaded=false;
template<class F> F symbol(const char* name){auto p=dlsym(library,name);if(!p)throw std::runtime_error(std::string("Steam API non disponibile: ")+name);return reinterpret_cast<F>(p);}
#define API(name,ret,...) symbol<ret(*)(__VA_ARGS__)>("SteamAPI_" #name)
#define UGC(name,ret,...) symbol<ret(*)(void*,__VA_ARGS__)>("SteamAPI_ISteamUGC_" #name)
#define FRIEND(name,ret,...) symbol<ret(*)(void*,__VA_ARGS__)>("SteamAPI_ISteamFriends_" #name)
size_t receive(char* data,size_t size,size_t count,void* opaque){auto& buffer=*static_cast<std::string*>(opaque);size_t n=size*count;if(buffer.size()+n>8*1024*1024)return 0;buffer.append(data,n);return n;}
int transferProgress(void* interruptible,curl_off_t,curl_off_t,curl_off_t,curl_off_t){std::lock_guard lock(mutex);return quitting||(interruptible&&!commands.empty());}
std::string http(const std::string& url,bool interruptible=false){
 auto* c=curl_easy_init();if(!c)throw std::runtime_error("HTTP non disponibile");std::string data;
 curl_easy_setopt(c,CURLOPT_URL,url.c_str());curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,receive);curl_easy_setopt(c,CURLOPT_WRITEDATA,&data);
 curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT,8L);curl_easy_setopt(c,CURLOPT_TIMEOUT,8L);curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,1L);curl_easy_setopt(c,CURLOPT_MAXREDIRS,3L);
 curl_easy_setopt(c,CURLOPT_PROTOCOLS_STR,"https");curl_easy_setopt(c,CURLOPT_REDIR_PROTOCOLS_STR,"https");curl_easy_setopt(c,CURLOPT_USERAGENT,"WallpaperLab/0.12");curl_easy_setopt(c,CURLOPT_NOPROGRESS,0L);curl_easy_setopt(c,CURLOPT_XFERINFOFUNCTION,transferProgress);curl_easy_setopt(c,CURLOPT_XFERINFODATA,interruptible?reinterpret_cast<void*>(1):nullptr);
 auto error=curl_easy_perform(c);long status=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&status);curl_easy_cleanup(c);
 if(error!=CURLE_OK||status!=200)throw std::runtime_error("Connessione Steam non riuscita (HTTP "+std::to_string(status)+")");return data;
}
std::string escape(const std::string& text){auto* c=curl_easy_init();auto* s=curl_easy_escape(c,text.c_str(),text.size());std::string r=s?s:"";curl_free(s);curl_easy_cleanup(c);return r;}
std::string idString(uint64_t id){return std::to_string(id);}
uint64_t validID(const std::string& s){if(s.empty()||s.size()>20||s.find_first_not_of("0123456789")!=std::string::npos)throw std::runtime_error("ID Steam non valido");return std::stoull(s);}
void connect(){
 if(!authorsLoaded){try{auto file=stateDir()/"workshop-authors.json";if(fs::is_regular_file(file)){auto cache=readJson(file);if(cache.is_object()&&cache.size()<=5000)knownAuthors=cache;}}catch(...){}authorsLoaded=true;}
 if(initialized)return;
 if(!library){std::vector<fs::path> paths;
  if(auto p=getenv("WALLPAPER_LAB_STEAM_API"))paths.emplace_back(p);
  paths.push_back(fs::read_symlink("/proc/self/exe").parent_path()/"lib/libsteam_api.so");
  paths.push_back(fs::read_symlink("/proc/self/exe").parent_path()/"libsteam_api.so");
  auto home=fs::path(getenv("HOME"));auto wx=home/".local/share/wallpaperx";
  if(fs::is_directory(wx))for(auto& e:fs::directory_iterator(wx))paths.push_back(e.path()/"resources/app.asar.unpacked/node_modules/steamworks.js/dist/linux64/libsteam_api.so");
  for(auto& p:paths)if(fs::is_regular_file(p)){library=dlopen(p.c_str(),RTLD_NOW|RTLD_LOCAL);if(library)break;}
  if(!library)throw std::runtime_error("Libreria Steamworks non trovata. Installa il runtime Steamworks (libsteam_api.so).");
 }
 setenv("SteamAppId","431960",1);setenv("SteamGameId","431960",1);
 char error[1024]={};int result=API(InitFlat,int,char*)(error);
 if(result!=0)throw std::runtime_error(std::string("Apri Steam e accedi con un account che possiede Wallpaper Engine. ")+error);
 auto apps=API(SteamApps_v008,void*)();if(!apps||!API(ISteamApps_BIsSubscribedApp,bool,void*,uint32_t)(apps,431960)){API(Shutdown,void)();throw std::runtime_error("Workshop richiede Steam aperto con un account che possiede Wallpaper Engine. Accedi a Steam con quell’account e riprova.");}
 initialized=true;ugc=API(SteamUGC_v020,void*)();friends=API(SteamFriends_v017,void*)();pipe=API(GetHSteamPipe,int)();
 if(!ugc||!friends||!pipe)throw std::runtime_error("Interfaccia Steam UGC non disponibile");
 API(ManualDispatch_Init,void)();update([](json& s){s["connected"]=true;s["error"]="";});
}
void pump(){
 API(ManualDispatch_RunFrame,void,int)(pipe);Callback cb{};
 while(API(ManualDispatch_GetNextCallback,bool,int,Callback*)(pipe,&cb)){
  if(cb.callback==3406&&cb.size>=int(sizeof(DownloadResult))){DownloadResult r;memcpy(&r,cb.data,sizeof(r));if(r.app==431960)update([&](json& s){auto key=idString(r.id);if(s["downloads"].contains(key)){s["downloads"][key]["result"]=r.result;if(r.result!=1){s["downloads"][key]["status"]="error";s["downloads"][key]["error"]="Steam: errore download "+std::to_string(r.result);}}});}
  API(ManualDispatch_FreeLastCallback,void,int)(pipe);
 }
}
// Manual dispatch's call-result lookup is safe to poll after RunFrame; consume
// exactly once and enforce timeout so network errors never block the interface.
template<class T>T awaitCall(uint64_t call,int callback){
 if(!call)throw std::runtime_error("Steam ha rifiutato la richiesta");
 auto deadline=std::chrono::steady_clock::now()+25s;
 while(std::chrono::steady_clock::now()<deadline){
  {std::lock_guard lock(mutex);if(quitting)throw std::runtime_error("Richiesta interrotta");}
  API(ManualDispatch_RunFrame,void,int)(pipe);T result{};bool failed=false;
  if(API(ManualDispatch_GetAPICallResult,bool,int,uint64_t,void*,int,int,bool*)(pipe,call,&result,sizeof(result),callback,&failed)){
   if(failed)throw std::runtime_error("Steam: errore di rete. Riprova.");return result;
  }
  std::this_thread::sleep_for(30ms);
 }
 throw std::runtime_error("Steam non ha risposto. Controlla la connessione e riprova.");
}
std::string plainDescription(std::string text){
 text=std::regex_replace(text,std::regex("\\[/?(?:h[123]|b|i|u|strike|spoiler|list|olist|table|tr|td|th|quote|code|url|img|previewyoutube|youtube|hr|\\*|color|size|noparse)(?:=[^\\]]*)?\\]",std::regex::icase),"");
 return text;
}
std::string authorName(uint64_t id){const char* name=FRIEND(GetFriendPersonaName,const char*,uint64_t)(friends,id);return name&&strcmp(name,"[unknown]")?name:idString(id);}
uint64_t resolveAuthor(std::string text){
 if(text.empty())return 0;
 if(text.find_first_not_of("0123456789")==std::string::npos){auto id=validID(text);if(id>76561197960265728ull)return id;throw std::runtime_error("Inserisci uno SteamID64 o il nome/profilo dell'autore");}
 std::string url;
 if(text.starts_with("https://steamcommunity.com/profiles/")||text.starts_with("https://steamcommunity.com/id/")){url=text;auto q=url.find('?');if(q!=std::string::npos)url.resize(q);if(url.back()!='/')url+='/';url+="?xml=1";}
 else {
  std::vector<std::string> ids;std::string lowered=text;for(auto& c:lowered)c=std::tolower(static_cast<unsigned char>(c));
  for(auto& [id,name]:knownAuthors.items()){std::string n=name;for(auto& c:n)c=std::tolower(static_cast<unsigned char>(c));if(n==lowered)ids.push_back(id);}
  if(ids.size()==1)return validID(ids.front());
  if(ids.size()>1)throw std::runtime_error("Nome autore non univoco. Scegli un suggerimento o usa il link del profilo Steam.");
  // Steam's public user-search endpoint requires a web login (HTTP 401).
  // Do not scrape cookies: resolve a public vanity URL or a known creator name.
  url="https://steamcommunity.com/id/"+escape(text)+"/?xml=1";
 }
 std::smatch match;auto xml=http(url);if(!std::regex_search(xml,match,std::regex("<steamID64>([0-9]+)</steamID64>")))throw std::runtime_error("Nome autore non trovato. Steam richiede il login web per cercare tutti i nomi: scegli un autore dai risultati o incolla il suo profilo/SteamID64.");return validID(match[1]);
}
void query(const json& q){
 uint64_t handle=~0ull;auto owner=resolveAuthor(q.value("author",std::string()));int page=std::clamp(q.value("page",1),1,1000);int sort=q.value("sort",3);
 std::string search=q.value("text",std::string());
 std::smatch idMatch;uint64_t direct=0;if(std::regex_search(search,idMatch,std::regex("^(?:https://steamcommunity\\.com/sharedfiles/filedetails/\\?id=)?([0-9]{8,20})(?:&.*)?$")))direct=validID(idMatch[1]);
 json rows=json::array();QueryResult result{};bool filterAuthor=(owner&&(!search.empty()||!q.value("tags",json::array()).empty()))||direct;unsigned matchesCount=0;int nativePage=filterAuthor?1:page;
 try{do {
 if(direct){handle=UGC(CreateQueryUGCDetailsRequest,uint64_t,uint64_t*,uint32_t)(ugc,&direct,1);search.clear();}
 else if(owner)handle=UGC(CreateQueryUserUGCRequest,uint64_t,uint32_t,int,int,int,uint32_t,uint32_t,uint32_t)(ugc,uint32_t(owner),0,0,sort==0?5:sort==19?3:0,431960,431960,nativePage);
 else handle=UGC(CreateQueryAllUGCRequestPage,uint64_t,int,int,uint32_t,uint32_t,uint32_t)(ugc,sort,0,431960,431960,page);
 if(handle==~0ull)throw std::runtime_error("Query Steam non valida");
  UGC(SetReturnLongDescription,bool,uint64_t,bool)(ugc,handle,true);UGC(SetReturnMetadata,bool,uint64_t,bool)(ugc,handle,true);UGC(SetReturnChildren,bool,uint64_t,bool)(ugc,handle,true);
  UGC(SetMatchAnyTag,bool,uint64_t,bool)(ugc,handle,q.value("any",false));UGC(SetRankedByTrendDays,bool,uint64_t,uint32_t)(ugc,handle,q.value("days",7));
  if(!search.empty()&&!owner&&!UGC(SetSearchText,bool,uint64_t,const char*)(ugc,handle,search.c_str()))throw std::runtime_error("Testo di ricerca non valido per Steam");
  if(!owner&&!direct)for(auto& tag:q.value("tags",json::array())){auto t=tag.get<std::string>();if(!UGC(AddRequiredTag,bool,uint64_t,const char*)(ugc,handle,t.c_str()))throw std::runtime_error("Filtro Steam rifiutato: "+t);}
  // Keep explicit content out of the initial browse, matching the safe default.
  if(!q.value("mature",false)){UGC(AddExcludedTag,bool,uint64_t,const char*)(ugc,handle,"Mature");UGC(AddExcludedTag,bool,uint64_t,const char*)(ugc,handle,"Nudity");UGC(AddExcludedTag,bool,uint64_t,const char*)(ugc,handle,"Sexual Content");}
  result=awaitCall<QueryResult>(UGC(SendQueryUGCRequest,uint64_t,uint64_t)(ugc,handle),3401);
  if(result.result!=1)throw std::runtime_error("Steam: ricerca non riuscita ("+std::to_string(result.result)+")");
  for(unsigned i=0;i<std::min(result.returned,50u);i++){
   Details d{};if(!UGC(GetQueryUGCResult,bool,uint64_t,uint32_t,Details*)(ugc,handle,i,&d)||d.result!=1||d.banned||d.consumerApp!=431960)continue;d.title[128]=0;d.description[7999]=0;if(owner&&d.owner!=owner)continue;
   if(owner&&!search.empty()){auto title=std::string(d.title)+" "+std::string(d.description);auto lower=[](std::string s){for(auto& c:s)c=std::tolower(static_cast<unsigned char>(c));return s;};if(lower(title).find(lower(search))==std::string::npos)continue;}
   FRIEND(RequestUserInformation,bool,uint64_t,bool)(friends,d.owner,true);
   char preview[2048]={},metadata[10001]={};UGC(GetQueryUGCPreviewURL,bool,uint64_t,uint32_t,char*,uint32_t)(ugc,handle,i,preview,sizeof(preview));UGC(GetQueryUGCMetadata,bool,uint64_t,uint32_t,char*,uint32_t)(ugc,handle,i,metadata,sizeof(metadata));
   json tags=json::array();auto num=UGC(GetQueryUGCNumTags,uint32_t,uint64_t,uint32_t)(ugc,handle,i);
   for(unsigned n=0;n<std::min(num,100u);n++){char tag[512]={};if(UGC(GetQueryUGCTag,bool,uint64_t,uint32_t,uint32_t,char*,uint32_t)(ugc,handle,i,n,tag,sizeof(tag)))tags.push_back(tag);}
   if(filterAuthor){bool matched=q.value("any",false)?false:true;auto required=q.value("tags",json::array());for(auto& tag:required){bool has=std::find(tags.begin(),tags.end(),tag)!=tags.end();matched=q.value("any",false)?matched||has:matched&&has;}if(!required.empty()&&!matched)continue;matchesCount++;if(matchesCount<=unsigned((page-1)*50)||matchesCount>unsigned(page*50))continue;}
   json children=json::array();if(d.children&&d.children<=256){std::vector<uint64_t> ids(d.children);if(UGC(GetQueryUGCChildren,bool,uint64_t,uint32_t,uint64_t*,uint32_t)(ugc,handle,i,ids.data(),ids.size()))for(auto id:ids)children.push_back(idString(id));}
   uint64_t subs=0,faves=0;UGC(GetQueryUGCStatistic,bool,uint64_t,uint32_t,int,uint64_t*)(ugc,handle,i,0,&subs);UGC(GetQueryUGCStatistic,bool,uint64_t,uint32_t,int,uint64_t*)(ugc,handle,i,1,&faves);
   uint64_t bytes=0;char folder[4096]={};uint32_t timestamp=0;bool local=UGC(GetItemInstallInfo,bool,uint64_t,uint64_t*,char*,uint32_t,uint32_t*)(ugc,d.id,&bytes,folder,sizeof(folder),&timestamp)&&fs::is_regular_file(fs::path(folder)/"project.json");
   rows.push_back({{"dependencies",children},{"plain_description",plainDescription(std::string(d.description,strnlen(d.description,sizeof(d.description))))},{"id",idString(d.id)},{"title",std::string(d.title,strnlen(d.title,sizeof(d.title)))},{"description",std::string(d.description,strnlen(d.description,sizeof(d.description)))},{"owner",idString(d.owner)},{"author",authorName(d.owner)},{"created",d.created},{"updated",d.updated},{"tags",tags},{"preview_url",preview},{"thumbnail",""},{"metadata",metadata},{"votes_up",d.votesUp},{"votes_down",d.votesDown},{"score",d.score},{"subscriptions",subs},{"favorites",faves},{"size",d.size>0?uint64_t(d.size):bytes},{"children",d.children},{"file_type",d.type},{"installed",local},{"folder",local?folder:""}});
  }
  UGC(ReleaseQueryUGCRequest,bool,uint64_t)(ugc,handle);handle=~0ull;
  if(!filterAuthor||nativePage*50>=int(result.total)||!result.returned||nativePage>=1000)break;
  nativePage++;pump();{std::lock_guard lock(mutex);if(quitting||!commands.empty()){state["busy"]=false;return;}}
 }while(true);
 if(filterAuthor)result.total=matchesCount;
  pump();for(auto& r:rows)r["author"]=authorName(validID(r["owner"]));
  update([&](json& s){s["items"]=rows;s["total"]=result.total;s["page"]=page;s["query"]=q;s["author_search"]=owner!=0;s["author_name_filter"]=owner!=0&&!search.empty();s["busy"]=false;s["revision"]=s["revision"].get<int>()+1;});
  // Disk cache uses Steam IDs, never remote file names. Only HTTPS images
  // from Steam's own CDN are fetched; no project code is executed in previews.
  for(size_t first=0;first<rows.size();first+=4){
   pump();{std::lock_guard lock(mutex);if(quitting||!commands.empty())break;}
   std::vector<std::future<void>> requests;
   for(size_t n=first;n<std::min(first+4,rows.size());n++){auto r=rows[n];
    std::string url=r["preview_url"];if(!url.starts_with("https://"))continue;
    auto host=url.substr(8,url.find('/',8)-8);if(!(host.ends_with(".steamstatic.com")||host.ends_with(".steamusercontent.com")||host=="steamuserimages-a.akamaihd.net"))continue;
    requests.push_back(std::async(std::launch::async,[r,url]{
     auto file=stateDir()/"workshop-cache"/(r["id"].get<std::string>()+"-"+std::to_string(r["updated"].get<uint32_t>())+".image");
     try{fs::create_directories(file.parent_path());if(!fs::is_regular_file(file)){auto data=http(url,true);auto temp=file;temp+=".tmp";std::ofstream out(temp,std::ios::binary);out.write(data.data(),data.size());out.close();if(!out)throw std::runtime_error("Cache non scrivibile");fs::rename(temp,file);}update([&](json& s){for(auto& entry:s["items"])if(entry["id"]==r["id"])entry["thumbnail"]=file.string();});}catch(...){}
    }));
   }
   for(auto& request:requests)request.get();
  }
 }catch(...){if(handle!=~0ull)UGC(ReleaseQueryUGCRequest,bool,uint64_t)(ugc,handle);throw;}
}
void requestDownload(const std::string& id,std::set<uint64_t>& visited,int depth=0){
 auto value=validID(id);if(depth>8||visited.size()>128)throw std::runtime_error("Troppe dipendenze nel progetto Steam");if(!visited.insert(value).second)return;
 // Verify the item belongs to Wallpaper Engine, rather than trusting a UI ID.
 auto handle=UGC(CreateQueryUGCDetailsRequest,uint64_t,uint64_t*,uint32_t)(ugc,&value,1);
 if(handle==~0ull)throw std::runtime_error("ID Workshop non disponibile");
 UGC(SetReturnChildren,bool,uint64_t,bool)(ugc,handle,true);
 try{auto result=awaitCall<QueryResult>(UGC(SendQueryUGCRequest,uint64_t,uint64_t)(ugc,handle),3401);Details d{};
 if(result.result!=1||!UGC(GetQueryUGCResult,bool,uint64_t,uint32_t,Details*)(ugc,handle,0,&d)||d.result!=1||d.consumerApp!=431960||d.banned||d.type==2)throw std::runtime_error("Elemento non scaricabile come wallpaper di Wallpaper Engine");
 json children=json::array();if(d.children>128)throw std::runtime_error("Troppe dipendenze nel progetto Steam");if(d.children){std::vector<uint64_t> ids(d.children);if(!UGC(GetQueryUGCChildren,bool,uint64_t,uint32_t,uint64_t*,uint32_t)(ugc,handle,0,ids.data(),ids.size()))throw std::runtime_error("Impossibile leggere le dipendenze Steam");for(auto child:ids)children.push_back(idString(child));}
 UGC(ReleaseQueryUGCRequest,bool,uint64_t)(ugc,handle);handle=~0ull;
 for(auto& child:children)requestDownload(child.get<std::string>(),visited,depth+1);
 auto subscribed=awaitCall<SubscribeResult>(UGC(SubscribeItem,uint64_t,uint64_t)(ugc,value),1313);
 if(subscribed.result!=1)throw std::runtime_error("Steam: sottoscrizione fallita ("+std::to_string(subscribed.result)+")");
 if(!UGC(DownloadItem,bool,uint64_t,bool)(ugc,value,false))throw std::runtime_error("Steam non ha avviato il download. Controlla account e spazio su disco.");
 update([&](json& s){s["downloads"][id]={{"status","downloading"},{"current",0},{"total",0},{"error",""},{"dependencies",children},{"dependency",depth>0},{"result",0}};});
 }catch(...){if(handle!=~0ull)UGC(ReleaseQueryUGCRequest,bool,uint64_t)(ugc,handle);throw;}
}
void progress(){
 json downloads;{std::lock_guard lock(mutex);downloads=state["downloads"];}
 for(auto& [id,entry]:downloads.items())if(entry.value("status",std::string())=="downloading"){
  auto value=validID(id);uint64_t current=0,total=0;
  UGC(GetItemDownloadInfo,bool,uint64_t,uint64_t*,uint64_t*)(ugc,value,&current,&total);
  auto flags=UGC(GetItemState,uint32_t,uint64_t)(ugc,value);
  uint64_t size=0;uint32_t timestamp=0;char folder[4096]={};
  bool complete=entry.value("result",0)==1&&(flags&4)&&!(flags&(8|16|32))&&UGC(GetItemInstallInfo,bool,uint64_t,uint64_t*,char*,uint32_t,uint32_t*)(ugc,value,&size,folder,sizeof(folder),&timestamp);
  bool dependenciesReady=true,dependencyError=false;
  for(auto& child:entry.value("dependencies",json::array())){auto dep=downloads.find(child.get<std::string>());if(dep==downloads.end()||dep->value("status",std::string())!="installed")dependenciesReady=false;if(dep!=downloads.end()&&dep->value("status",std::string())=="error")dependencyError=true;}
  if(dependencyError){update([&](json& s){s["downloads"][id]["status"]="error";s["downloads"][id]["error"]="Download di una dipendenza fallito. Premi Riprova download.";});continue;}
  bool mainFileReady=complete;complete=complete&&dependenciesReady;
  if(complete&&!entry.value("dependency",false)&&!fs::is_regular_file(fs::path(folder)/"project.json")){
   update([&](json& s){s["downloads"][id]["status"]="error";s["downloads"][id]["error"]="Scaricato da Steam, ma non contiene project.json: formato non importabile";});continue;
  }
  update([&](json& s){auto& d=s["downloads"][id];d["current"]=complete?size:current;d["total"]=mainFileReady?size:total;d["phase"]=mainFileReady&&!dependenciesReady?"dependencies":"content";if(complete){d["status"]="installed";d["folder"]=folder;for(auto& item:s["items"])if(item["id"]==id){item["installed"]=true;item["folder"]=folder;}}});
  if(complete&&fs::is_regular_file(fs::path(folder)/"project.json"))installed=true;
 }
 // Persona information arrives asynchronously, independently of catalog results.
 bool changed=false;
 update([&](json& s){for(auto& row:s["items"]){auto id=row["owner"].get<std::string>();auto name=authorName(validID(id));row["author"]=name;if(name!=id&&knownAuthors.value(id,std::string())!=name){knownAuthors[id]=name;changed=true;}}
  if(changed)s["names_revision"]=s.value("names_revision",0)+1;
  s["authors"]=json::array();for(auto& [id,name]:knownAuthors.items())s["authors"].push_back({{"id",id},{"name",name}});
 });
 if(changed&&knownAuthors.size()<=5000)try{writeJson(stateDir()/"workshop-authors.json",knownAuthors);}catch(...){}
}
void run(){
 curl_global_init(CURL_GLOBAL_DEFAULT);
 while(true){json command;
  {std::unique_lock lock(mutex);wake.wait_for(lock,150ms,[]{return quitting||!commands.empty();});if(quitting)break;if(!commands.empty()){command=commands.front();commands.pop_front();}}
  try{
   if(!command.is_null()){
    if(command["op"]=="search")update([](json& s){s["error"]="";s["busy"]=true;});
    connect();if(command["op"]=="search")query(command["query"]);
    else if(command["op"]=="download"){std::set<uint64_t> visited;requestDownload(command["id"],visited);}
    else if(command["op"]=="retry")connect();
   }
   if(initialized){pump();progress();}
  }catch(const std::exception& e){update([&](json& s){if(!command.is_null()&&command["op"]=="download"){auto id=command["id"].get<std::string>();s["downloads"][id]={{"status","error"},{"error",e.what()}};}else{s["error"]=e.what();s["busy"]=false;}});}
 }
 if(initialized)API(Shutdown,void)();if(library)dlclose(library);curl_global_cleanup();
}
void submit(json command){std::lock_guard lock(mutex);if(!worker.joinable())worker=std::thread(run);if(command["op"]=="search")commands.erase(std::remove_if(commands.begin(),commands.end(),[](const json& c){return c["op"]=="search";}),commands.end());commands.push_back(std::move(command));wake.notify_one();}
struct Cleanup {~Cleanup(){shutdown();}} cleanup;
}
void search(json query){update([](json& s){s["busy"]=true;s["error"]="";});submit({{"op","search"},{"query",query}});}
void download(const std::string& id){validID(id);update([&](json& s){s["downloads"][id]={{"status","queued"},{"current",0},{"total",0}};});submit({{"op","download"},{"id",id}});}
void retry(){submit({{"op","retry"}});}
json snapshot(){std::lock_guard lock(mutex);return state;}
bool takeInstalled(){return installed.exchange(false);}
void shutdown(){{std::lock_guard lock(mutex);quitting=true;wake.notify_all();}if(worker.joinable())worker.join();}
}
