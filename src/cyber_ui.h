#pragma once
static bool CyberTheme(void){return lab_option("ui_design")==2;}
static RenderTexture2D cyberCity;
static int cyberCityPalette=-1,cyberCityLight=-1,cyberCityPreset=-1;
static Color CyberSecondary(void){return themeId==8?ThemeHex(0xffd16d):themeId==7?ThemeHex(0xff734d):themeId==5?ThemeHex(0xff65c5):ThemeHex(0x49deee);}
static void CyberGlowLine(Vector2 a,Vector2 b,Color color){
 DrawLineEx(a,b,7,Fade(color,.04f));DrawLineEx(a,b,3,Fade(color,.13f));DrawLineEx(a,b,1,Fade(color,.8f));
}
static void CyberPanel(Rectangle r,Color color){
 float c=fminf(9,fminf(r.width,r.height)*.18f);Vector2 points[8]={{r.x+c,r.y},{r.x+r.width,r.y},{r.x+r.width,r.y+r.height-c},{r.x+r.width-c,r.y+r.height},{r.x,r.y+r.height},{r.x,r.y+c},{r.x+c,r.y},{r.x+r.width,r.y}};
 Vector2 center={r.x+r.width/2,r.y+r.height/2};for(int n=0;n<6;n++){int next=(n+1)%6;DrawTriangle(center,points[next],points[n],color);}
 if(r.width>26&&r.height>22){
  for(int n=0;n<6;n++)CyberGlowLine(points[n],points[(n+1)%6],Fade(AppTheme.border,.55f));
  if(!lab_option("reduce_motion")&&r.width>90&&r.height>60&&fmodf(GetTime()+r.x*.0007f+r.y*.001f,3.5f)<.14f)DrawRectangle(r.x+r.width*.3f,r.y+r.height-2,r.width*.3f,1,Fade(CyberSecondary(),.8f));

  if(r.width>180&&r.height>200){for(int n=0;n<5;n++)DrawRectangle(r.x+r.width-8,r.y+24+n*9,3,4,Fade(AppTheme.border,n%2?.6f:.25f));}
 }
}
static void CyberCityArtwork(int w,int h){
 unsigned char bgAlpha=AppTheme.bg.a,surfaceAlpha=surface.a;AppTheme.bg.a=surface.a=255;
 int preset=lab_option("app_background_preset");Color secondary=preset==3?ThemeHex(0xd5ff42):CyberSecondary();int horizon=h*(preset==1?.84f:.77f);
 if(preset==2){
  DrawRectangleGradientV(0,0,w,h,AppTheme.bg,ColorLerp(AppTheme.bg,AppTheme.border,.12f));
  for(int x=12;x<w;x+=24)for(int y=8;y<h;y+=19){unsigned code=(x*13+y*37)%17;DrawText(code%2?"1":"0",x,y,10,Fade(code<3?secondary:AppTheme.border,code<3?.55f:.12f));}
  for(int x=10;x<w;x+=120){DrawRectangle(x,0,1,h,Fade(secondary,.12f));DrawRectangle(x+4,h*.3f,24,h*.18f,Fade(AppTheme.border,.04f));}
  AppTheme.bg.a=bgAlpha;surface.a=surfaceAlpha;return;
 }
 DrawRectangleGradientV(0,0,w,h,AppTheme.bg,ColorLerp(AppTheme.bg,AppTheme.border,.16f));
 // Three deterministic skyline layers, avenue perspective and reflected shop signs.
 for(int layer=0;layer<3;layer++)for(int i=0;i<24;i++){
  unsigned seed=(i+1)*7919u+layer*2197u+preset*4127u;int bw=38+seed%75,x=i*w/23-40+(layer-1)*21;
  int bh=90+(seed/17)%(h/2);if(layer==0)bh+=h*.17f;
  int y=horizon-bh;Color building=ColorLerp(AppTheme.bg,surface,.18f+layer*.24f);
  DrawRectangle(x,y,bw,bh,building);DrawLine(x,y,x+bw,y,Fade(secondary,.2f));
  if(i%3==0){DrawLine(x+bw/2,y-45,x+bw/2,y,Fade(AppTheme.border,.45f));DrawCircle(x+bw/2,y-46,2,AppTheme.border);}
  for(int wy=y+9;wy<horizon-6;wy+=12)for(int wx=x+6;wx<x+bw-4;wx+=9){unsigned lit=(wx*13u+wy*7u+seed)%11;if(lit<5)DrawRectangle(wx,wy,lit==0?5:3,2,Fade(lit%2?secondary:AppTheme.border,.12f+layer*.1f));}
  if(layer==2&&i%3==0){
   int sx=x+bw*.55f,sy=y+25,sh=fminf(120,bh-35);DrawRectangle(sx,sy,18,sh,ColorLerp(building,AppTheme.border,.23f));
   CyberGlowLine((Vector2){sx,sy},(Vector2){sx,sy+sh},AppTheme.border);
   for(int gy=sy+7;gy<sy+sh-12;gy+=20){
    // Katakana-inspired geometric sign lettering, drawn independently of the UI font.
    DrawLine(sx+4,gy,sx+13,gy+2,secondary);DrawLine(sx+10,gy-2,sx+7,gy+12,secondary);DrawLine(sx+4,gy+7,sx+13,gy+7,secondary);
   }
   DrawRectangle(sx-5,horizon+12,25,38+(i%4)*16,Fade(AppTheme.border,.07f));
  }
  if(layer==1&&i%5==0){Rectangle sign={x-8,y+bh*.4f,bw+7,30};DrawRectangleRec(sign,ColorLerp(building,secondary,.15f));DrawRectangleLinesEx(sign,1,Fade(secondary,.7f));DrawText(i%2?"NIGHT // 24H":"2077",sign.x+5,sign.y+10,10,AppTheme.border);}
 }
 for(int n=0;n<11;n++){
  float ex=w*.5f+(n-5)*w*.23f;CyberGlowLine((Vector2){w*.5f,horizon},(Vector2){ex,h},Fade(secondary,.18f));
  int y=horizon+powf(n/10.f,2)*(h-horizon);DrawLine(0,y,w,y,Fade(AppTheme.border,.10f));
 }
 for(int n=0;n<100;n++){int x=(n*197+47)%w,y=horizon+(n*53)%(h-horizon);DrawRectangle(x,y,7+(n%6)*9,1,Fade(n%2?secondary:AppTheme.border,.1f));}
 // Overhead cables frame the street rather than crossing foreground content.
 if(preset==3){
  Vector2 a={w*.66f,h*.12f},b={w*.47f,h*.42f},c={w*.59f,h*.42f},d={w*.39f,h*.72f};
  DrawLineEx(a,b,w*.018f,Fade(secondary,.32f));DrawLineEx(b,c,w*.018f,Fade(secondary,.32f));DrawLineEx(c,d,w*.018f,Fade(secondary,.32f));
  DrawRing((Vector2){w*.54f,h*.42f},h*.23f,h*.232f,0,300,64,Fade(AppTheme.border,.25f));
 }
 for(int n=0;n<3;n++)DrawLineBezier((Vector2){0,90+n*17},(Vector2){w*.4f,205+n*11},2,Fade(AppTheme.text,.12f));
 AppTheme.bg.a=bgAlpha;surface.a=surfaceAlpha;
}
static void CyberBackdrop(int w,int h){
 int light=lab_option("light_theme"),preset=lab_option("app_background_preset");
 if(!cyberCity.id||cyberCity.texture.width!=w||cyberCity.texture.height!=h||cyberCityPalette!=themeId||cyberCityLight!=light||cyberCityPreset!=preset){
  if(cyberCity.id)UnloadRenderTexture(cyberCity);cyberCity=LoadRenderTexture(w,h);BeginTextureMode(cyberCity);ClearBackground(AppTheme.bg);CyberCityArtwork(w,h);EndTextureMode();if(uiCanvasActive)rlEnableFramebuffer(uiCanvas.id);cyberCityPalette=themeId;cyberCityLight=light;cyberCityPreset=preset;
 }
 DrawTexturePro(cyberCity.texture,(Rectangle){0,0,w,-h},(Rectangle){0,0,w,h},(Vector2){0},0,Fade(WHITE,uiSurfaceAlpha));
 float t=lab_option("reduce_motion")?0:GetTime();
 for(int n=0;n<45;n++){float x=(n*137+83)%w,y=fmodf(n*83+t*(70+n%30),h);DrawLine(x,y,x-2,y+13,Fade(CyberSecondary(),.11f));}
 for(int y=0;y<h;y+=5)DrawRectangle(0,y,w,1,Fade(AppTheme.bg,.16f));
 // Occasional low-opacity glitch strips stay in the decorative backdrop.
 if(!lab_option("reduce_motion")&&fmodf(t,8)<.13f){int y=((int)(t*8)*71)%h;DrawRectangle(w*.1f,y,w*.46f,3,Fade(AppTheme.border,.24f));DrawRectangle(w*.56f,y+9,w*.3f,2,Fade(CyberSecondary(),.2f));}
}
