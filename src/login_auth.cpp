#include "login_auth.hpp"
#include <glib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <algorithm>
namespace lab::login {
std::vector<Session> sessions(const fs::path& directory){
    std::vector<Session> result;std::error_code ec;
    for(auto& entry:fs::directory_iterator(directory,ec)){
        if(entry.path().extension()!=".desktop")continue;GKeyFile* key=g_key_file_new();GError* error=nullptr;
        if(!g_key_file_load_from_file(key,entry.path().c_str(),G_KEY_FILE_NONE,&error)){if(error)g_error_free(error);g_key_file_free(key);continue;}
        auto get=[&](const char* name){gchar* value=g_key_file_get_string(key,"Desktop Entry",name,nullptr);std::string text=value?value:"";g_free(value);return text;};
        auto command=get("Exec"),name=get("Name"),desktop=get("DesktopNames"),tryExec=get("TryExec");
        bool hidden=get("Hidden")=="true";g_key_file_free(key);if(hidden||command.empty()||command.find('%')!=std::string::npos)continue;
        if(!tryExec.empty()){gchar* found=g_find_program_in_path(tryExec.c_str());if(!found)continue;g_free(found);}
        int count=0;gchar** words=nullptr;if(!g_shell_parse_argv(command.c_str(),&count,&words,nullptr)||!count)continue;
        Session s{entry.path().filename().string(),name.empty()?entry.path().stem().string():name};for(int i=0;i<count;i++)s.command.emplace_back(words[i]);g_strfreev(words);
        std::replace(desktop.begin(),desktop.end(),';',':');if(!desktop.empty()&&desktop.back()==':')desktop.pop_back();
        s.environment={"XDG_SESSION_TYPE=wayland","XDG_SESSION_DESKTOP="+entry.path().stem().string(),"XDG_CURRENT_DESKTOP="+desktop};result.push_back(std::move(s));
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.name<b.name;});return result;
}
Auth::Auth(const std::string& path){
    if(path.empty()||path.size()>=sizeof(sockaddr_un::sun_path))throw std::invalid_argument("Invalid greetd socket");
    fd=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0);if(fd<0)throw std::runtime_error("Cannot open greetd socket");
    timeval timeout{20,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    sockaddr_un address{};address.sun_family=AF_UNIX;std::memcpy(address.sun_path,path.c_str(),path.size()+1);
    if(connect(fd,reinterpret_cast<sockaddr*>(&address),sizeof(address))){close(fd);fd=-1;throw std::runtime_error("Cannot connect to greetd");}
}
Auth::~Auth(){if(fd>=0)shutdown(fd,SHUT_RDWR);if(job.valid())try{job.get();}catch(...){}if(fd>=0)close(fd);}
json Auth::exchange(json request){
    std::string bytes=request.dump();request.clear();uint32_t length=bytes.size();
    auto transfer=[&](void* memory,size_t count,bool writing){auto p=static_cast<char*>(memory);while(count){ssize_t n=writing?::send(fd,p,count,MSG_NOSIGNAL):recv(fd,p,count,0);if(n<0&&errno==EINTR)continue;if(n<=0)throw std::runtime_error("greetd connection closed or timed out");count-=n;p+=n;}};
    try{transfer(&length,4,true);transfer(bytes.data(),bytes.size(),true);}catch(...){explicit_bzero(bytes.data(),bytes.size());throw;}
    explicit_bzero(bytes.data(),bytes.size());bytes.clear();transfer(&length,4,false);if(length==0||length>1024*1024)throw std::runtime_error("Invalid greetd response size");bytes.resize(length);transfer(bytes.data(),length,false);return json::parse(bytes);
}
void Auth::send(json request){if(busy())throw std::logic_error("Authentication request already pending");operation=request.at("type");error.clear();phase=Phase::Waiting;job=std::async(std::launch::async,[this,r=std::move(request)]()mutable{return exchange(std::move(r));});}
void Auth::create(const std::string& username){
    if(phase!=Phase::Idle&&phase!=Phase::Error)return;if(username.empty()||username.size()>128)return;
    // greetd retains failed authentication sessions until explicitly cancelled.
    if(phase==Phase::Error){retryUsername=username;send({{"type","cancel_session"}});return;}
    send({{"type","create_session"},{"username",username}});
}
void Auth::respond(std::string response){if(phase!=Phase::Prompt)return;send({{"type","post_auth_message_response"},{"response",response}});explicit_bzero(response.data(),response.size());}
void Auth::cancel(){if(!busy()){retryUsername.clear();send({{"type","cancel_session"}});}}
void Auth::start(const Session& session){
    if(phase!=Phase::Authenticated||session.command.empty())return;
    // greetd joins cmd with spaces and executes it through /bin/sh. Preserve
    // the trusted desktop entry's argument boundaries, including quoted paths.
    std::string command;for(auto& argument:session.command){auto quoted=g_shell_quote(argument.c_str());if(!command.empty())command+=' ';command+=quoted;g_free(quoted);}
    send({{"type","start_session"},{"cmd",json::array({command})},{"env",session.environment}});
}
void Auth::poll(){
    if(!job.valid()||job.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return;
    try{
        auto response=job.get();auto type=response.at("type").get<std::string>();
        if(type=="success"){phase=operation=="cancel_session"?Phase::Idle:operation=="start_session"?Phase::Started:Phase::Authenticated;prompt.clear();if(phase==Phase::Idle&&!retryUsername.empty()){auto username=std::move(retryUsername);retryUsername.clear();create(username);}return;}
        if(type=="error"){phase=Phase::Error;error=response.value("description",std::string("Authentication failed")).substr(0,1024);return;}
        if(type!="auth_message")throw std::runtime_error("Unknown greetd response");
        auto kind=response.at("auth_message_type").get<std::string>();prompt=response.at("auth_message").get<std::string>().substr(0,1024);
        if(kind=="info"||kind=="error"){send({{"type","post_auth_message_response"},{"response",nullptr}});error=prompt;return;}
        if(kind!="secret"&&kind!="visible")throw std::runtime_error("Unknown greetd authentication prompt");secret=kind=="secret";phase=Phase::Prompt;
    }catch(const std::exception& e){phase=Phase::Error;error=e.what();}
}
}
