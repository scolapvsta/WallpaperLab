#pragma once
static bool DefaultTheme(void){return lab_option("ui_design")==0;}
static bool PixelTheme(void){return lab_option("ui_design")==3;}
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
 int p=lab_option("app_background_preset");DrawRectangleGradientV(0,0,w,h,AppTheme.bg,ColorLerp(AppTheme.bg,AppTheme.border,.045f));
 if(p==1)for(int x=0;x<w;x+=48)for(int y=0;y<h;y+=48){DrawLine(x,y,x+8,y,Fade(AppTheme.border,.1f));DrawLine(x,y,x,y+8,Fade(AppTheme.border,.1f));}
 if(p==2)for(int i=0;i<12;i++)DrawEllipseLines(w*.8f,h*.68f,100+i*30,45+i*22,Fade(AppTheme.border,.055f));
 if(p==3)for(int x=18;x<w;x+=36)for(int y=18;y<h;y+=36)DrawCircle(x,y,1,Fade(AppTheme.border,.1f));
}
static void PixelBackdrop(int w,int h){
 int p=lab_option("app_background_preset");float t=lab_option("reduce_motion")?0:GetTime();Color dim=ColorLerp(AppTheme.bg,AppTheme.border,.10f);int unit=fmaxf(4,w/240);
 DrawRectangleGradientV(0,0,w,h,AppTheme.bg,dim);
 if(p==0){
  for(int i=0;i<8;i++){int x=(int)(i*w/7.f+t*5)%(w+100)-50,y=h*.24f+(i%3)*27;DrawRectangle(x,y,12*unit,3*unit,Fade(AppTheme.text,.12f));DrawRectangle(x+3*unit,y-2*unit,6*unit,2*unit,Fade(AppTheme.text,.12f));}
  for(int i=0;i<16;i++){int x=i*w/15,top=h*.67f+(i%4)*unit*4;DrawRectangle(x,top,w/15+2,h-top,ColorLerp(AppTheme.bg,AppTheme.border,.16f));DrawRectangle(x,top,w/15+2,unit,Fade(AppTheme.border,.4f));}
 }else if(p==1){
  for(int y=0;y<h;y+=32)for(int x=-16*((y/32)%2);x<w;x+=64)DrawRectangleLines(x,y,62,30,Fade(AppTheme.border,.08f));
  for(int x=40;x<w;x+=220){DrawRectangle(x,h*.48f,8,30,dim);DrawRectangle(x-3,h*.48f-10,14,14,Fade(ThemeHex(0xffb95c),.5f+.1f*sinf(t*4+x)));}
 }else if(p==2){
  for(int i=0;i<6;i++){Rectangle screen={w*(i+.12f)/6,h*.45f,w/7.f,h*.37f};DrawRectangleRec(screen,Fade(BLACK,.25f));DrawRectangleLinesEx(screen,3,Fade(AppTheme.border,.22f));DrawRectangle(screen.x+8,screen.y+8,screen.width-16,screen.height*.5f,dim);DrawRectangle(screen.x+8,screen.y+screen.height-20,8,8,AppTheme.border);}
 }else{
  for(int i=0;i<95;i++){int x=(i*149+37)%w,y=(int)(i*61+t*6)%h;DrawRectangle(x,y,i%9?2:4,i%9?2:4,Fade(AppTheme.text,.12f+(i%4)*.07f));}
  DrawCircle(w*.78f,h*.4f,h*.11f,dim);DrawRectangle(w*.67f,h*.4f,w*.22f,4,Fade(AppTheme.border,.18f));
 }
 for(int i=0;i<5;i++){
  float x=w*(i+.35f)/5,y=p==3?h*(.3f+(i%3)*.2f):h*.78f+sinf(t*2+i)*5;
  PixelCharacter(x,y,fmaxf(1,unit*.75f),i,(int)(t*4));
  DrawRectangle(x+4,y+unit*12,unit*9,unit,Fade(BLACK,.25f));
 }
 if(!lab_option("reduce_motion")&&fmodf(t,4)<.08f)DrawRectangle(w*.1f,h*.28f,w*.2f,2,Fade(AppTheme.border,.3f));
}
