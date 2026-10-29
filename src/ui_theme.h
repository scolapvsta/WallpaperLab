#pragma once
#include "theme_catalog.h"
static Shader backdropShader;
static int backdropResolution,backdropTime,backdropStyle,backdropBase,backdropAccent,backdropAlpha;
static float themeRoundness=1,uiSurfaceAlpha=1,themeTransparencyPreview=-1;
static int themeId=0;
static Color ThemeHex(unsigned int rgb){return (Color){rgb>>16,(rgb>>8)&255,rgb&255,255};}
#include "strawberry_ui.h"
#include "cyber_ui.h"
#include "pixel_ui.h"
static void ApplyTheme(void){
 themeId=lab_option("ui_theme");if(themeId<0||themeId>=LAB_THEME_COUNT)themeId=0;LabTheme resolved=LabResolvedPalette(themeId,lab_option("light_theme"));const LabTheme* t=&resolved;
 uiSurfaceAlpha=1-Clamp(themeTransparencyPreview>=0?themeTransparencyPreview:lab_option("window_transparency"),0,30)/100.f;
 AppTheme.bg=ThemeHex(t->bg);surface=ThemeHex(t->surface);AppTheme.text=ThemeHex(t->text);muted=ThemeHex(t->muted);AppTheme.border=ThemeHex(t->accent);themeRoundness=StrawberryTheme()?1.7f:t->radius;
 if(!StrawberryTheme()&&t->effect==7&&!lab_option("reduce_motion"))AppTheme.border=ColorFromHSV(IsWindowReady()?fmodf(GetTime()*10,360):215,.55f,1);
 control=ColorLerp(surface,AppTheme.text,t->light?.055f:.055f);controlHover=ColorLerp(surface,AppTheme.border,t->light?.13f:.19f);lineColor=ColorLerp(surface,AppTheme.text,t->light?.19f:.16f);
 AppTheme.bg.a=surface.a=control.a=controlHover.a=roundf(uiSurfaceAlpha*255);AppTheme.idle=control;AppTheme.hover=controlHover;
}
static void ThemePanel(Rectangle r,float roundness,int segments,Color color){
 if(CyberTheme()){CyberPanel(r,color);return;}
 if(PixelTheme()){PixelPanel(r,color);return;}
 if(DefaultTheme()){DrawRectangleRounded(r,Clamp(roundness,0,1),segments,color);if(r.width>28&&r.height>24)DrawRectangleRoundedLinesEx(r,Clamp(roundness,0,1),segments,1,Fade(AppTheme.border,.13f));return;}
 if(StrawberryTheme()){
  DrawRectangleRounded(r,Clamp(roundness*1.7f,.08f,1),segments,color);
  if(r.width>90&&r.height>28)DrawRectangleRoundedLinesEx(r,Clamp(roundness*1.7f,.08f,1),segments,1,Fade(AppTheme.border,.25f));
  if(r.width>180&&r.height>220){BerryLace(r.x+8,r.y+1,r.width-16);BerryBow(r.x+r.width-18,r.y+12,.5f);}
  return;
 }
 float radius=Clamp(roundness*themeRoundness,0,1);int effect=labThemes[themeId].effect;
 if((effect==1||effect==3&&!lab_option("light_theme")||effect==6||effect==7)&&r.width>28&&r.height>24){
  for(int n=3;n>0;n--)DrawRectangleRoundedLinesEx((Rectangle){r.x-n,r.y-n,r.width+2*n,r.height+2*n},radius,segments,1,Fade(AppTheme.border,.035f));
 }
 if(effect==11){DrawRectangleRec(r,color);DrawRectangleLinesEx(r,1,Fade(AppTheme.border,.38f));if(r.width>400&&r.height>250){for(int n=0;n<3;n++){DrawRectangle(r.x+5+n*6,r.y+5,4,4,Fade(AppTheme.border,.45f));}DrawRectangle(r.x+3,r.y+r.height-4,r.width-6,2,Fade(AppTheme.border,.15f));}}else DrawRectangleRounded(r,radius,segments,color);
 if((effect==1||(effect==3&&!lab_option("light_theme"))||effect==6||effect==7)&&r.width>28&&r.height>24){
  DrawRectangleRoundedLinesEx(r,radius,segments,1,Fade(AppTheme.border,.24f));
  if(r.width>400&&r.height>250){DrawRectangleGradientV(r.x+16,r.y+2,r.width-32,48,Fade(AppTheme.border,.065f),BLANK);float l=16;Color rim=Fade(AppTheme.border,.55f);DrawLineEx((Vector2){r.x+12,r.y+12+l},(Vector2){r.x+12,r.y+12},1.5f,rim);DrawLineEx((Vector2){r.x+12,r.y+12},(Vector2){r.x+12+l,r.y+12},1.5f,rim);DrawLineEx((Vector2){r.x+r.width-12-l,r.y+r.height-12},(Vector2){r.x+r.width-12,r.y+r.height-12},1.5f,rim);DrawLineEx((Vector2){r.x+r.width-12,r.y+r.height-12},(Vector2){r.x+r.width-12,r.y+r.height-12-l},1.5f,rim);if(effect==6)for(int y=r.y+45;y<r.y+r.height-35;y+=16)DrawRectangle(r.x+3,y,2,4,Fade(AppTheme.border,.38f));}
 }
 if(effect==2&&r.width>28&&r.height>24){DrawRectangleRoundedLinesEx(r,radius,segments,1,Fade(AppTheme.border,.22f));DrawLine(r.x+8,r.y+1,r.x+r.width-8,r.y+1,Fade(WHITE,.13f));}
 if(effect==10&&r.width>90&&r.height>34)DrawRectangleRoundedLinesEx(r,radius,segments,1,Fade(AppTheme.border,.12f));
}
static void ThemeButton(Rectangle r,Color color,bool textOnly){
 if(StrawberryTheme())BerryCookie(r,color);else ThemePanel(r,.22f,12,color);
 if(CyberTheme()&&textOnly&&r.width>70){DrawRectangle(r.x+6,r.y+8,2,fminf(12,r.height-16),Fade(CyberSecondary(),.65f));}
}
static void ButtonFeedback(Rectangle r,bool hot,bool pressed,bool active){
 if(StrawberryTheme()){
  if(hot||active)DrawRectangleRoundedLinesEx((Rectangle){r.x+1,r.y+1,r.width-2,r.height-2},1,20,hot?2:1.3f,Fade(AppTheme.border,pressed?.9f:hot?.75f:.5f));
  if(hot)DrawRectangleRounded((Rectangle){r.x+5,r.y+4,r.width-10,r.height-8},1,20,Fade(pressed?ThemeHex(0xc9974f):WHITE,pressed?.15f:.2f));
 }else if(PixelTheme()&&hot){DrawRectangleLinesEx(r,2,AppTheme.border);if(pressed)DrawRectangle(r.x+2,r.y+2,r.width-4,r.height-4,Fade(AppTheme.border,.2f));}
}
static void InitBackdrop(void){
 const char* fs="#version 330 core\nin vec2 fragTexCoord;out vec4 finalColor;uniform vec2 resolution;uniform float time,style,alpha;uniform vec3 base,accent;\n"
 "void main(){vec2 uv=gl_FragCoord.xy/resolution;float t=time*.04;float halo=exp(-length(uv-vec2(.15+.05*sin(t),.8))*4.)*.08+exp(-length(uv-vec2(.85,.2))*5.)*.035;vec3 c=base;"
 "if(style==1.||style==5.||style==7.){c+=accent*halo;float grid=(1.-smoothstep(0.,.7,abs(mod(gl_FragCoord.x,48.)-24.)))*(1.-uv.y)*.018;c+=accent*grid;}"
 "if(style==2.)c+=accent*halo*1.8+vec3(.025,.01,.04)*sin(uv.x*5.+uv.y*4.+t);"
 "if(style==11.){vec2 cell=floor(gl_FragCoord.xy/8.);float grid=step(.96,fract(sin(dot(cell,vec2(12.9898,78.233)))*43758.5453));c+=accent*grid*.025;}if(style==3.)c+=accent*halo*.7;if(style==4.||style==9.||style==10.)c+=accent*halo*.2;"
 "if(style==6.){vec2 p=mod(gl_FragCoord.xy,24.);float dotLed=(1.-smoothstep(.3,1.2,length(p-12.)))*.09;c+=accent*dotLed*(.6+.4*sin(time*.5+uv.x*5.));}"
 "finalColor=vec4(c,alpha);}";
 backdropShader=LoadShaderFromMemory(NULL,fs);backdropResolution=GetShaderLocation(backdropShader,"resolution");backdropTime=GetShaderLocation(backdropShader,"time");backdropStyle=GetShaderLocation(backdropShader,"style");backdropBase=GetShaderLocation(backdropShader,"base");backdropAccent=GetShaderLocation(backdropShader,"accent");backdropAlpha=GetShaderLocation(backdropShader,"alpha");
}
static Texture2D appBackgroundTexture;
static char appBackgroundLoaded[4096]="";
static bool DrawAppBackground(int w,int h){
 const char* path=lab_app_background();
 if(strcmp(path,appBackgroundLoaded)){
  if(appBackgroundTexture.id){UnloadTexture(appBackgroundTexture);appBackgroundTexture=(Texture2D){0};}
  snprintf(appBackgroundLoaded,sizeof(appBackgroundLoaded),"%s",path);
  if(*path){Image image=LoadImage(path);if(image.data){appBackgroundTexture=LoadTextureFromImage(image);UnloadImage(image);SetTextureFilter(appBackgroundTexture,TEXTURE_FILTER_BILINEAR);}}
 }
 if(!appBackgroundTexture.id)return false;
 float scale=fmaxf(w/(float)appBackgroundTexture.width,h/(float)appBackgroundTexture.height);
 Rectangle dst={(w-appBackgroundTexture.width*scale)/2,(h-appBackgroundTexture.height*scale)/2,appBackgroundTexture.width*scale,appBackgroundTexture.height*scale};
 DrawTexturePro(appBackgroundTexture,(Rectangle){0,0,appBackgroundTexture.width,appBackgroundTexture.height},dst,(Vector2){0},0,Fade(WHITE,uiSurfaceAlpha));
 DrawRectangle(0,0,w,h,Fade(AppTheme.bg,lab_option("light_theme")?.12f:.25f));return true;
}
static RenderTexture2D backgroundPreview;
static int previewTheme=-1,previewDesign=-1,previewPreset=-1;static char previewPath[4096]="";
static void DrawBackgroundPreview(Rectangle r){
 const char* path=lab_app_background();int preset=lab_option("app_background_preset"),design=lab_option("ui_design");
 if(!backgroundPreview.id||previewTheme!=themeId||previewDesign!=design||previewPreset!=preset||strcmp(path,previewPath)){
  if(!backgroundPreview.id)backgroundPreview=LoadRenderTexture(320,180);
  rlDisableScissorTest();BeginTextureMode(backgroundPreview);ClearBackground(AppTheme.bg);rlPushMatrix();rlScalef(.25f,.25f,1);
  if(!DrawAppBackground(1280,720)){if(CyberTheme())CyberCityArtwork(1280,720);else if(PixelTheme())PixelBackdrop(1280,720);else if(DefaultTheme())DefaultBackdrop(1280,720);else BerryBackdrop(1280,720);}
  rlPopMatrix();EndTextureMode();if(uiCanvasActive)rlEnableFramebuffer(uiCanvas.id);BeginScissorMode(inputClip.x,inputClip.y,inputClip.width,inputClip.height);
  previewTheme=themeId;previewDesign=design;previewPreset=preset;snprintf(previewPath,sizeof(previewPath),"%s",path);
 }
 ThemePanel(r,.1f,8,control);DrawTexturePro(backgroundPreview.texture,(Rectangle){0,0,320,-180},(Rectangle){r.x+2,r.y+2,r.width-4,r.height-4},(Vector2){0},0,WHITE);
}
static void DrawBackdrop(int width,int height){
 if(DrawAppBackground(width,height))return;
 if(CyberTheme()){CyberBackdrop(width,height);return;}
 if(PixelTheme()){PixelBackdrop(width,height);return;}
 if(DefaultTheme()){DefaultBackdrop(width,height);return;}
 if(StrawberryTheme()){BerryBackdrop(width,height);return;}
 int effect=labThemes[themeId].effect;
 if(!effect||!backdropShader.id){DrawRectangle(0,0,width,height,AppTheme.bg);return;}
 Vector2 resolution={width,height};float time=lab_option("reduce_motion")?0:GetTime(),style=effect;float base[]={AppTheme.bg.r/255.f,AppTheme.bg.g/255.f,AppTheme.bg.b/255.f},accent[]={AppTheme.border.r/255.f,AppTheme.border.g/255.f,AppTheme.border.b/255.f};
 SetShaderValue(backdropShader,backdropResolution,&resolution,SHADER_UNIFORM_VEC2);SetShaderValue(backdropShader,backdropTime,&time,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropStyle,&style,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropBase,base,SHADER_UNIFORM_VEC3);SetShaderValue(backdropShader,backdropAccent,accent,SHADER_UNIFORM_VEC3);SetShaderValue(backdropShader,backdropAlpha,&uiSurfaceAlpha,SHADER_UNIFORM_FLOAT);BeginShaderMode(backdropShader);DrawRectangle(0,0,width,height,WHITE);EndShaderMode();
 if(effect==3||effect==2){for(int n=0;n<10;n++){float px=fmodf(n*173.3f+width*.13f+sinf(time*.2f+n)*24,width),py=fmodf(n*97.1f+time*(effect==3?8:3),height);DrawEllipse(px,py,effect==3?2.5f:1.4f,effect==3?1.3f:1.4f,Fade(AppTheme.border,lab_option("light_theme")?.12f:.1f));}}
}
static void ThemeWindowTrim(int sw,int sh){
 if(CyberTheme()){
  DrawLine(2,2,sw-2,2,Fade(AppTheme.border,.6f));
  for(int n=0;n<12;n++)DrawRectangle(sw-150+n*11,sh-4,6,2,Fade(n%3?AppTheme.border:CyberSecondary(),lab_option("reduce_motion")?.6f:.45f+.25f*sinf(GetTime()*2+n*.6f)));return;
 }

 if(StrawberryTheme()){DrawRectangleRoundedLinesEx((Rectangle){2,2,sw-4,sh-4},.02f,12,2,Fade(AppTheme.border,.4f));return;}
 int effect=labThemes[themeId].effect;if(!effect)return;
 if(effect==7){for(int x=0;x<sw;x+=8)DrawRectangle(x,0,8,2,ColorFromHSV(fmodf(x*360.f/sw+(lab_option("reduce_motion")?0:GetTime()*10),360),.6f,.9f));}
 else DrawRectangleRoundedLinesEx((Rectangle){1,1,sw-2,sh-2},effect==6||effect==11?.002f:.018f,12,effect==1||effect==3?1.5f:1,Fade(AppTheme.border,effect==1||effect==3?.48f:.2f));
}
// Compact switcher skin shared by the live popup and all four UI previews.
static void SwitcherBackdrop(int w,int h,bool wave){
 Rectangle tray=wave?(Rectangle){0,h*.29f,w,h*.42f}:(Rectangle){0,0,w,h};ThemePanel(tray,.12f,12,Fade(surface,.96f));
 if(StrawberryTheme()){
  if(w>60){BerryLace(tray.x+5,tray.y+1,w-10);BerryBow(w-18,tray.y+12,.45f);}
 }else if(CyberTheme()){
  for(int x=10;x<w-10;x+=34)DrawRectangle(x,tray.y+tray.height-5,12,2,Fade(AppTheme.border,.38f));
  DrawLine(5,tray.y+5,5,tray.y+tray.height-5,Fade(CyberSecondary(),.5f));
 }else if(PixelTheme()){
  for(int x=4;x<w;x+=16)DrawRectangle(x,tray.y+tray.height-6,10,3,Fade(AppTheme.border,.26f));
  if(w>60)PixelCharacter(8,tray.y+8,1,2,lab_option("reduce_motion")?0:(int)(GetTime()*4));
 }
}
static void SwitcherCardDecoration(Rectangle r,bool current){
 if(StrawberryTheme())BerryBow(r.x+r.width-15,r.y+r.height-15,.3f);
 else if(PixelTheme()){PixelCharacter(r.x+r.width-19,r.y+r.height-19,1,2,0);if(current)DrawRectangleLinesEx((Rectangle){r.x+3,r.y+3,r.width-6,r.height-6},2,AppTheme.border);}
 else if(CyberTheme()){DrawLine(r.x+6,r.y+r.height-7,r.x+22,r.y+r.height-7,Fade(AppTheme.border,.8f));DrawLine(r.x+6,r.y+r.height-7,r.x+6,r.y+r.height-20,Fade(CyberSecondary(),.7f));}
}
