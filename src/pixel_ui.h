#pragma once
static bool DefaultTheme(void){return ActiveThemeDesign()==0;}
static bool PixelTheme(void){return ActiveThemeDesign()==3;}
// Original 12x12 characters: adventurer, slime, robot and ghost.
static void PixelCharacter(float x,float y,float scale,int kind,int frame){
 scale=fmaxf(1,floorf(scale));static const char* art[]={
 "....hhhh....""...hhhhhh...""...hffffh...""...hfkfkh...""....ffff....""...aaaaaa...""..aawwaaaa..""..ffwwaaff..""....aaaa....""...aa..aa...""...kk..kk...""..kk....kk..",
 "............""............""............""....aaaa....""..aaaaaaaa.."".aaaaaaaaaa."".aakkaakkaa.""aaakkaakkaaa""aaaaaaaaaaaa"".aaffffaa aa""..aaaaaaaa..""...aa..aa...",
 "....wwww....""...aaaaaa...""..aawwwwaa..""..awkaakwa..""..awwwwwwa..""...aaaaaa...""..aawwwwaa.."".aaawaaawaa."".wwaaaaaaww.""...aa..aa...""...ww..ww...""..kkk..kkk..",
 "....aaaa....""..aaaaaaaa.."".aaaaaaaaaa."".aawwaawwaa.""aaakw aakwaa""aaaaaaaaaaaa""aaaaaffaaaaa""aaaaaaaaaaaa"".aaaaaaaaaa."".aa..aa..aa.""............""............"};
 const char* sprite=art[kind%4];Color outline=ColorLerp(AppTheme.bg,BLACK,.25f),accent=AppTheme.border;
 for(int row=0;row<12;row++)for(int col=0;col<12;col++){
  char c=sprite[row*12+col];if(c=='.'||c==' '||!c)continue;
  Color ink=c=='k'?outline:c=='w'?ThemeHex(0xfff4dc):c=='f'?ThemeHex(0xf5bca0):c=='h'?ColorLerp(accent,BLACK,.3f):accent;
  float step=row>=9&&(frame%2)&&kind%4==0?(col<6?-1:1):0;DrawRectangle(x+(col+step)*scale,y+row*scale,scale,scale,ink);
 }
}
static void PixelPanel(Rectangle r,Color color){
 Color rim=Fade(AppTheme.border,.45f);DrawRectangle(r.x+3,r.y+3,r.width,r.height,Fade(BLACK,.32f));DrawRectangleRec(r,color);DrawRectangleLinesEx(r,2,rim);
 if(r.width>24&&r.height>24){DrawRectangle(r.x+3,r.y+3,r.width-6,2,Fade(WHITE,.08f));DrawRectangle(r.x+r.width-5,r.y+r.height-5,3,3,AppTheme.border);}
}
static void DefaultBackdrop(int w,int h){
 int p=ActiveBackgroundPreset();DrawRectangleGradientV(0,0,w,h,AppTheme.bg,ColorLerp(AppTheme.bg,AppTheme.border,.045f));
 if(p==1)for(int x=0;x<w;x+=48)for(int y=0;y<h;y+=48){DrawLine(x,y,x+8,y,Fade(AppTheme.border,.1f));DrawLine(x,y,x,y+8,Fade(AppTheme.border,.1f));}
 if(p==2)for(int i=0;i<12;i++)DrawEllipseLines(w*.8f,h*.68f,100+i*30,45+i*22,Fade(AppTheme.border,.055f));
 if(p==3)for(int x=18;x<w;x+=36)for(int y=18;y<h;y+=36)DrawCircle(x,y,1,Fade(AppTheme.border,.1f));
}
// Original geometric pixel artwork, drawn at integer sprite coordinates.
static Color ArcadeCyan(void){return ColorLerp(ThemeHex(0x63efff),AppTheme.border,.18f);}
static void ArcadeStar(float x,float y,int u,Color color){
 DrawRectangle(x-u,y-3*u,2*u,6*u,color);DrawRectangle(x-3*u,y-u,6*u,2*u,color);DrawRectangle(x-u,y-u,2*u,2*u,ThemeHex(0xfffae0));
}
static void ArcadeCabinet(float x,float y,float s,int kind){
 Color pink=AppTheme.border,cyan=ArcadeCyan(),shell=ColorLerp(surface,pink,.16f),dark=ColorLerp(AppTheme.bg,BLACK,.3f);
 rlPushMatrix();rlTranslatef(roundf(x),roundf(y),0);rlScalef(s,s,1);
 DrawRectangle(3,5,42,75,dark);DrawRectangle(0,0,42,76,shell);DrawRectangleLinesEx((Rectangle){0,0,42,76},1,pink);
 DrawRectangle(3,3,36,11,pink);DrawRectangle(7,6,28,3,ThemeHex(0xfff2ad));
 DrawRectangle(4,18,34,30,dark);DrawRectangleLinesEx((Rectangle){4,18,34,30},1,cyan);
 if(kind%2){ArcadeStar(21,32,2,ThemeHex(0xffdf6f));}else{PixelCharacter(13,24,1,2,0);}
 DrawRectangle(-3,49,48,10,pink);DrawRectangle(-1,49,44,2,cyan);DrawRectangle(8,47,2,7,dark);DrawRectangle(6,46,6,3,cyan);
 DrawRectangle(27,52,4,3,ThemeHex(0xffd66f));DrawRectangle(34,52,4,3,cyan);
 DrawRectangle(10,63,22,9,dark);DrawRectangle(16,65,9,2,pink);DrawRectangle(3,76,7,4,dark);DrawRectangle(32,76,7,4,dark);
 rlPopMatrix();
}
static void ArcadePrizeMachine(float x,float y,float s){
 Color pink=AppTheme.border,cyan=ArcadeCyan(),cream=ThemeHex(0xfff2db);
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(s,s,1);
 DrawRectangle(4,5,68,100,ColorLerp(AppTheme.bg,BLACK,.25f));
 DrawRectangle(0,0,68,98,ColorLerp(surface,pink,.25f));DrawRectangleLinesEx((Rectangle){0,0,68,98},2,pink);
 DrawRectangle(4,12,60,58,ColorLerp(AppTheme.bg,cyan,.16f));
 DrawRectangle(8,17,4,46,Fade(cream,.3f));DrawRectangle(15,17,2,46,Fade(cream,.16f));
 DrawRectangle(2,3,64,8,pink);DrawRectangle(7,5,54,3,cream);ArcadeStar(34,0,3,ThemeHex(0xffdc65));
 DrawRectangle(32,14,2,16,cream);DrawLine(33,30,25,39,cyan);DrawLine(33,30,41,39,cyan);DrawLine(25,39,29,43,cyan);DrawLine(41,39,37,43,cyan);
 for(int i=0;i<5;i++){int px=8+(i%3)*18,py=51+(i/3)*10;DrawRectangle(px,py,14,11,i%2?pink:cyan);DrawRectangle(px+2,py-3,10,3,i%2?pink:cyan);DrawRectangle(px+3,py+2,2,4,cream);DrawRectangle(px+9,py+2,2,4,cream);}
 DrawRectangle(2,13,3,57,cyan);DrawRectangle(63,13,3,57,cream);DrawRectangle(-4,71,76,11,pink);DrawRectangle(-3,71,74,2,cream);
 DrawRectangle(12,74,2,6,cyan);DrawRectangle(9,73,8,3,cream);DrawRectangle(47,75,7,3,ThemeHex(0xffdc65));
 DrawRectangle(23,86,23,8,AppTheme.bg);DrawRectangle(26,87,17,2,cyan);DrawRectangle(4,98,8,5,cyan);DrawRectangle(56,98,8,5,cyan);
 rlPopMatrix();
}
static void ArcadeGhost(float x,float y,int u,Color body,bool eyesOnly,int frame){
 if(!eyesOnly){
  DrawRectangle(x+3*u,y,6*u,2*u,body);DrawRectangle(x+u,y+2*u,10*u,3*u,body);DrawRectangle(x,y+5*u,12*u,5*u,body);
  for(int n=0;n<3;n++)DrawRectangle(x+(n*4+(frame%2))*u,y+10*u,2*u,2*u,body);
 }
 for(int n=0;n<2;n++){DrawRectangle(x+(2+n*5)*u,y+3*u,3*u,4*u,ThemeHex(0xfffaf0));DrawRectangle(x+(3+n*5)*u,y+4*u,u,2*u,ThemeHex(0x282266));}
}
static void ArcadePac(float x,float y,int u,bool left,float t){
 float mouth=.12f+.65f*fabsf(sinf(t*11));
 for(int row=-6;row<=6;row++)for(int col=-6;col<=6;col++){
  float dx=left?-col:col;
  if(col*col+row*row>37||(dx>=0&&fabsf(row)<dx*mouth))continue;
  DrawRectangle(x+col*u,y+row*u,u,u,ThemeHex(0xffdc65));
 }
}
// A deterministic chase, power-pellet reversal, four captures and returning eyes.
// Reduced motion uses one fixed, readable chase frame.
static void ArcadeChaseFrame(Rectangle lane,float t){
 float travel=fmaxf(40,lane.width-54),x;
 int u=lane.height>=40?2:1;float y=lane.y+lane.height/2;
 bool powered=t>=4;float progress=Clamp((t-4)/6,0,1);
 x=lane.x+26+travel*(powered?.12f+.82f*progress:.88f-.76f*t/4);
 DrawRectangle(lane.x+10,y+10*u,lane.width-20,1,Fade(ArcadeCyan(),.35f));
 for(int i=0;i<24;i++){float dot=lane.x+26+i*travel/24;if(!powered&&dot<x)DrawRectangle(dot,y,2,2,ThemeHex(0xffeec1));}
 if(!powered)DrawCircle(lane.x+26+travel*.12f,y,4,ThemeHex(0xfff2c4));
 Color colors[]={ThemeHex(0xff6a9a),ThemeHex(0x75edff),ThemeHex(0xc7a1ff),ThemeHex(0xffbf78)};
 for(int i=0;i<4;i++){
  float capture=5.3f+i*1.05f;bool eaten=t>=capture;
  float gx=powered?lane.x+26+travel*(.12f+.82f*Clamp((capture-4)/6,0,1))+(t-capture)*travel*.025f:x+(24+i*21)*u;
  if(eaten){float home=Clamp((t-capture)/2,0,1);gx=gx*(1-home)+(lane.x+lane.width*.5f)*home;}
  gx=Clamp(gx,lane.x+8,lane.x+lane.width-14*u);
  ArcadeGhost(gx,y-6*u,u,powered?ThemeHex(0x6879ef):colors[i],eaten,(int)(t*7));
  if(eaten&&t<capture+.6f)DrawText(TextFormat("%d",200<<i),gx,y-17*u,10,ThemeHex(0xfff0bf));
 }
 ArcadePac(x,y,u,!powered,t);
}
static void ArcadeChase(Rectangle lane){ArcadeChaseFrame(lane,lab_option("reduce_motion")?2:fmodf(GetTime(),14));}
static void PixelBackdrop(int w,int h){
 int p=ActiveBackgroundPreset();float t=lab_option("reduce_motion")?0:GetTime();Color cyan=ArcadeCyan(),pink=AppTheme.border;
 DrawRectangleGradientV(0,0,w,h,AppTheme.bg,ColorLerp(AppTheme.bg,pink,.17f));
 // Pixel constellations and candy-coloured nebula bands connect all four scenes.
 for(int i=0;i<42;i++){int x=(i*197+37)%w,y=(i*83+19)%(int)(h*.85f)+(int)roundf(sinf(t+i)*7);if(i%5==0)ArcadeStar(x,y,1+i%2,Fade(i%2?pink:cyan,.3f+.12f*sinf(t+i)));else DrawRectangle(x,y,2,2,Fade(AppTheme.text,.28f));}
 if(p==0||p==2){
  int horizon=h*.76f;
  for(int n=0;n<10;n++){int y=horizon+(n*n)*(h-horizon)/100;DrawRectangle(0,y,w,2,Fade(cyan,.16f));DrawLine(w/2,horizon,(n-5)*w*.35f+w/2,h,Fade(pink,.2f));}
  for(int i=0;i<(p==2?5:7);i++){
   float scale=fmaxf(1,floorf(w/(p==2?460.f:420.f))),x=w*(i+.2f)/(p==2?5:7),y=horizon-80*scale+(i%2)*14;
   if(p==2)ArcadePrizeMachine(x,y-20*scale,scale);else ArcadeCabinet(x,y,scale,i);DrawRectangle(x+3,horizon+12,40*scale,3,Fade(i%2?cyan:pink,.16f));
  }
  for(int i=0;i<4;i++){int x=w*(i+.12f)/4;DrawRectangle(x,h*.13f,w*.15f,3,Fade(pink,.25f));DrawRectangle(x,h*.13f+3,w*.15f,3,pink);DrawRectangle(x+5,h*.13f+8,w*.14f,2,Fade(cyan,.4f));}
 }else if(p==1){
  for(int i=0;i<9;i++){int x=i*w/8,y=h*.55f+(i%3)*30;DrawRectangle(x,y,w/8+2,8,Fade(pink,.35f));DrawRectangle(x,y+8,w/8+2,h-y,Fade(surface,.5f));}
  for(int i=0;i<6;i++){int x=w*(i+.5f)/6,y=h*.25f+(i%3)*44;DrawRectangle(x,y,54,12,Fade(cyan,.2f));DrawRectangle(x+8,y-8,36,8,Fade(pink,.25f));ArcadeStar(x+32,y+54,2,ThemeHex(0xffdc79));}
 }else{
  for(int i=0;i<7;i++){int x=w*(i+.2f)/7,y=h*.3f+(i%3)*h*.13f;ArcadeStar(x,y,2+i%3,Fade(i%2?pink:ThemeHex(0xffdc79),.65f));}
  DrawCircle(w*.8f,h*.3f,h*.09f,ColorLerp(surface,pink,.35f));DrawCircle(w*.83f,h*.27f,h*.085f,AppTheme.bg);
 }
 ArcadeChase((Rectangle){16,h*.88f,w-32,h*.09f});
}
