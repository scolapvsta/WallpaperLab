#pragma once
// Original vector artwork: no reference photographs or Workshop assets are shipped.
static bool StrawberryTheme(void){return lab_option("ui_design")==1;}
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
 Color red=ColorLerp(ThemeHex(0xe87687),AppTheme.border,.25f),leaf=lab_option("light_theme")?ThemeHex(0x8caa81):ThemeHex(0x93aa87);
 // Tapered strawberry with a soft, rounded shoulder.
 for(int i=0;i<20;i++){float t=i/20.f;float half=s*(1-t)*sqrtf(t+.18f)*1.3f;DrawEllipse(x,y-s*.5f+i*s*.075f,half,s*.11f,red);}
 for(int row=0;row<3;row++)for(int col=0;col<3-row;col++)DrawEllipse(x+(col-(2-row)*.5f)*s*.48f,y-s*.18f+row*s*.39f,s*.055f,s*.09f,ThemeHex(0xffebc3));
 for(int n=-1;n<=1;n++)DrawTriangle((Vector2){x,y-s*.45f},(Vector2){x+n*s*.7f-s*.15f,y-s*.9f},(Vector2){x+n*s*.7f+s*.2f,y-s*.72f},leaf);
}
// A fluffy, full-bodied rabbit with a lop ear, glossy eyes, gingham bow and biscuit.
static void BunnyOval(float x,float y,float rx,float ry,Color fill,Color edge){DrawEllipse(x,y,rx+1,ry+1,edge);DrawEllipse(x,y,rx,ry,fill);}
static void BerryBunny(float x,float y,float s){
 Color cream=ThemeHex(0xfffbf7),pink=ThemeHex(0xf2b9cf),ink=ThemeHex(0x805b64),edge=ThemeHex(0xbba4aa);
 rlPushMatrix();rlTranslatef(x,y,0);rlScalef(s,s,1);
 // Body, tail and feet are behind the oversized head.
 BunnyOval(22,35,11,10,cream,edge);BunnyOval(0,31,21,24,cream,edge);
 BunnyOval(-14,48,12,8,cream,edge);BunnyOval(13,49,12,8,cream,edge);
 rlPushMatrix();rlTranslatef(12,-29,0);rlRotatef(22,0,0,1);BunnyOval(0,-7,10,25,cream,edge);DrawEllipse(0,-9,5,16,pink);rlPopMatrix();
 rlPushMatrix();rlTranslatef(-19,-19,0);rlRotatef(-51,0,0,1);BunnyOval(0,-5,11,23,cream,edge);DrawEllipse(0,-8,5,13,pink);rlPopMatrix();
 BunnyOval(0,0,29,25,cream,edge);
 // Small tufts soften the outline without covering the facial features.
 for(int n=0;n<7;n++){float a=(195+n*25)*DEG2RAD;DrawCircle(cosf(a)*27,sinf(a)*23,3.2f,cream);}
 for(int side=-1;side<=1;side+=2){
  DrawEllipse(side*19,10,7,3.5f,Fade(pink,.8f));BunnyOval(side*11,2,6,8,ink,ink);
  DrawCircle(side*11-2,-1,2.5f,WHITE);DrawCircle(side*11+2,5,1.2f,WHITE);
  DrawLineEx((Vector2){side*16,-6},(Vector2){side*20,-9},1.2f,ink);
  DrawLineEx((Vector2){side*19,10},(Vector2){side*21,7},.8f,ink);
 }
 DrawEllipse(0,11,2,1.5f,pink);DrawLineBezier((Vector2){-3,16},(Vector2){0,13},.9f,ink);DrawLineBezier((Vector2){0,13},(Vector2){3,16},.9f,ink);
 BerryBow(0,-25,.83f);
 // Gingham stitches on the oversized bow.
 for(int side=-1;side<=1;side+=2)for(int n=0;n<3;n++)DrawLine(side*(7+n*3),-32,side*(7+n*3),-21,Fade(WHITE,.65f));
 BunnyOval(-8,31,12,10,ThemeHex(0xf7dca0),ThemeHex(0xc6a16b));
 for(int n=0;n<4;n++)DrawCircle(-14+n*4,30+(n%2)*3,1.1f,ThemeHex(0xb78a58));
 BunnyOval(-20,34,7,10,cream,edge);BunnyOval(5,34,7,9,cream,edge);
 rlPopMatrix();
}
static void BerryLace(float x,float y,float width){
 Color lace=ColorLerp(surface,ThemeHex(0xfff9eb),.5f);lace.a=surface.a;
 DrawRectangle(x,y,width,5,lace);
 for(float px=x+7;px<x+width-6;px+=14){DrawCircle(px,y+4,7,lace);DrawCircleLines(px,y+5,3,Fade(AppTheme.border,.28f));}
 DrawLine(x,y,x+width,y,Fade(AppTheme.border,.28f));
}
static void BerryBackdrop(int width,int height){
 int p=lab_option("app_background_preset");DrawRectangle(0,0,width,height,AppTheme.bg);
 if(p==0){
  Color check=Fade(AppTheme.border,.045f);for(int x=0;x<width;x+=48)DrawRectangle(x,0,24,height,check);for(int y=0;y<height;y+=48)DrawRectangle(0,y,width,24,check);
  for(int row=0,y=195;y<height;y+=144,row++)for(int x=35+(row%2)*95;x<width;x+=190){if((x/190+row)%2)BerryFruit(x,y,10);else BerryBow(x,y,.46f);}
 }else if(p==1){
  DrawRectangleGradientV(0,0,width,height,ColorLerp(surface,AppTheme.bg,.4f),AppTheme.bg);
  for(int x=-height;x<width;x+=180)DrawLineEx((Vector2){x,0},(Vector2){x+height*.6f,height},24,Fade(AppTheme.border,.07f));
  for(int y=200;y<height;y+=170){DrawRectangle(0,y,width,12,Fade(AppTheme.border,.1f));DrawLine(0,y+5,width,y+5,Fade(WHITE,.6f));BerryBow(width-75,y+6,1.2f);}
 }else if(p==2){
  DrawRectangleGradientV(0,0,width,height,ThemeHex(0xfffaf0),AppTheme.bg);
  for(int y=225;y<height;y+=130)for(int x=55+(y/130%2)*70;x<width;x+=160){
   DrawLine(x,y+8,x-6,y+34,Fade(ThemeHex(0x91ad7d),.45f));BerryFruit(x,y,18);
   for(int n=0;n<5;n++){float a=n*2*PI/5;DrawCircle(x+44+cosf(a)*6,y+25+sinf(a)*6,4,Fade(WHITE,.8f));}DrawCircle(x+44,y+25,3,ThemeHex(0xefc870));
  }
  if(width>500)BerryBunny(width-100,height-112,1.8f);
 }else{
  DrawRectangleGradientV(0,0,width,height,surface,ColorLerp(surface,AppTheme.bg,.35f));
  for(int y=190;y<height;y+=180){DrawRectangle(0,y,width,52,Fade(AppTheme.border,.075f));BerryLace(0,y,width);BerryLace(0,y+47,width);}
  for(int x=90;x<width;x+=260)for(int y=265;y<height;y+=240){for(int n=0;n<24;n++){float a=n*PI/12;DrawCircle(x+cosf(a)*44,y+sinf(a)*32,5,Fade(AppTheme.border,.09f));}BerryBow(x,y,.9f);}
 }
}
static void BerryCookie(Rectangle r,Color state){
 Color crust=ThemeHex(0xc99953),dough=ColorLerp(ThemeHex(0xffe4a0),state,.06f);crust.a=dough.a=state.a;
 DrawRectangleRounded((Rectangle){r.x,r.y+1,r.width,r.height-1},1,24,crust);
 DrawRectangleRounded((Rectangle){r.x+2,r.y+1,r.width-4,r.height-4},1,24,dough);
 if(r.width>16&&r.height>16)DrawRectangleRoundedLinesEx((Rectangle){r.x+5,r.y+4,r.width-10,r.height-9},1,24,1,Fade(crust,.38f));
}
