#include "popup.hpp"
#include <algorithm>
#include <fcntl.h>
#include <regex>
#include <sys/file.h>
#include <unistd.h>
namespace lab {
namespace {
fs::path directory(){auto p=stateDir()/"popup";fs::create_directories(p);return p;}
struct EventLock {
    int fd;
    EventLock(){fd=open((directory()/"events.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);if(fd<0||flock(fd,LOCK_EX)){if(fd>=0)close(fd);throw std::runtime_error("Impossibile bloccare il popup");}}
    ~EventLock(){close(fd);}
};
bool same(const json& a,const json& b){return a.value("epoch",std::string())==b.value("epoch",std::string())&&a.value("session",0LL)==b.value("session",0LL);}
int claim(){int fd=open((directory()/"instance.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);if(fd<0)throw std::runtime_error("Impossibile aprire il lock del popup");if(flock(fd,LOCK_EX|LOCK_NB)){close(fd);return -1;}return fd;}
}
json mergePopupEvent(json current,const std::string& epoch,long long session,int steps,bool commit){
    if(!std::regex_match(epoch,std::regex("[A-Za-z0-9_-]{1,80}"))||session<1||steps<1||steps>1000000)throw std::runtime_error("Evento carousel non valido");
    if(current.value("epoch",std::string())==epoch){
        auto previous=current.value("session",0LL);
        if(session<previous)return current;
        if(session==previous){
            if(current.value("done",false))return current;
            current["steps"]=std::max(steps,current.value("steps",0));
            current["commit"]=commit||current.value("commit",false);return current;
        }
    }
    return {{"epoch",epoch},{"session",session},{"steps",steps},{"commit",commit},{"done",false}};
}
int popupSelection(int count,int current,int steps){
    if(count<=0)return -1;
    return ((current>=0&&current<count?current:count-1)+(long long)std::max(steps,0))%count;
}
json popupState(){auto f=directory()/"event.json";return fs::exists(f)?readJson(f):json::object();}
PopupHost::~PopupHost(){if(fd>=0)close(fd);}
bool PopupHost::receive(const std::string& epoch,long long session,int steps,bool commit){
    EventLock lock;auto state=mergePopupEvent(popupState(),epoch,session,steps,commit);
    writeJson(directory()/"event.json",state);
    if(state.value("done",false))return false;
    fd=claim();return fd>=0;
}
bool PopupHost::resume(){EventLock lock;auto state=popupState();if(state.empty()||state.value("done",false))return false;fd=claim();return fd>=0;}
bool PopupHost::finish(const json& seen){
    EventLock lock;auto state=popupState();
    if(same(state,seen)){state["done"]=true;writeJson(directory()/"event.json",state);}
    if(fd>=0){close(fd);fd=-1;}
    return !state.empty()&&!state.value("done",false);
}
}
