#include "login_auth.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <fstream>
#include <cstring>
#include <iostream>
using namespace lab;using namespace lab::login;
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void await(Auth& a,Auth::Phase phase){auto end=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(a.busy()&&std::chrono::steady_clock::now()<end){a.poll();std::this_thread::sleep_for(std::chrono::milliseconds(1));}check(a.phase==phase,"Unexpected authentication phase");}
int main(){
 auto path=fs::temp_directory_path()/("lab-auth-"+std::to_string(getpid()));int listener=socket(AF_UNIX,SOCK_STREAM,0);sockaddr_un address{};address.sun_family=AF_UNIX;strcpy(address.sun_path,path.c_str());check(bind(listener,(sockaddr*)&address,sizeof(address))==0,"bind");listen(listener,1);
 std::exception_ptr failure;std::thread server([&]{try{
  int client=accept(listener,nullptr,nullptr);
  auto exchange=[&](const json& expected,const json& answer){uint32_t length;check(recv(client,&length,4,MSG_WAITALL)==4,"request size");std::string data(length,'\0');check(recv(client,data.data(),length,MSG_WAITALL)==length,"request body");check(json::parse(data)==expected,"Unexpected greetd request");data=answer.dump();length=data.size();auto bytes=reinterpret_cast<const char*>(&length);for(int i=0;i<4;i++)check(send(client,bytes+i,1,MSG_NOSIGNAL)==1,"partial header");for(char c:data)check(send(client,&c,1,MSG_NOSIGNAL)==1,"partial body");};
  exchange({{"type","create_session"},{"username","demo"}},{{"type","auth_message"},{"auth_message_type","info"},{"auth_message","Two factors required"}});
  exchange({{"type","post_auth_message_response"},{"response",nullptr}},{{"type","auth_message"},{"auth_message_type","secret"},{"auth_message","Password:"}});
  exchange({{"type","post_auth_message_response"},{"response","test-password"}},{{"type","auth_message"},{"auth_message_type","visible"},{"auth_message","Token:"}});
  exchange({{"type","post_auth_message_response"},{"response","123456"}},{{"type","success"}});
  exchange({{"type","start_session"},{"cmd",json::array({"'/usr/bin/false'"})},{"env",json::array()}},{{"type","error"},{"error_type","error"},{"description","Session failed"}});
  exchange({{"type","cancel_session"}},{{"type","success"}});
  exchange({{"type","create_session"},{"username","demo"}},{{"type","error"},{"error_type","auth_error"},{"description","Wrong credentials"}});
  exchange({{"type","cancel_session"}},{{"type","success"}});
  exchange({{"type","create_session"},{"username","demo"}},{{"type","success"}});
  exchange({{"type","start_session"},{"cmd",json::array({"'/usr/bin/true' 'quoted argument'"})},{"env",json::array({"XDG_SESSION_TYPE=wayland"})}},{{"type","success"}});
  close(client);
 }catch(...){failure=std::current_exception();}});
 try{
  Auth a(path);a.start(Session{"","",{"/usr/bin/false"},{}});check(!a.busy(),"Unauthenticated session start");a.create("");check(!a.busy(),"Empty username accepted");a.create("demo");await(a,Auth::Phase::Prompt);check(a.secret&&a.prompt=="Password:","Secret prompt missing");
  a.respond("test-password");await(a,Auth::Phase::Prompt);check(!a.secret&&a.prompt=="Token:","Visible second factor missing");a.respond("123456");await(a,Auth::Phase::Authenticated);
  a.start(Session{"","",{"/usr/bin/false"},{}});await(a,Auth::Phase::Error);check(a.error=="Session failed","Session failure lost");a.cancel();await(a,Auth::Phase::Idle);
  a.create("demo");await(a,Auth::Phase::Error);check(a.error=="Wrong credentials","Wrong credentials lost");a.create("demo");await(a,Auth::Phase::Authenticated);a.start(Session{"","",{"/usr/bin/true","quoted argument"},{"XDG_SESSION_TYPE=wayland"}});await(a,Auth::Phase::Started);
 }catch(...){shutdown(listener,SHUT_RDWR);server.detach();fs::remove(path);throw;}
 server.join();close(listener);fs::remove(path);if(failure)std::rethrow_exception(failure);
 auto root=fs::temp_directory_path()/("lab-sessions-"+std::to_string(getpid()));fs::create_directory(root);
 writeJson(root/"ignored.json",json::object());
 {std::ofstream f(root/"valid.desktop");f<<"[Desktop Entry]\nName=Example\nExec=/usr/bin/true 'argument with spaces'\nDesktopNames=Example;Test;\nTryExec=/usr/bin/true\n";}
 {std::ofstream f(root/"hidden.desktop");f<<"[Desktop Entry]\nName=Hidden\nExec=/usr/bin/true\nHidden=true\n";}
 {std::ofstream f(root/"missing.desktop");f<<"[Desktop Entry]\nExec=does-not-exist\nTryExec=wallpaper-lab-test-missing\n";}
 auto choices=sessions(root);check(choices.size()==1&&choices[0].command[1]=="argument with spaces","Trusted session parsing");check(choices[0].environment.back()=="XDG_CURRENT_DESKTOP=Example:Test","Desktop environment parsing");fs::remove_all(root);
 std::cout<<"Partial IPC reads, multi-factor prompts, errors, retry, cancellation and trusted sessions passed\n";
}
