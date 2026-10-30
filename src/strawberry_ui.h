#pragma once
// Original vector artwork: no reference photographs or Workshop assets are shipped.
static bool StrawberryTheme(void){return ActiveThemeDesign()==1;}
static void BerryBow(float x,float y,float s){
 Color edge=Fade(AppTheme.border,.7f),pink=ColorLerp(surface,AppTheme.border,.35f);
 for(int side=-1;side<=1;side+=2){
  DrawTriangle((Vector2){x,y},(Vector2){x+side*18*s,y+10*s},(Vector2){x+side*21*s,y-11*s},pink);
  DrawTriangle((Vector2){x,y},(Vector2){x+side*21*s,y-11*s},(Vector2){x+side*18*s,y+10*s},pink);
  DrawLineEx((Vector2){x,y},(Vector2){x+side*20*s,y-10*s},1*s,edge);
  DrawLineEx((Vector2){x+side*20*s,y-10*s},(Vector2){x+side*18*s,y+10*s},1*s,edge);
  DrawLineEx((Vector2){x+side*18*s,y+10*s},(Vector2){x,y},1*s,edge);
  DrawLineBezier((Vector2){x+side*3*s,y+3*s},(Vector2){x+side*12*s,y+24*s},5*s,pink);
  DrawLineBezier((Vector2){x+side*3*s,y+3*s},(Vector2){x+side*12*s,y+24*s},1*s,edge);
 }
 DrawEllipse(x,y,4*s,5*s,pink);DrawEllipseLines(x,y,4*s,5*s,edge);
}
static void BerryFruit(float x,float y,float s){
 Color red=ColorLerp(ThemeHex(0xe87687),AppTheme.border,.25f),leaf=ThemeHex(0x8caa81);
 // Tapered strawberry with a soft, rounded shoulder.
 for(int i=0;i<20;i++){float t=i/20.f;float half=s*(1-t)*sqrtf(t+.18f)*1.3f;DrawEllipse(x,y-s*.5f+i*s*.075f,half,s*.11f,red);}
 for(int row=0;row<3;row++)for(int col=0;col<3-row;col++)DrawEllipse(x+(col-(2-row)*.5f)*s*.48f,y-s*.18f+row*s*.39f,s*.055f,s*.09f,ThemeHex(0xffebc3));
 for(int n=-1;n<=1;n++)DrawTriangle((Vector2){x,y-s*.45f},(Vector2){x+n*s*.7f-s*.15f,y-s*.9f},(Vector2){x+n*s*.7f+s*.2f,y-s*.72f},leaf);
}
// Minimal confectionery mascot: long ears, dot eyes and a tiny ribbon.
static void BerryBunny(float x,float y,float s){
 Color cream=ThemeHex(0xfffbf7),pink=ColorLerp(surface,AppTheme.border,.22f),ink=ColorLerp(AppTheme.text,AppTheme.border,.15f);
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(s,s,1);
 DrawEllipse(-9,-23,7,21,cream);DrawEllipse(9,-25,7,23,cream);
 DrawEllipse(-9,-25,2.5f,13,pink);DrawEllipse(9,-27,2.5f,14,pink);
 DrawEllipse(0,19,18,21,cream);DrawEllipse(-11,36,10,5,cream);DrawEllipse(11,36,10,5,cream);
 DrawEllipse(0,0,25,21,cream);DrawEllipse(-16,7,5,2.5f,pink);DrawEllipse(16,7,5,2.5f,pink);
 DrawCircle(-8,1,2,ink);DrawCircle(8,1,2,ink);DrawCircle(0,6,1.5f,AppTheme.border);
 DrawLineBezier((Vector2){-3,10},(Vector2){0,8},1,ink);DrawLineBezier((Vector2){0,8},(Vector2){3,10},1,ink);
 BerryBow(0,21,.38f);rlPopMatrix();
}
static void BerryCake(float x,float y,float s){
 Color cream=ThemeHex(0xfffaf0),sponge=ThemeHex(0xf1cfad),jam=ColorLerp(ThemeHex(0xe998ac),AppTheme.border,.2f);
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(s,s,1);
 DrawEllipse(0,15,23,5,Fade(AppTheme.border,.12f));
 DrawRectangleRounded((Rectangle){-18,-4,36,19},.2f,8,sponge);
 DrawRectangle(-18,3,36,4,jam);DrawRectangle(-18,7,36,3,cream);
 DrawEllipse(0,-4,18,6,cream);
 for(int i=0;i<5;i++)DrawCircle(-14+i*7,-1,3,cream);
 BerryFruit(0,-12,7);DrawCircle(-11,-6,2,ThemeHex(0xffe1e9));DrawCircle(11,-6,2,ThemeHex(0xffe1e9));rlPopMatrix();
}
static void BerryLace(float x,float y,float width){
 Color lace=ColorLerp(surface,ThemeHex(0xfff9eb),.5f);lace.a=surface.a;
 DrawRectangle(x,y,width,5,lace);
 for(float px=x+7;px<x+width-6;px+=14){DrawCircle(px,y+4,7,lace);DrawCircleLines(px,y+5,3,Fade(AppTheme.border,.28f));}
 DrawLine(x,y,x+width,y,Fade(AppTheme.border,.28f));
}
static void BerryBackdrop(int width,int height){
 int p=ActiveBackgroundPreset();DrawRectangle(0,0,width,height,AppTheme.bg);
 if(p==0){
  Color check=Fade(AppTheme.border,.045f);for(int x=0;x<width;x+=48)DrawRectangle(x,0,24,height,check);for(int y=0;y<height;y+=48)DrawRectangle(0,y,width,24,check);
  for(int row=0,y=195;y<height;y+=144,row++)for(int x=35+(row%2)*95;x<width;x+=190){if((x/190+row)%3==0)BerryCake(x,y,.65f);else if((x/190+row)%3==1)BerryFruit(x,y,10);else BerryBow(x,y,.46f);}
 }else if(p==1){
  DrawRectangleGradientV(0,0,width,height,ColorLerp(surface,AppTheme.bg,.4f),AppTheme.bg);
  for(int x=-height;x<width;x+=180)DrawLineEx((Vector2){x,0},(Vector2){x+height*.6f,height},24,Fade(AppTheme.border,.07f));
  for(int y=200;y<height;y+=170){DrawRectangle(0,y,width,12,Fade(AppTheme.border,.1f));DrawLine(0,y+5,width,y+5,Fade(WHITE,.6f));BerryBow(width-75,y+6,1.2f);BerryCake(65,y+42,.85f);}
 }else if(p==2){
  DrawRectangleGradientV(0,0,width,height,ThemeHex(0xfffaf0),AppTheme.bg);
  for(int y=225;y<height;y+=130)for(int x=55+(y/130%2)*70;x<width;x+=160){
   DrawLine(x,y+8,x-6,y+34,Fade(ThemeHex(0x91ad7d),.45f));BerryFruit(x,y,18);
   for(int n=0;n<5;n++){float a=n*2*PI/5;DrawCircle(x+44+cosf(a)*6,y+25+sinf(a)*6,4,Fade(WHITE,.8f));}DrawCircle(x+44,y+25,3,ThemeHex(0xefc870));
  }
  if(width>500){BerryBunny(width-87,height-97,1.25f);BerryCake(width-165,height-59,1.1f);}
  for(int x=85;x<width-180;x+=320)BerryCake(x,height-55,.8f);
 }else{
  DrawRectangleGradientV(0,0,width,height,surface,ColorLerp(surface,AppTheme.bg,.35f));
  for(int y=190;y<height;y+=180){DrawRectangle(0,y,width,52,Fade(AppTheme.border,.075f));BerryLace(0,y,width);BerryLace(0,y+47,width);}
  for(int x=90;x<width;x+=260)for(int y=265;y<height;y+=240){for(int n=0;n<24;n++){float a=n*PI/12;DrawCircle(x+cosf(a)*44,y+sinf(a)*32,5,Fade(AppTheme.border,.09f));}BerryCake(x,y,.85f);BerryBow(x+65,y-14,.5f);}
 }
}
static void BerryCookie(Rectangle r,Color state){
 Color crust=ThemeHex(0xc99953),dough=ColorLerp(ThemeHex(0xffe4a0),state,.06f);crust.a=dough.a=state.a;
 DrawRectangleRounded((Rectangle){r.x,r.y+1,r.width,r.height-1},1,24,crust);
 DrawRectangleRounded((Rectangle){r.x+2,r.y+1,r.width-4,r.height-4},1,24,dough);
 if(r.width>16&&r.height>16)DrawRectangleRoundedLinesEx((Rectangle){r.x+5,r.y+4,r.width-10,r.height-9},1,24,1,Fade(crust,.38f));
}
