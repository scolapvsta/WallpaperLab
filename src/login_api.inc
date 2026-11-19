// Included inside the controller's C API, sharing the catalog without rescanning it.
static void ensureLogin(){if(!loginLoaded){try{loginDocument=login::load();}catch(const std::exception& e){loginDocument=login::defaults();errorMessage=e.what();loginNotice="Operazione non riuscita. Dettagli nel registro.";std::cerr<<"Native login: "<<e.what()<<'\n';try{writeJson(stateDir()/"login-screen/error.json",{{"error",errorMessage}});}catch(...){};}for(auto key:{"output","ui_primary"}){auto selected=loginDocument.value(key,std::string());for(size_t n=0;n<monitors.size();n++)if(selected==monitorLabels[n])loginDocument[key]=monitors[n];if(loginDocument[key]==""&&!monitors.empty())loginDocument[key]=monitors.front();}loginLoaded=true;}}
static void loginChangedNow(){loginDirty=true;loginChanged=std::chrono::steady_clock::now();loginRevision++;}
int lab_login_enabled(){try{ensureLogin();return loginDocument.value("enabled",false);}catch(...){return 0;}}
unsigned lab_login_revision(){ensureLogin();return loginRevision;}
int lab_login_system_active(){if(getenv("WALLPAPER_LAB_UI_READ_ONLY"))return 0;try{return fs::read_symlink("/etc/systemd/system/display-manager.service").filename()=="greetd.service"&&fs::exists("/usr/local/lib/wallpaper-lab-login/installed.json");}catch(...){return 0;}}
int lab_login_restore(){
    if(getenv("WALLPAPER_LAB_UI_READ_ONLY"))return 0;
    if(loginJob.valid())return 0;loginNotice="Attendi la fine del caricamento";
    loginJob=std::async(std::launch::async,[]{auto installed=readJson("/usr/local/lib/wallpaper-lab-login/installed.json");auto helper=fs::read_symlink("/proc/self/exe").parent_path()/"login_deploy.py";auto response=capture({"pkexec","/usr/bin/python3",helper.string(),"rollback",installed.at("backup")});if(response.empty())throw std::runtime_error("Native login restoration failed");auto result=json::parse(response.front());if(!result.value("ok",false))throw std::runtime_error(result.value("error",std::string("Native login restoration failed")));result["disable"]=true;return result;});return 1;
}
int lab_login_enable(int enabled){if(!enabled&&lab_login_system_active())return lab_login_restore();return guarded([&]{ensureLogin();loginDocument["enabled"]=bool(enabled);loginChangedNow();login::save(loginDocument);loginDirty=false;});}
int lab_login_selected(){ensureLogin();auto source=login::sourceFor(loginDocument);for(size_t i=0;i<items.size();i++)if(items[i].path==source)return i;return -1;}
const char* lab_login_reason(int i){static std::string reason;if(i<0||i>=int(items.size()))return "";reason=items[i].reason.empty()?lab_tr(login::unsupportedReason(items[i]).c_str()):localizedDiagnostic(items[i].reason);return reason.c_str();}
int lab_login_move(int widget,float x,float y){return guarded([&]{ensureLogin();auto position=login::snapPosition(loginDocument,widget,x,y);auto& w=loginDocument["widgets"][login::widgets.at(widget)];if(w["x"]==position[0]&&w["y"]==position[1])return;w["x"]=position[0];w["y"]=position[1];loginChangedNow();});}
int lab_login_supported(int i){return i>=0&&i<int(items.size())&&login::supported(items[i]);}
int lab_login_select(int i){return guarded([&]{ensureLogin();if(!lab_login_supported(i))throw std::invalid_argument("Wallpaper is unsupported for native login");if(loginDocument.value("mode",std::string())=="single")loginDocument["wallpapers"][loginDocument["output"].get<std::string>()]=items[i].path;else loginDocument["source"]=items[i].path;loginChangedNow();});}
int lab_login_output_selected(int output){ensureLogin();if(output<0||output>=int(monitors.size()))return -1;auto source=login::sourceFor(loginDocument,monitors[output]);for(size_t i=0;i<items.size();i++)if(items[i].path==source)return i;return -1;}
int lab_login_display_rect(int i,float* x,float* y,float* w,float* h){auto canvas=displayCanvas(displayData,settings);if(i<0||i>=int(canvas.rects.size()))return 0;auto& r=canvas.rects[i];*x=r.x;*y=r.y;*w=r.w;*h=r.h;return 1;}
int lab_login_display_pixels(int i,float* w,float* h){if(i<0||i>=int(monitors.size()))return 0;for(const auto& display:displayData)if(display.name==monitors[i]){*w=display.width;*h=display.height;return *w>0&&*h>0;}return 0;}
float lab_login_number(const char* path){try{ensureLogin();auto v=loginDocument.at(json::json_pointer(path));return v.is_boolean()?float(v.get<bool>()):v.get<float>();}catch(...){return 0;}}
const char* lab_login_text(const char* path){static std::string value;try{ensureLogin();value=loginDocument.at(json::json_pointer(path)).get<std::string>();}catch(...){value.clear();}return value.c_str();}
int lab_login_set_number(const char* path,float value){return guarded([&]{ensureLogin();auto next=loginDocument;auto pointer=json::json_pointer(path);auto replacement=next.at(pointer).is_boolean()?json(bool(value)):json(value);if(next.at(pointer)==replacement)return;next.at(pointer)=replacement;loginDocument=login::validate(next);loginChangedNow();});}
int lab_login_set_text(const char* path,const char* value){return guarded([&]{ensureLogin();auto next=loginDocument;auto pointer=json::json_pointer(path);if(!next.at(pointer).is_string())throw std::invalid_argument("Login field is not text");if(next.at(pointer)==value)return;next.at(pointer)=value;loginDocument=login::validate(next);loginChangedNow();});}
int lab_login_reset_ui(int widget){return guarded([&]{ensureLogin();writeJson(stateDir()/"backups"/("login-ui-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"login-screen.json",loginDocument);loginDocument=login::resetUi(loginDocument,widget);loginChangedNow();});}
static json loginPreviousColors;
int lab_login_palette(const unsigned char* rgb,int width,int height){return guarded([&]{ensureLogin();if(width<1||height<1||width*height>4096)return;auto next=login::applyPalette(loginDocument,rgb,width,height);if(next!=loginDocument){loginPreviousColors=login::colors(loginDocument);loginDocument=next;loginChangedNow();loginNotice="Colori suggeriti. Opacità e disposizione invariate.";}});}
int lab_login_palette_undo(){return guarded([&]{if(loginPreviousColors.is_null())return;loginDocument=login::restoreColors(loginDocument,loginPreviousColors);loginPreviousColors=nullptr;loginChangedNow();});}
int lab_login_palette_can_undo(){return !loginPreviousColors.is_null();}
static std::vector<json> loginPresets[2];static bool loginPresetsLoaded[2]={};
static void refreshLoginPresets(int full){loginPresets[full]=login::listPresets(full);loginPresetsLoaded[full]=true;}
void lab_login_preset_refresh(int full){if(full>=0&&full<2)refreshLoginPresets(full);}
int lab_login_preset_count(int full){if(full<0||full>1)return 0;if(!loginPresetsLoaded[full])refreshLoginPresets(full);return loginPresets[full].size();}
const char* lab_login_preset_name(int full,int index){if(index<0||index>=lab_login_preset_count(full))return "";return loginPresets[full][index]["name"].get_ref<const std::string&>().c_str();}
int lab_login_preset_save(int full,const char* name){return guarded([&]{if(full<0||full>1)return;ensureLogin();if(full&&lab_login_selected()<0)throw std::invalid_argument("Preset wallpaper is unavailable");login::savePreset(loginDocument,name,full);refreshLoginPresets(full);loginNotice="Preset salvato.";});}
int lab_login_preset_load(int full,int index){return guarded([&]{ensureLogin();if(index<0||index>=lab_login_preset_count(full))throw std::invalid_argument("Invalid login preset");auto next=login::applyPreset(loginDocument,loginPresets[full][index],full,items);writeJson(stateDir()/"backups"/("login-preset-load-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"login-screen.json",loginDocument);loginDocument=next;loginPreviousColors=nullptr;loginChangedNow();loginNotice="Preset caricato nella bozza.";
    // Report a Fontconfig substitution once on load, never on every preview frame.
    std::set<std::string> checked;for(auto id:login::widgets){auto family=loginDocument["widgets"][id].value("font",std::string());if(family.empty())family=loginDocument.value("font",std::string());if(family.empty()||!checked.insert(family).second)continue;
        auto query=FcPatternCreate();FcPatternAddString(query,FC_FAMILY,reinterpret_cast<const FcChar8*>(family.c_str()));FcConfigSubstitute(nullptr,query,FcMatchPattern);FcDefaultSubstitute(query);FcResult result;auto match=FcFontMatch(nullptr,query,&result);bool found=false;if(match){for(int n=0;;n++){FcChar8* actual=nullptr;if(FcPatternGetString(match,FC_FAMILY,n,&actual)!=FcResultMatch)break;if(FcStrCmpIgnoreCase(actual,reinterpret_cast<const FcChar8*>(family.c_str()))==0)found=true;}FcPatternDestroy(match);}FcPatternDestroy(query);if(!found){loginNotice=std::string(translate("Font dell'interfaccia"))+" · "+translate("Non disponibile")+": "+family;break;}
    }
});}
int lab_login_preset_folder(int full){return guarded([&]{if(full<0||full>1)return;auto dir=login::presetDirectory(full);fs::create_directories(dir);capture({"xdg-open",dir.string()});refreshLoginPresets(full);});}
int lab_login_variant_selected(int i){ensureLogin();auto choice=loginDocument["variants"].value(items.at(i).path,json::object());for(size_t n=0;n<items[i].variants.size();n++){auto& v=items[i].variants[n];if(choice.value("property",std::string())==v.property&&choice.value("value",std::string())==v.value)return n;}return -1;}
int lab_login_variant_set(int i,int v){return guarded([&]{ensureLogin();if(v<0)loginDocument["variants"].erase(items.at(i).path);else{auto& choice=items.at(i).variants.at(v);loginDocument["variants"][items[i].path]={{"property",choice.property},{"value",choice.value}};}loginChangedNow();});}
int lab_login_save(){if(!loginLoaded||!loginDirty)return 1;return guarded([&]{login::save(loginDocument);loginDirty=false;loginNotice="Bozza salvata. Applica per aggiornare il login all’avvio.";});}
float lab_login_image(const char* key){ensureLogin();return login::imageProfile(loginDocument).at(key);}
int lab_login_image_set(const char* key,float value){return guarded([&]{ensureLogin();auto next=loginDocument;if(next.value("mode",std::string())=="single")next["output_profiles"][next["output"].get<std::string>()][login::sourceFor(next)][key]=value;else next["profiles"][login::sourceFor(next)][key]=value;loginDocument=login::validate(next);loginChangedNow();});}
const unsigned char* lab_login_pixels(int preset,int width,int height){
    static login::Pixels pixels;try{ensureLogin();auto d=loginDocument;login::CanvasState display;int item=lab_login_selected();if(item>=0)display.title=items[item].title;display.preview=false;display.focus=-1;display.username="User";pixels=login::draw(d,width,height,display,std::time(nullptr));return pixels.rgba.data();}catch(...){return nullptr;}
}
static std::future<json> loginStillJob;
static std::set<std::string> loginStillFailed;
static std::string loginStillStatus;
float lab_login_output_image(int output,const char* key){ensureLogin();if(output<0||output>=int(monitors.size()))return 0;return login::imageProfile(loginDocument,monitors[output]).at(key);}
const char* lab_login_still_notice(){return lab_tr(loginStillStatus.c_str());}
static void pollLoginStill(){
 if(!loginStillJob.valid()||loginStillJob.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return;
 try{auto result=loginStillJob.get();if(result.value("ok",false))loginStillStatus="Fotogramma della scena";else{loginStillFailed.insert(result.at("key"));loginStillStatus="Miniatura del Workshop";}}catch(...){loginStillStatus="Miniatura del Workshop";}
}
const char* lab_login_still(int index){
 static std::string path;pollLoginStill();path.clear();if(index<0||index>=int(items.size())||!login::supported(items[index]))return path.c_str();
 const auto item=items[index];auto profiles=placements.value(item.path,json::object()).value("properties",json::object());
 auto variants=loginDocument.value("variants",json::object());auto key=std::to_string(std::hash<std::string>{}(item.path+profiles.dump()+variants.value(item.path,json::object()).dump()));auto destination=stateDir()/"login-screen/stills"/(key+".png");
 if(fs::is_regular_file(destination)){path=destination.string();return path.c_str();}
 if(loginStillJob.valid()||loginStillFailed.contains(key))return path.c_str();
 auto desktop=settings;auto config=login::defaults();config["source"]=item.path;config["mode"]="mirror";config["variants"]=variants;
 loginStillStatus="Attendi la fine del caricamento";
 loginStillJob=std::async(std::launch::async,[item,desktop,config,key,destination]{
  json process;fs::path dir;
  try{
   auto file=login::prepare(config,item,desktop,{"default"});dir=file.parent_path();auto d=readJson(file);d["owner"]={{"pid",getpid()},{"start",startTime(getpid())},{"exe",executable()}};
   auto args=login::renderArguments(d,true);args.insert(args.begin()+1,"--render");
   int width=item.width>0?item.width:1920,height=item.height>0?item.height:1080;double factor=std::min({1.,1280./width,720./height});width=std::max(32,int(width*factor));height=std::max(32,int(height*factor));
   auto geometry=std::find(args.begin(),args.end(),"--window");*(geometry+1)="0x0x"+std::to_string(width)+"x"+std::to_string(height);
   args.insert(args.end(),{"--lab-hidden","--lab-feed-dir",dir.string()});std::vector<char*> av;for(auto& arg:args)av.push_back(arg.data());av.push_back(nullptr);
   posix_spawnattr_t attr;posix_spawnattr_init(&attr);posix_spawnattr_setflags(&attr,POSIX_SPAWN_SETPGROUP);posix_spawnattr_setpgroup(&attr,0);posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);posix_spawn_file_actions_addopen(&actions,STDOUT_FILENO,(dir/"still.log").c_str(),O_CREAT|O_WRONLY|O_TRUNC,0600);posix_spawn_file_actions_adddup2(&actions,STDOUT_FILENO,STDERR_FILENO);
   pid_t pid=-1;int error=posix_spawn(&pid,args[0].c_str(),&actions,&attr,av.data(),environ);posix_spawn_file_actions_destroy(&actions);posix_spawnattr_destroy(&attr);if(error)throw std::runtime_error("Cannot start native login preview");process={{"pid",pid},{"start",startTime(pid)}};
   auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);bool captured=false;
   while(std::chrono::steady_clock::now()<deadline&&owned(process)){
    writeJson(dir/"request.json",{{"path",item.path},{"output","default"},{"transition",true}});
    std::ifstream input(dir/"frame-default.bin",std::ios::binary);uint32_t header[5]{};input.read(reinterpret_cast<char*>(header),sizeof(header));
    if(input&&header[0]==0x574c4631&&header[1]==uint32_t(pid)&&header[2]>0&&header[2]<=1280&&header[3]>0&&header[3]<=720&&header[4]>=4){
     std::vector<char> pixels(size_t(header[2])*header[3]*3);input.read(pixels.data(),pixels.size());if(input){fs::create_directories(destination.parent_path());auto temp=destination;temp+=".tmp";std::vector<unsigned char> rgba(size_t(header[2])*header[3]*4);for(size_t n=0;n<pixels.size()/3;n++){rgba[n*4]=pixels[n*3+2];rgba[n*4+1]=pixels[n*3+1];rgba[n*4+2]=pixels[n*3];rgba[n*4+3]=255;}auto surface=cairo_image_surface_create_for_data(rgba.data(),CAIRO_FORMAT_ARGB32,header[2],header[3],header[2]*4);auto status=cairo_surface_write_to_png(surface,temp.c_str());cairo_surface_destroy(surface);if(status!=CAIRO_STATUS_SUCCESS)throw std::runtime_error("Native login preview failed; see renderer.log");fs::rename(temp,destination);captured=true;break;}}
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
   }
   stop(process);return json{{"key",key},{"ok",captured}};
  }catch(...){stop(process);return json{{"key",key},{"ok",false}};}
 });return path.c_str();
}
const char* lab_login_shader(){static auto path=(fs::read_symlink("/proc/self/exe").parent_path()/"login-assets/preview.frag").string();return path.c_str();}
int lab_login_busy(){return loginJob.valid();}
const char* lab_login_notice(){return loginNotice.c_str();}
void lab_login_stop_preview(){
    stop(loginPreviewProcess);if(loginPreviewProcess.contains("pid")){int status;waitpid(loginPreviewProcess["pid"].get<int>(),&status,WNOHANG);}loginPreviewProcess=json::object();
    if(!loginPreviewDirectory.empty()){std::error_code ec;fs::remove_all(loginPreviewDirectory,ec);loginPreviewDirectory.clear();}
}
int lab_login_preview(int windowed){
    if(loginJob.valid())return 0;int selected=lab_login_selected();if(selected<0||!lab_login_save())return 0;
    auto config=loginDocument;auto item=items[selected];auto desktop=settings;auto outputs=monitors;auto catalog=items;
    loginNotice="Attendi la fine del caricamento";
    loginJob=std::async(std::launch::async,[config,item,desktop,outputs,catalog,windowed]{
        auto file=login::prepare(config,item,desktop,outputs,catalog),dir=file.parent_path();auto d=readJson(file);d["owner"]={{"pid",getpid()},{"start",startTime(getpid())},{"exe",executable()}};writeJson(file,d);
        auto ready=dir/"ready";std::vector<std::string> args={executable(),"--login-preview",file.string(),windowed?"--window":"--outputs",ready.string()};std::vector<char*> av;for(auto& a:args)av.push_back(a.data());av.push_back(nullptr);
        posix_spawnattr_t attr;posix_spawnattr_init(&attr);posix_spawnattr_setflags(&attr,POSIX_SPAWN_SETPGROUP);posix_spawnattr_setpgroup(&attr,0);
        posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);posix_spawn_file_actions_addopen(&actions,STDOUT_FILENO,(dir/"renderer.log").c_str(),O_CREAT|O_WRONLY|O_TRUNC,0600);posix_spawn_file_actions_adddup2(&actions,STDOUT_FILENO,STDERR_FILENO);
        pid_t pid=-1;int error=posix_spawn(&pid,args[0].c_str(),&actions,&attr,av.data(),environ);posix_spawn_file_actions_destroy(&actions);posix_spawnattr_destroy(&attr);
        if(error)throw std::runtime_error("Cannot start native login preview");json process={{"pid",pid},{"start",startTime(pid)}};
        try{
            auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(25);while(std::chrono::steady_clock::now()<deadline&&owned(process)&&!fs::exists(ready))std::this_thread::sleep_for(std::chrono::milliseconds(40));
            if(!fs::exists(ready)||!owned(process))throw std::runtime_error("Native login preview failed; see renderer.log");return json{{"preview",process},{"directory",dir.string()}};
        }catch(...){stop(process);int status;waitpid(pid,&status,WNOHANG);throw;}
    });return 1;
}
int lab_login_publish(){
    if(getenv("WALLPAPER_LAB_UI_READ_ONLY"))return 0;
    if(loginJob.valid())return 0;int selected=lab_login_selected();if(selected<0||!lab_login_save())return 0;
    auto config=loginDocument;auto item=items[selected];auto desktop=settings;auto outputs=monitors;auto catalog=items;loginNotice="Attendi la fine del caricamento";
    loginJob=std::async(std::launch::async,[config,item,desktop,outputs,catalog]{auto file=login::prepare(config,item,desktop,outputs,catalog);auto helper=fs::read_symlink("/proc/self/exe").parent_path()/"login_deploy.py";
        auto invoke=[&](std::vector<std::string> args){auto response=capture(args);if(response.empty())throw std::runtime_error("Native login publication failed");auto result=json::parse(response.front());if(!result.value("ok",false))throw std::runtime_error(result.value("error",std::string("Native login publication failed")));return result;};
        auto prepared=invoke({"python3",helper.string(),"prepare",file.string()});writeJson(stateDir()/"login-screen/publication.json",prepared);
        auto result=invoke({"pkexec","/usr/bin/python3",helper.string(),"install",prepared.at("package")});result["draft"]=config;return result;});return 1;
}
void lab_login_poll(){
    pollLoginStill();
    if(loginDirty&&std::chrono::steady_clock::now()-loginChanged>std::chrono::milliseconds(350))lab_login_save();
    if(!loginPreviewProcess.empty()&&!owned(loginPreviewProcess))lab_login_stop_preview();
    if(loginJob.valid()&&loginJob.wait_for(std::chrono::seconds(0))==std::future_status::ready){
        try{auto result=loginJob.get();if(result.contains("preview")){lab_login_stop_preview();loginPreviewProcess=result["preview"];loginPreviewDirectory=result["directory"].get<std::string>();loginNotice="Anteprima";}
            else{if(result.value("disable",false)){loginDocument["enabled"]=false;loginChangedNow();lab_login_save();}loginNotice=result.value("next_boot",false)?(result.value("draft",json::object())==loginDocument?"Login salvato per il prossimo avvio.":"Bozza salvata. Applica per aggiornare il login all’avvio."):"Modifiche applicate e salvate.";writeJson(stateDir()/"login-screen/publication.json",result);}}
        catch(const std::exception& e){errorMessage=e.what();loginNotice="Operazione non riuscita. Dettagli nel registro.";std::cerr<<"Native login: "<<e.what()<<'\n';try{writeJson(stateDir()/"login-screen/error.json",{{"error",errorMessage}});}catch(...){};}
    }
}
