// Explicit desktop regression: the final compositor must ignore scene GL masks.
#include <GL/glew.h>
#include "raylib.h"
#include "rlgl.h"
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "WallpaperEngine/Render/Helpers/Transition.h"
int main(int argc,char** argv){
 namespace fs=std::filesystem;fs::path root=argc>1?argv[1]:"local/transition-state";fs::create_directories(root);
 const int w=128,h=96;auto snapshot=root/"old.bin";std::ofstream stream(snapshot,std::ios::binary);uint32_t header[]={0x574c4631,0,w,h,0};stream.write((char*)header,20);for(int n=0;n<w*h;n++){unsigned char rgb[]={180,30,60};stream.write((char*)rgb,3);}stream.close();
 auto config=nlohmann::json{{"fixture",{{"file",fs::absolute(snapshot).string()},{"effect",1},{"start_file",fs::absolute(root/"never-start").string()}}}}.dump();setenv("WALLPAPER_LAB_TRANSITION",config.c_str(),1);
 SetTraceLogLevel(LOG_ERROR);InitWindow(320,240,"Wallpaper Lab GL state regression");glewExperimental=GL_TRUE;glewInit();while(glGetError()!=GL_NO_ERROR){};auto target=LoadRenderTexture(w,h);BeginTextureMode(target);ClearBackground((Color){0,0,0,0});rlDrawRenderBatchActive();
 glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
 WallpaperEngine::Render::Helpers::transitionFrame("fixture",{0,0,w,h},true);
 unsigned char pixel[4]={};glReadPixels(w/2,h/2,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);GLboolean mask[4];glGetBooleanv(GL_COLOR_WRITEMASK,mask);bool preserved=mask[0]&&mask[1]&&mask[2]&&!mask[3];glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);EndTextureMode();Image shot=LoadImageFromTexture(target.texture);ExportImage(shot,(root/"mask.png").c_str());UnloadImage(shot);
 // Deterministic GPU matrix: all effects, curves, directions, origins and zoom amounts.
 auto compile=[](GLenum kind,const char* code){GLuint id=glCreateShader(kind);glShaderSource(id,1,&code,nullptr);glCompileShader(id);return id;};
 const char* vs="#version 330 core\nout vec2 uv;void main(){uv=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(uv*2.-1.,0,1);}";
 GLuint vertex=compile(GL_VERTEX_SHADER,vs),fragment=compile(GL_FRAGMENT_SHADER,LAB_TRANSITION_FRAGMENT),program=glCreateProgram();glAttachShader(program,vertex);glAttachShader(program,fragment);glLinkProgram(program);glDeleteShader(vertex);glDeleteShader(fragment);GLuint vao,tex[2];glGenVertexArrays(1,&vao);glGenTextures(2,tex);
 const unsigned char colors[2][4]={{180,30,60,255},{20,60,180,255}};
 for(int n=0;n<2;n++){glActiveTexture(GL_TEXTURE0+n);glBindTexture(GL_TEXTURE_2D,tex[n]);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,colors[n]);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);}
 BeginTextureMode(target);rlDrawRenderBatchActive();glUseProgram(program);glBindVertexArray(vao);glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);
 auto loc=[&](const char* name){return glGetUniformLocation(program,name);};glUniform1i(loc("previousImage"),0);glUniform1i(loc("currentImage"),1);glUniform3f(loc("adjustments"),0,1,1);glUniform4f(loc("adjustMore"),0,0,1,0);glUniform3f(loc("geometry"),0,1,1);glUniform2f(loc("aspect"),float(w)/h,1);int cases=0;bool matrix=true;std::vector<unsigned char> rgba(w*h*4);
 for(int effect=1;effect<=8;effect++)for(int curve=0;curve<3;curve++)for(int origin=0;origin<3;origin++)for(int dir=0;dir<(effect==5||effect==6?4:1);dir++)for(int zoom=0;zoom<(effect==3||effect==4?3:1);zoom++){
  glUniform1i(loc("effect"),effect);glUniform1i(loc("curve"),curve);glUniform1i(loc("direction"),dir);glUniform1f(loc("zoomAmount"),zoom==0?.05f:zoom==1?.3f:.6f);glUniform2f(loc("origin"),origin==0?.5f:origin==1?.03f:.97f,origin==0?.5f:origin==1?.97f:.03f);int lastBlue=0;
  for(float progress:{0.f,.1f,.25f,.5f,.75f,.9f,1.f}){
   glUniform1f(loc("progress"),progress);glDrawArrays(GL_TRIANGLES,0,3);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());int blue=0,black=0;
   for(int n=0;n<w*h;n++){auto p=&rgba[n*4];matrix&=p[3]==255;if(progress==0)matrix&=p[0]==180&&p[1]==30&&p[2]==60;if(progress==1)matrix&=p[0]==20&&p[1]==60&&p[2]==180;blue+=p[2]>150&&p[0]<50;black+=p[0]+p[1]+p[2]<10;}
   if(effect!=2&&effect!=8)matrix&=black==0;
   if(effect==7){matrix&=blue>=lastBlue;if(progress==.75f)matrix&=blue<w*h;lastBlue=blue;}
   if(effect==8&&progress==.5f)matrix&=black>w*h*.99f;
  }cases++;
 }
 glBindVertexArray(0);glUseProgram(0);EndTextureMode();glDeleteProgram(program);glDeleteVertexArrays(1,&vao);glDeleteTextures(2,tex);UnloadRenderTexture(target);CloseWindow();std::cout<<"{\"custom_shader_cases\":"<<cases<<",\"matrix_passed\":"<<matrix<<"}\n";std::cout<<"{\"rgba\":["<<int(pixel[0])<<","<<int(pixel[1])<<","<<int(pixel[2])<<","<<int(pixel[3])<<"],\"mask_restored\":"<<preserved<<"}\n";return !matrix||!preserved||pixel[0]!=180||pixel[1]!=30||pixel[2]!=60||pixel[3]!=255;
}
