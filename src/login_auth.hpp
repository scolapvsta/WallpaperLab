#pragma once
#include "catalog.hpp"
#include <future>
namespace lab::login {
struct Session {std::string id,name;std::vector<std::string> command,environment;};
std::vector<Session> sessions(const fs::path& directory="/usr/share/wayland-sessions");
class Auth {
    int fd=-1;
    std::future<json> job;
    std::string operation,retryUsername;
    json exchange(json request);
    void send(json request);
public:
    enum class Phase {Idle,Waiting,Prompt,Authenticated,Started,Error};
    Phase phase=Phase::Idle;
    std::string prompt,error;
    bool secret=true;
    explicit Auth(const std::string& socket);
    ~Auth();
    Auth(const Auth&)=delete;
    bool busy()const{return job.valid();}
    void create(const std::string& username);
    void respond(std::string response);
    void cancel();
    void start(const Session& session);
    void poll();
};
}
