// Included inside the controller's C API, sharing the catalog without rescanning it.
static void ensureLogin(){if(!loginLoaded){try{loginDocument=login::load();}catch(const std::exception& e){loginDocument=login::defaults();errorMessage=e.what();loginNotice="Operazione non riuscita. Dettagli nel registro.";std::cerr<<"Native login: "<<e.what()<<'\n';try{writeJson(stateDir()/"login-screen/error.json",{{"error",errorMessage}});}catch(...){};}loginLoaded=true;}}
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
int lab_login_selected(){ensureLogin();auto source=loginDocument.value("source",std::string());for(size_t i=0;i<items.size();i++)if(items[i].path==source)return i;return -1;}
int lab_login_supported(int i){return i>=0&&i<int(items.size())&&login::supported(items[i]);}
int lab_login_select(int i){return guarded([&]{ensureLogin();if(!lab_login_supported(i))throw std::invalid_argument("Wallpaper is unsupported for native login");loginDocument["source"]=items[i].path;loginChangedNow();});}
int lab_login_template(int preset){return guarded([&]{ensureLogin();loginDocument=login::applyTemplate(loginDocument,preset);loginChangedNow();});}
float lab_login_number(const char* path){try{ensureLogin();auto v=loginDocument.at(json::json_pointer(path));return v.is_boolean()?float(v.get<bool>()):v.get<float>();}catch(...){return 0;}}
const char* lab_login_text(const char* path){static std::string value;try{ensureLogin();value=loginDocument.at(json::json_pointer(path)).get<std::string>();}catch(...){value.clear();}return value.c_str();}
int lab_login_set_number(const char* path,float value){return guarded([&]{ensureLogin();auto next=loginDocument;auto pointer=json::json_pointer(path);auto replacement=next.at(pointer).is_boolean()?json(bool(value)):json(value);if(next.at(pointer)==replacement)return;next.at(pointer)=replacement;loginDocument=login::validate(next);loginChangedNow();});}
int lab_login_set_text(const char* path,const char* value){return guarded([&]{ensureLogin();auto next=loginDocument;auto pointer=json::json_pointer(path);if(!next.at(pointer).is_string())throw std::invalid_argument("Login field is not text");if(next.at(pointer)==value)return;next.at(pointer)=value;loginDocument=login::validate(next);loginChangedNow();});}
int lab_login_save(){if(!loginLoaded||!loginDirty)return 1;return guarded([&]{login::save(loginDocument);loginDirty=false;loginNotice="Bozza salvata. Applica per aggiornare il login all’avvio.";});}
float lab_login_image(const char* key){ensureLogin();return login::imageProfile(loginDocument).at(key);}
int lab_login_image_set(const char* key,float value){return guarded([&]{ensureLogin();auto next=loginDocument;next["profiles"][next.at("source").get<std::string>()][key]=value;loginDocument=login::validate(next);loginChangedNow();});}
const unsigned char* lab_login_pixels(int preset,int width,int height){
    static login::Pixels pixels;try{ensureLogin();auto d=preset<0?loginDocument:login::applyTemplate(login::defaults(),preset);login::CanvasState display;int item=lab_login_selected();if(item>=0)display.title=items[item].title;display.preview=false;display.username="User";pixels=login::draw(d,width,height,display,std::time(nullptr));return pixels.rgba.data();}catch(...){return nullptr;}
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
    auto config=loginDocument;auto item=items[selected];auto desktop=settings;auto outputs=monitors;
    loginNotice="Attendi la fine del caricamento";
    loginJob=std::async(std::launch::async,[config,item,desktop,outputs,windowed]{
        auto file=login::prepare(config,item,desktop,outputs),dir=file.parent_path();auto d=readJson(file);d["owner"]=getpid();writeJson(file,d);
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
    auto config=loginDocument;auto item=items[selected];auto desktop=settings;auto outputs=monitors;loginNotice="Attendi la fine del caricamento";
    loginJob=std::async(std::launch::async,[config,item,desktop,outputs]{auto file=login::prepare(config,item,desktop,outputs);auto helper=fs::read_symlink("/proc/self/exe").parent_path()/"login_deploy.py";
        auto invoke=[&](std::vector<std::string> args){auto response=capture(args);if(response.empty())throw std::runtime_error("Native login publication failed");auto result=json::parse(response.front());if(!result.value("ok",false))throw std::runtime_error(result.value("error",std::string("Native login publication failed")));return result;};
        auto prepared=invoke({"python3",helper.string(),"prepare",file.string()});writeJson(stateDir()/"login-screen/publication.json",prepared);
        auto result=invoke({"pkexec","/usr/bin/python3",helper.string(),"install",prepared.at("package")});result["draft"]=config;return result;});return 1;
}
void lab_login_poll(){
    if(loginDirty&&std::chrono::steady_clock::now()-loginChanged>std::chrono::milliseconds(350))lab_login_save();
    if(!loginPreviewProcess.empty()&&!owned(loginPreviewProcess))lab_login_stop_preview();
    if(loginJob.valid()&&loginJob.wait_for(std::chrono::seconds(0))==std::future_status::ready){
        try{auto result=loginJob.get();if(result.contains("preview")){lab_login_stop_preview();loginPreviewProcess=result["preview"];loginPreviewDirectory=result["directory"].get<std::string>();loginNotice="Anteprima";}
            else{if(result.value("disable",false)){loginDocument["enabled"]=false;loginChangedNow();lab_login_save();}loginNotice=result.value("next_boot",false)?(result.value("draft",json::object())==loginDocument?"Login salvato per il prossimo avvio.":"Bozza salvata. Applica per aggiornare il login all’avvio."):"Modifiche applicate e salvate.";writeJson(stateDir()/"login-screen/publication.json",result);}}
        catch(const std::exception& e){errorMessage=e.what();loginNotice="Operazione non riuscita. Dettagli nel registro.";std::cerr<<"Native login: "<<e.what()<<'\n';try{writeJson(stateDir()/"login-screen/error.json",{{"error",errorMessage}});}catch(...){};}
    }
}
