// The backdrop is blurred on the GPU; animated wallpapers never read back to CPU.
struct Backdrop {GLuint source=0,pass=0,fbo=0;int w=0,h=0;};
std::map<std::string,Backdrop> backdrops;GLuint blurProgram=0;
void blurBackdrop(const std::string& name,int x,int y,int w,int h){
    if(!astronaut(document))return;double radius=document["widgets"]["login"]["astronaut"]["blur"];if(radius<=0)return;
    GLint drawFbo,readFbo;glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&drawFbo);glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&readFbo);
    if(!blurProgram){
        const char* vs="#version 330 core\nout vec2 uv;void main(){vec2 p[6]=vec2[6](vec2(-1,-1),vec2(1,-1),vec2(1,1),vec2(-1,-1),vec2(1,1),vec2(-1,1));vec2 q=p[gl_VertexID];gl_Position=vec4(q,0,1);uv=vec2(q.x*.5+.5,.5-q.y*.5);}";
        const char* fs="#version 330 core\nin vec2 uv;out vec4 pixel;uniform sampler2D image;uniform vec2 delta;void main(){vec2 p=vec2(uv.x,1-uv.y);vec4 sum=vec4(0);float weight=0;for(int n=-4;n<=4;n++){float t=float(n);float a=exp(-t*t/8.);sum+=texture(image,p+delta*t)*a;weight+=a;}pixel=sum/weight;}";
        auto vertex=compile(GL_VERTEX_SHADER,vs),fragment=compile(GL_FRAGMENT_SHADER,fs);blurProgram=glCreateProgram();glAttachShader(blurProgram,vertex);glAttachShader(blurProgram,fragment);glLinkProgram(blurProgram);glDeleteShader(vertex);glDeleteShader(fragment);GLint ok;glGetProgramiv(blurProgram,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("Login blur shader linking failed");
    }
    auto& b=backdrops[name];int bw=std::max(1,w/4),bh=std::max(1,h/4);
    if(!b.source){glGenTextures(1,&b.source);glGenTextures(1,&b.pass);glGenFramebuffers(1,&b.fbo);}
    if(b.w!=w||b.h!=h){for(int n=0;n<2;n++){glBindTexture(GL_TEXTURE_2D,n?b.pass:b.source);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,n?bw:w,n?bh:h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}b.w=w;b.h=h;}
    glBindFramebuffer(GL_READ_FRAMEBUFFER,drawFbo);glBindTexture(GL_TEXTURE_2D,b.source);glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,x,y,w,h);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,b.fbo);glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,b.pass,0);
    if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){glBindFramebuffer(GL_DRAW_FRAMEBUFFER,drawFbo);glBindFramebuffer(GL_READ_FRAMEBUFFER,readFbo);throw std::runtime_error("Login blur framebuffer unavailable");}
    glViewport(0,0,bw,bh);glDisable(GL_BLEND);glUseProgram(blurProgram);glBindVertexArray(vao);glUniform1i(glGetUniformLocation(blurProgram,"image"),0);
    double pixels=radius*std::min(w/1920.,h/1080.);glUniform2f(glGetUniformLocation(blurProgram,"delta"),pixels/(4*w),0);glDrawArrays(GL_TRIANGLES,0,6);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,drawFbo);glBindFramebuffer(GL_READ_FRAMEBUFFER,readFbo);glViewport(x,y,w,h);glBindTexture(GL_TEXTURE_2D,b.pass);glUniform2f(glGetUniformLocation(blurProgram,"delta"),0,pixels/(4*h));
    auto r=widgetRect(document,2);GLint oldScissor[4];glGetIntegerv(GL_SCISSOR_BOX,oldScissor);glEnable(GL_SCISSOR_TEST);glScissor(x+int(r[0]*w),y+int((1-r[1]-r[3])*h),int(std::ceil(r[2]*w)),int(std::ceil(r[3]*h)));glDrawArrays(GL_TRIANGLES,0,6);glDisable(GL_SCISSOR_TEST);glScissor(oldScissor[0],oldScissor[1],oldScissor[2],oldScissor[3]);
}
