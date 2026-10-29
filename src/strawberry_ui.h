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
static void BerryBunny(float x,float y,float s){
 Color cream=lab_option("light_theme")?ThemeHex(0xfffaf0):ThemeHex(0xe8cbd1),pink=ThemeHex(0xefaabd),ink=ThemeHex(0x724353);
 for(int side=-1;side<=1;side+=2){DrawEllipse(x+side*9*s,y-24*s,7*s,22*s,cream);DrawEllipse(x+side*9*s,y-25*s,3*s,15*s,pink);}
 DrawEllipse(x,y,24*s,21*s,cream);
 for(int side=-1;side<=1;side+=2){DrawEllipse(x+side*15*s,y+6*s,5*s,2.7f*s,pink);DrawEllipse(x+side*8*s,y-1*s,1.7f*s,2.7f*s,ink);}
 DrawCircle(x,y+6*s,2*s,pink);DrawLineBezier((Vector2){x-5*s,y+10*s},(Vector2){x,y+7*s},s,ink);DrawLineBezier((Vector2){x,y+7*s},(Vector2){x+5*s,y+10*s},s,ink);
 BerryBow(x+17*s,y-17*s,.45f*s);
}
static void BerryLace(float x,float y,float width){
 Color lace=ColorLerp(surface,ThemeHex(0xfff9eb),.5f);lace.a=surface.a;
 DrawRectangle(x,y,width,5,lace);
 for(float px=x+7;px<x+width-6;px+=14){DrawCircle(px,y+4,7,lace);DrawCircleLines(px,y+5,3,Fade(AppTheme.border,.28f));}
 DrawLine(x,y,x+width,y,Fade(AppTheme.border,.28f));
}
static void BerryBackdrop(int width,int height){
 DrawRectangle(0,0,width,height,AppTheme.bg);
 Color check=Fade(AppTheme.border,lab_option("light_theme")?.045f:.065f);
 for(int x=0;x<width;x+=48)DrawRectangle(x,0,24,height,check);
 for(int y=0;y<height;y+=48)DrawRectangle(0,y,width,24,check);
 for(int row=0,y=195;y<height;y+=144,row++)for(int x=35+(row%2)*95;x<width;x+=190){
  if((x/190+row)%3==0)BerryBow(x,y,.46f);else if((x/190+row)%3==1)BerryFruit(x,y,10);
  else {DrawCircle(x,y,2,Fade(AppTheme.border,.23f));DrawCircle(x+12,y+14,1,Fade(AppTheme.border,.2f));}
 }
}
static void BerryCookie(Rectangle r,Color icing){
 float tooth=fminf(5,r.height*.12f);Color crust=ColorLerp(ThemeHex(0xc58a66),AppTheme.border,.18f);crust.a=icing.a;
 // Scallops stay inside the existing hit rectangle, including small icon buttons.
 DrawRectangleRounded(r,.28f,12,crust);
 Rectangle body={r.x+tooth,r.y+tooth,r.width-2*tooth,r.height-2*tooth};DrawRectangleRounded(body,.3f,12,icing);
 int nx=fmaxf(2,roundf(body.width/(tooth*1.65f))),ny=fmaxf(2,roundf(body.height/(tooth*1.65f)));
 for(int i=0;i<=nx;i++){float x=body.x+body.width*i/nx;DrawCircle(x,body.y,tooth,icing);DrawCircle(x,body.y+body.height,tooth,icing);}
 for(int i=0;i<=ny;i++){float y=body.y+body.height*i/ny;DrawCircle(body.x,y,tooth,icing);DrawCircle(body.x+body.width,y,tooth,icing);}
 Rectangle stitch={r.x+7,r.y+6,r.width-14,r.height-12};
 if(stitch.width>8&&stitch.height>6){
  for(float x=stitch.x+3;x<stitch.x+stitch.width-2;x+=7){DrawLine(x,stitch.y,x+2,stitch.y,Fade(crust,.65f));DrawLine(x,stitch.y+stitch.height,x+2,stitch.y+stitch.height,Fade(crust,.65f));}
 }
}
