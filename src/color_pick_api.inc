// A nonblocking desktop picker. Never copy sampled colors or images to the clipboard.
static pid_t colorPickPid=-1;static int colorPickFd=-1;
static std::string colorPickStart,colorPickExe,colorPickOutput;
static std::chrono::steady_clock::time_point colorPickDeadline;
int lab_color_pick_available(){return executableAvailable("hyprpicker")&&getenv("WAYLAND_DISPLAY");}
void lab_color_pick_cancel(){
    if(colorPickPid>1)try{
        if(startTime(colorPickPid)==colorPickStart&&fs::read_symlink("/proc/"+std::to_string(colorPickPid)+"/exe")==colorPickExe)kill(colorPickPid,SIGTERM);
        int status;waitpid(colorPickPid,&status,0);
    }catch(...){}
    colorPickPid=-1;if(colorPickFd>=0)close(colorPickFd);colorPickFd=-1;colorPickOutput.clear();
}
int lab_color_pick_start(){
    if(colorPickPid>1||!lab_color_pick_available())return 0;
    return guarded([&]{
        std::istringstream paths(getenv("PATH"));std::string path;fs::path exe;
        while(std::getline(paths,path,':'))if(!path.empty()&&access((fs::path(path)/"hyprpicker").c_str(),X_OK)==0){exe=fs::canonical(fs::path(path)/"hyprpicker");break;}
        int pipes[2];if(exe.empty()||pipe2(pipes,O_CLOEXEC|O_NONBLOCK))throw std::runtime_error("Cannot start screen color picker");
        posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);posix_spawn_file_actions_adddup2(&actions,pipes[1],STDOUT_FILENO);posix_spawn_file_actions_addclose(&actions,pipes[0]);posix_spawn_file_actions_addclose(&actions,pipes[1]);
        std::vector<std::string> args={exe.string(),"--format","hex","--no-fancy"};std::vector<char*> av;for(auto& a:args)av.push_back(a.data());av.push_back(nullptr);
        pid_t pid;int error=posix_spawn(&pid,av[0],&actions,nullptr,av.data(),environ);posix_spawn_file_actions_destroy(&actions);close(pipes[1]);
        if(error){close(pipes[0]);throw std::runtime_error("Cannot start screen color picker");}
        colorPickPid=pid;colorPickFd=pipes[0];colorPickStart=startTime(pid);colorPickExe=exe.string();colorPickOutput.clear();colorPickDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(90);
    });
}
int lab_color_pick_poll(unsigned* color){
    if(colorPickPid<1)return -1;
    char buffer[64];ssize_t n;while((n=read(colorPickFd,buffer,sizeof(buffer)))>0)if(colorPickOutput.size()<256)colorPickOutput.append(buffer,n);
    int status;auto done=waitpid(colorPickPid,&status,WNOHANG);
    if(done==0){if(std::chrono::steady_clock::now()>colorPickDeadline){lab_color_pick_cancel();return -1;}return 0;}
    while((n=read(colorPickFd,buffer,sizeof(buffer)))>0)if(colorPickOutput.size()<256)colorPickOutput.append(buffer,n);
    colorPickPid=-1;close(colorPickFd);colorPickFd=-1;
    if(done<0||!WIFEXITED(status)||WEXITSTATUS(status))return -1;
    auto at=colorPickOutput.find('#');if(at==std::string::npos||colorPickOutput.size()<at+7)return -1;
    auto hex=colorPickOutput.substr(at+1,6);if(hex.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)return -1;
    *color=std::stoul(hex,nullptr,16);return 1;
}
