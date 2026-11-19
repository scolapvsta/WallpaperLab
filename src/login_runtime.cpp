#include "login_runtime.hpp"
#include "login_canvas.hpp"
#include "login_astronaut.hpp"
#include "login_motion.hpp"
#include <sys/wait.h>
#include "login_auth.hpp"
#include "displays.hpp"
#include "theme_fonts.hpp"
#include "i18n.hpp"
#include <GL/glew.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <iostream>
#include <map>
#include <memory>
#include <clocale>
#include <glib.h>
#include <chrono>
int engineMain(int,char**,const std::string&);
namespace {
using namespace lab;using namespace lab::login;
json document;CanvasState state;bool active=false,quit=false;std::unique_ptr<Auth> auth;
pid_t powerChild=-1;std::string systemStatus;
std::string activeOutput;
std::map<std::string,json> outputProfiles;
std::map<std::string,std::array<float,13>> outputImages;
std::vector<Session> choices;int selectedSession=0;std::string primary,wallpaperOutput;double pointerX=0,pointerY=0;
Motion motion;double shownAt=-1;double seconds(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
std::chrono::steady_clock::time_point began;uint64_t revision=1;std::string diagnostic;
xkb_context* keyContext=nullptr;xkb_keymap* keymap=nullptr;xkb_state* keyState=nullptr;xkb_compose_state* compose=nullptr;
GLuint program=0,vao=0;struct Overlay{GLuint texture=0;int w=0,h=0;uint64_t revision=0;std::array<int,4> motionBounds={};std::time_t minute=0;int animation=-1;};std::map<std::string,Overlay> overlays;
bool showUI(const std::string& output){if(output=="default")return true;auto mode=document.value("ui_outputs",std::string("primary"));return mode=="all"||(mode=="primary"?output==primary:output!=primary);}
void touch(){motion.caretSince=seconds();revision++;}
void focus(int field){state.focus=field;state.cursor=(field==0?state.username:state.input).size();touch();}
void wipe(){explicit_bzero(state.input.data(),state.input.size());state.input.clear();motion.clear();state.cursor=0;state.reveal=false;}
void submit(){
    systemStatus.clear();
    if(state.preview){state.status=translate("Anteprima");if(state.focus==0)focus(1);revision++;return;}
    if(!auth||auth->busy())return;
    if(auth->phase==Auth::Phase::Idle||auth->phase==Auth::Phase::Error){auth->create(state.username);focus(1);wipe();}
    else if(auth->phase==Auth::Phase::Prompt){auth->respond(state.input);wipe();}
    state.busy=auth->busy();revision++;
}
void powerAction(int action){
    static const char* methods[]={"PowerOff","Reboot","Suspend","Hibernate"};
    if(action<0||action>3)return;
    if(state.preview){state.status=std::string("Preview · ")+methods[action];touch();return;}
    if(powerChild>0)return;if(auth)auth->cancel();wipe();state.reveal=false;
    const char* argv[]={"busctl","--system","--timeout=20","call","org.freedesktop.login1","/org/freedesktop/login1","org.freedesktop.login1.Manager",methods[action],"b","false",nullptr};
    GError* error=nullptr;GPid child=0;if(!g_spawn_async(nullptr,const_cast<char**>(argv),nullptr,GSpawnFlags(G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD|G_SPAWN_STDOUT_TO_DEV_NULL|G_SPAWN_STDERR_TO_DEV_NULL),nullptr,nullptr,&child,&error)){state.status="System action unavailable";if(error)g_error_free(error);}else{powerChild=child;state.status="Requesting system action…";}systemStatus=state.status;state.busy=powerChild>0;touch();
}
void poll(){
    if(powerChild>0){int status=0;if(waitpid(powerChild,&status,WNOHANG)==powerChild){g_spawn_close_pid(powerChild);powerChild=-1;systemStatus=WIFEXITED(status)&&WEXITSTATUS(status)==0?"":"System action not permitted by the session";state.status=systemStatus;revision++;}}

    if(state.preview&&std::chrono::steady_clock::now()-began>std::chrono::seconds(60))quit=true;
    if(!auth)return;auto old=auth->phase;auth->poll();
    auto status=auth->error.empty()?systemStatus:auth->error;bool busy=auth->busy()||powerChild>0;
    if(old!=auth->phase||state.status!=status||state.busy!=busy){revision++;state.prompt=auth->prompt;state.secret=auth->secret;state.status=status;state.busy=busy;}
    if(auth->phase==Auth::Phase::Authenticated){
        if(choices.empty()){state.status="No Wayland session installed";auth->cancel();return;}
        auth->start(choices.at(selectedSession));state.busy=true;revision++;
    }
    if(auth->phase==Auth::Phase::Started){
        // Remember identities only. Never persist PAM messages or responses.
        if(const char* storage=getenv("WALLPAPER_LAB_LOGIN_STATE"))try{writeJson(fs::path(storage)/"last-session.json",{{"username",state.username},{"session",choices.at(selectedSession).id}});}catch(...){}
        std::cout<<"WALLPAPER_LAB_LOGIN_STARTED\n"<<std::flush;quit=true;
    }
}
GLuint compile(GLenum kind,const char* text){GLuint id=glCreateShader(kind);glShaderSource(id,1,&text,nullptr);glCompileShader(id);GLint ok;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);if(!ok)throw std::runtime_error("Native login shader compilation failed");return id;}
void initializeGL(){
    const char* vs="#version 330 core\nout vec2 uv;void main(){vec2 p[6]=vec2[6](vec2(-1,-1),vec2(1,-1),vec2(1,1),vec2(-1,-1),vec2(1,1),vec2(-1,1));vec2 q=p[gl_VertexID];gl_Position=vec4(q,0,1);uv=vec2(q.x*.5+.5,.5-q.y*.5);}";
    const char* fs="#version 330 core\nin vec2 uv;out vec4 pixel;uniform sampler2D overlay;uniform float opacity;void main(){pixel=texture(overlay,uv)*opacity;}";
    auto vertex=compile(GL_VERTEX_SHADER,vs),fragment=compile(GL_FRAGMENT_SHADER,fs);program=glCreateProgram();glAttachShader(program,vertex);glAttachShader(program,fragment);glLinkProgram(program);glDeleteShader(vertex);glDeleteShader(fragment);GLint ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("Native login shader linking failed");glGenVertexArrays(1,&vao);
}
#include "login_blur.inc"
}
extern "C" void wallpaper_lab_login_output(const char* output){activeOutput=output;}
extern "C" int wallpaper_lab_login_image(float* values){
    if(!active)return 0;auto image=outputImages.find(activeOutput);if(image==outputImages.end())image=outputImages.find("default");if(image==outputImages.end())return 0;std::copy(image->second.begin(),image->second.end(),values);return 1;
}
extern "C" const char* wallpaper_lab_login_property(const char* key){
    static std::string value;auto it=outputProfiles.find(activeOutput);if(it==outputProfiles.end())it=outputProfiles.find("default");if(it==outputProfiles.end()||!it->second.contains(key))return nullptr;auto& property=it->second.at(key);value=property.is_string()?property.get<std::string>():property.dump();return value.c_str();
}
extern "C" int wallpaper_lab_login_active(){return active;}
extern "C" int wallpaper_lab_login_quit(){return active&&quit;}
extern "C" int wallpaper_lab_login_paused(){return active&&state.paused;}
extern "C" float wallpaper_lab_login_time(float now){static float previous=0,elapsed=0;if(!previous)previous=now;if(!state.paused)elapsed+=now-previous;previous=now;return elapsed;}
extern "C" void wallpaper_lab_login_draw(const char* name,int x,int y,int w,int h){
    if(!active)return;
    try{
        poll();
        if(!showUI(name))return;
        GLint oldProgram,oldVao,oldActive,oldTexture,oldViewport[4],srcRGB,dstRGB,srcA,dstA,eqRGB,eqA;GLboolean blend=glIsEnabled(GL_BLEND),depth=glIsEnabled(GL_DEPTH_TEST),scissor=glIsEnabled(GL_SCISSOR_TEST),cull=glIsEnabled(GL_CULL_FACE);glGetIntegerv(GL_CURRENT_PROGRAM,&oldProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);glGetIntegerv(GL_ACTIVE_TEXTURE,&oldActive);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&oldTexture);glGetIntegerv(GL_VIEWPORT,oldViewport);glGetIntegerv(GL_BLEND_SRC_RGB,&srcRGB);glGetIntegerv(GL_BLEND_DST_RGB,&dstRGB);glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcA);glGetIntegerv(GL_BLEND_DST_ALPHA,&dstA);glGetIntegerv(GL_BLEND_EQUATION_RGB,&eqRGB);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&eqA);
        if(!program)initializeGL();auto& image=overlays[name];auto minute=std::time(nullptr)/(document["widgets"]["clock"].value("seconds",false)?1:60);double now=seconds();if(shownAt<0)shownAt=now;bool moving=motion.sample(state,now,document["widgets"]["login"].value("animation",.28));
        int animation=moving?int(std::fmod(now,10000.)*60):state.caretAlpha>.5?600001:600002;
        bool full=image.w!=w||image.h!=h||image.revision!=revision||image.minute!=minute;
        if(full||image.animation!=animation){
            auto bounds=motionRect(document,w,h,state),region=full?std::array<int,4>{0,0,w,h}:bounds;
            if(!full&&image.motionBounds[2]>0){auto previous=image.motionBounds;int left=std::min(region[0],previous[0]),top=std::min(region[1],previous[1]);region={left,top,std::max(region[0]+region[2],previous[0]+previous[2])-left,std::max(region[1]+region[3],previous[1]+previous[3])-top};}image.motionBounds=bounds;
            auto pixels=draw(document,w,h,state,std::time(nullptr),region);if(!image.texture)glGenTextures(1,&image.texture);glBindTexture(GL_TEXTURE_2D,image.texture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            if(full)glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.rgba.data());else glTexSubImage2D(GL_TEXTURE_2D,0,region[0],region[1],region[2],region[3],GL_RGBA,GL_UNSIGNED_BYTE,pixels.rgba.data());
            image.w=w;image.h=h;image.revision=revision;image.minute=minute;image.animation=animation;
        }
        glViewport(x,y,w,h);glDisable(GL_DEPTH_TEST);glDisable(GL_SCISSOR_TEST);glDisable(GL_CULL_FACE);blurBackdrop(name,x,y,w,h);glEnable(GL_BLEND);glBlendEquation(GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glUseProgram(program);glBindVertexArray(vao);glBindTexture(GL_TEXTURE_2D,image.texture);glUniform1i(glGetUniformLocation(program,"overlay"),0);glUniform1f(glGetUniformLocation(program,"opacity"),ease((now-shownAt)/.28));glDrawArrays(GL_TRIANGLES,0,6);
        glBindTexture(GL_TEXTURE_2D,oldTexture);glActiveTexture(oldActive);glBindVertexArray(oldVao);glUseProgram(oldProgram);glViewport(oldViewport[0],oldViewport[1],oldViewport[2],oldViewport[3]);glBlendFuncSeparate(srcRGB,dstRGB,srcA,dstA);glBlendEquationSeparate(eqRGB,eqA);if(!blend)glDisable(GL_BLEND);if(depth)glEnable(GL_DEPTH_TEST);if(scissor)glEnable(GL_SCISSOR_TEST);if(cull)glEnable(GL_CULL_FACE);
        static bool ready=false;if(!ready){std::cout<<"WALLPAPER_LAB_LOGIN_READY\n"<<std::flush;ready=true;}
    }catch(const std::exception& e){if(diagnostic!=e.what()){diagnostic=e.what();std::cerr<<"Login drawing failed: "<<diagnostic<<'\n';}quit=true;}
}
extern "C" void wallpaper_lab_login_symbol(unsigned symbol){
    if(!active)return;
    if(state.powerConfirm>=0){if(symbol==XKB_KEY_Escape){state.powerConfirm=-1;touch();}else if(symbol==XKB_KEY_Return){int action=state.powerConfirm;state.powerConfirm=-1;powerAction(action);}return;}
    if(state.keyboardOpen&&symbol==XKB_KEY_Escape){state.keyboardOpen=false;touch();return;}
    if(state.menuOpen){
        if(symbol==XKB_KEY_Escape||symbol==XKB_KEY_Tab){state.menuOpen=false;touch();return;}
        if(symbol==XKB_KEY_Up||symbol==XKB_KEY_Down||symbol==XKB_KEY_Home||symbol==XKB_KEY_End){state.menuIndex=std::clamp(symbol==XKB_KEY_Home?0:symbol==XKB_KEY_End?int(choices.size())-1:state.menuIndex+(symbol==XKB_KEY_Up?-1:1),0,std::max(0,int(choices.size())-1));state.menuScroll=std::clamp(state.menuIndex-2,0,std::max(0,int(choices.size())-5));touch();return;}
        if(symbol==XKB_KEY_Return||symbol==XKB_KEY_KP_Enter){selectedSession=state.menuIndex;state.session=choices.at(selectedSession).name;state.menuOpen=false;touch();}return;
    }
    if(state.focus==2&&(symbol==XKB_KEY_Return||symbol==XKB_KEY_space||symbol==XKB_KEY_Down)&&!choices.empty()){state.keyboardOpen=false;state.menuOpen=true;state.menuIndex=selectedSession;state.menuScroll=std::clamp(selectedSession-2,0,std::max(0,int(choices.size())-5));touch();return;}
    if(symbol==XKB_KEY_Escape){if(state.preview)quit=true;else if(auth&&!auth->busy()){auth->cancel();wipe();focus(0);state.status.clear();}revision++;return;}
    if(state.busy)return;
    if(symbol==XKB_KEY_Return||symbol==XKB_KEY_KP_Enter){submit();return;}
    if(symbol==XKB_KEY_Tab){if(state.focus==0&&auth&&auth->phase==Auth::Phase::Idle)submit();else focus((state.focus+1)%3);revision++;return;}
    if(state.focus<2){
        if(state.focus==0&&auth&&auth->phase!=Auth::Phase::Idle&&auth->phase!=Auth::Phase::Error)return;
        auto& value=state.focus==0?state.username:state.input;state.cursor=std::clamp(state.cursor,0,int(value.size()));int before=state.cursor,after=state.cursor;
        if(before>0){before--;while(before>0&&(static_cast<unsigned char>(value[before])&0xc0)==0x80)before--;}
        if(after<int(value.size())){after++;while(after<int(value.size())&&(static_cast<unsigned char>(value[after])&0xc0)==0x80)after++;}
        if(symbol==XKB_KEY_Left)state.cursor=before;else if(symbol==XKB_KEY_Right)state.cursor=after;else if(symbol==XKB_KEY_Home)state.cursor=0;else if(symbol==XKB_KEY_End)state.cursor=value.size();
        else if(symbol==XKB_KEY_BackSpace&&state.cursor>0){if(state.focus==1)motion.erased(g_utf8_strlen(value.c_str(),before));value.erase(before,state.cursor-before);state.cursor=before;}
        else if(symbol==XKB_KEY_Delete){if(state.focus==1)motion.erased(g_utf8_strlen(value.c_str(),state.cursor));value.erase(state.cursor,after-state.cursor);}else return;touch();
    }
}
extern "C" void wallpaper_lab_login_text(unsigned code){
    if(!active||state.busy||state.menuOpen||state.powerConfirm>=0||code<32||code==127||code>0x10ffff||state.focus==2)return;
    auto& value=state.focus==0?state.username:state.input;if(value.size()>512)return;
    if(state.focus==0&&auth&&auth->phase!=Auth::Phase::Idle&&auth->phase!=Auth::Phase::Error)return;
    char bytes[8]{};int n=g_unichar_to_utf8(code,bytes);state.cursor=std::clamp(state.cursor,0,int(value.size()));if(state.focus==1)motion.inserted(g_utf8_strlen(value.c_str(),state.cursor),seconds());value.insert(state.cursor,bytes,n);state.cursor+=n;explicit_bzero(bytes,sizeof(bytes));touch();
}
extern "C" void wallpaper_lab_login_pointer(const char* output,double x,double y,int pressed){
    if(!active||!showUI(output))return;if(x>=0&&y>=0){pointerX=x;pointerY=y;}if(!pressed||state.busy)return;
    int action=hit(document,pointerX,pointerY,&state);
    if(action>=10&&action<=13){state.powerConfirm=action-10;state.keyboardOpen=false;state.menuOpen=false;touch();return;}
    if(action==300){int chosen=state.powerConfirm;state.powerConfirm=-1;powerAction(chosen);return;}
    if(action==301){state.powerConfirm=-1;touch();return;}
    if(action==14){state.keyboardOpen=!state.keyboardOpen;if(state.focus==2)focus(1);touch();return;}
    if(action==15){state.reveal=!state.reveal;touch();return;}
    if(action>=200&&action<250){std::string letters;for(auto& row:keyRows)letters+=row;int index=action-200;if(index<int(letters.size())){char code=letters[index];if(state.keyboardShift){std::string from="1234567890-=[];',./",to="!@#$%^&*()_+{}\":<>?";auto at=from.find(code);code=at==std::string::npos?std::toupper(code):to[at];}wallpaper_lab_login_text(code);}return;}
    if(action==250){state.keyboardShift=!state.keyboardShift;touch();return;}if(action==251){wallpaper_lab_login_text(' ');return;}if(action==252){wallpaper_lab_login_symbol(XKB_KEY_BackSpace);return;}if(action==253){wallpaper_lab_login_symbol(XKB_KEY_Return);return;}if(action==254){state.keyboardOpen=false;touch();return;}
    if(action==0||action==1){focus(action);if(action==1&&auth&&auth->phase==Auth::Phase::Idle)submit();}
    else if(action==2&&!choices.empty()){focus(2);state.keyboardOpen=false;state.menuOpen=true;state.menuIndex=selectedSession;state.menuScroll=std::clamp(selectedSession-2,0,std::max(0,int(choices.size())-5));}
    else if(action>=100&&action-100<int(choices.size())){selectedSession=action-100;state.session=choices[selectedSession].name;state.menuOpen=false;}else if(action==6)state.menuOpen=false;
    else if(action==3)submit();else if(action==4)wallpaper_lab_login_symbol(XKB_KEY_Escape);revision++;
}
extern "C" void wallpaper_lab_login_keymap(int fd,unsigned size){
    if(!active||size<1||size>1024*1024){close(fd);return;}auto memory=mmap(nullptr,size,PROT_READ,MAP_PRIVATE,fd,0);close(fd);if(memory==MAP_FAILED)return;
    auto next=xkb_keymap_new_from_buffer(keyContext,static_cast<const char*>(memory),size,XKB_KEYMAP_FORMAT_TEXT_V1,XKB_KEYMAP_COMPILE_NO_FLAGS);munmap(memory,size);if(!next)return;
    if(keyState)xkb_state_unref(keyState);if(keymap)xkb_keymap_unref(keymap);keymap=next;keyState=xkb_state_new(keymap);
}
extern "C" void wallpaper_lab_login_modifiers(unsigned depressed,unsigned latched,unsigned locked,unsigned group){if(keyState)xkb_state_update_mask(keyState,depressed,latched,locked,0,0,group);}
extern "C" void wallpaper_lab_login_key(unsigned key,unsigned pressed){
    if(!keyState||!pressed)return;auto symbol=xkb_state_key_get_one_sym(keyState,key+8);wallpaper_lab_login_symbol(symbol);
    if(symbol==XKB_KEY_Return||symbol==XKB_KEY_Tab||symbol==XKB_KEY_Escape||symbol==XKB_KEY_BackSpace)return;
    if(compose){xkb_compose_state_feed(compose,symbol);auto status=xkb_compose_state_get_status(compose);if(status==XKB_COMPOSE_COMPOSING)return;if(status==XKB_COMPOSE_COMPOSED){char result[64]{};xkb_compose_state_get_utf8(compose,result,sizeof(result));for(auto p=result;*p;p=g_utf8_next_char(p))wallpaper_lab_login_text(g_utf8_get_char(p));xkb_compose_state_reset(compose);return;}if(status==XKB_COMPOSE_CANCELLED){xkb_compose_state_reset(compose);return;}}
    wallpaper_lab_login_text(xkb_state_key_get_utf32(keyState,key+8));
}
int loginRun(const std::string& file,bool preview,bool windowed,const std::string& ready){
    document=validate(readJson(file));setLanguage(languageIndex(document.value("language",std::string("en"))));state=CanvasState{};motion=Motion{};shownAt=-1;state.preview=preview;state.title=document.value("title",std::string());began=std::chrono::steady_clock::now();quit=false;registerThemeFonts();
    for(auto& file:document.value("font_files",json::array()))FcConfigAppFontAddFile(FcConfigGetCurrent(),reinterpret_cast<const FcChar8*>(file.get_ref<const std::string&>().c_str()));
    if(document.contains("font_file"))FcConfigAppFontAddFile(FcConfigGetCurrent(),reinterpret_cast<const FcChar8*>(document["font_file"].get_ref<const std::string&>().c_str()));
    choices=sessions();for(size_t n=0;n<choices.size();n++)if(choices[n].id==document.value("session",std::string()))selectedSession=n;if(!choices.empty())state.session=choices[selectedSession].name;
    if(preview){if(auto user=getpwuid(getuid()))state.username=user->pw_name;}
    else{
        const char* socket=getenv("GREETD_SOCK");if(!socket)throw std::runtime_error("Native greeter must be launched by greetd");auth=std::make_unique<Auth>(socket);state.username=document.value("username",std::string());
        if(const char* storage=getenv("WALLPAPER_LAB_LOGIN_STATE"))try{auto last=readJson(fs::path(storage)/"last-session.json");state.username=last.value("username",state.username);for(size_t n=0;n<choices.size();n++)if(choices[n].id==last.value("session",std::string()))selectedSession=n;}catch(...){}
        if(!choices.empty())state.session=choices[selectedSession].name;
    }
    for(auto& choice:choices)state.sessions.push_back(choice.name);focus(preview?1:0);
    setlocale(LC_TIME,"");keyContext=xkb_context_new(XKB_CONTEXT_NO_FLAGS);auto locale=setlocale(LC_CTYPE,"");auto table=xkb_compose_table_new_from_locale(keyContext,locale?locale:"C",XKB_COMPOSE_COMPILE_NO_FLAGS);if(table){compose=xkb_compose_state_new(table,XKB_COMPOSE_STATE_NO_FLAGS);xkb_compose_table_unref(table);}
    try{
        auto rows=capture({"hyprctl","monitors","-j"});if(!windowed&&!rows.empty()){auto displays=parseDisplays(json::parse(rows.front()));std::vector<std::string> outputs;for(auto& d:displays)outputs.push_back(d.name);document["outputs"]=outputs;if(document.value("mode",std::string())=="span"&&document.contains("display_layout"))document["render_layout"]=renderLayout(displayCanvas(displays,document),displays);}
        auto outputs=document.at("outputs").get<std::vector<std::string>>();auto select=[&](const char* key){auto name=document.value(key,std::string());return std::find(outputs.begin(),outputs.end(),name)!=outputs.end()?name:outputs.empty()?std::string("default"):outputs.front();};primary=select("ui_primary");wallpaperOutput=select("output");
        // A secondary-only layout must remain reachable when only one screen is connected.
        if(outputs.size()<2&&document.value("ui_outputs",std::string())=="secondary")document["ui_outputs"]="primary";
        auto profiles=[&](const json& record){return readJson(record.at("profile_file").get<std::string>()).value(record.at("source").get<std::string>(),json::object()).value("properties",json::object());};outputProfiles["default"]=profiles(document);if(document.contains("prepared_wallpapers"))for(auto& [output,record]:document["prepared_wallpapers"].items())outputProfiles[output]=profiles(record);
        auto tuning=[&](const json& record){auto image=record.value("image_profile",imageProfile(document));std::array<float,13> values;const char* keys[]={"x","y","zoom","brightness","contrast","saturation","hue","temperature","gamma","vignette","rotation","flip_x","flip_y"};for(int n=0;n<13;n++)values[n]=image.at(keys[n]);return values;};outputImages["default"]=tuning(document);if(document.contains("prepared_wallpapers"))for(auto& [output,record]:document["prepared_wallpapers"].items())outputImages[output]=tuning(record);
        auto args=renderArguments(document,windowed);if(!windowed&&document.contains("render_layout"))args.insert(args.end(),{"--lab-layout",document["render_layout"].dump()});
        std::vector<char*> av;for(auto& arg:args)av.push_back(arg.data());av.push_back(nullptr);active=true;int code=engineMain(args.size(),av.data(),ready);active=false;auth.reset();wipe();return code;
    }catch(...){active=false;auth.reset();wipe();throw;}
}
