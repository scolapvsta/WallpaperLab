#pragma once
// Theme effects are local to the app; compositor and desktop styling stay untouched.
static Shader backdropShader;
static int backdropResolution,backdropTime,backdropStyle,backdropBase,backdropAccent;
static float themeRoundness=1;
static int themeId=0;
static const char* themeNames[]={"Standard","Standard","Neon ciano","Glass","Neon rosa","Pastello","Viola profondo","LED","RGB","Oceano","Ambra"};
// Muted surfaces and a single accent, inspired by Catppuccin, Rosé Pine and Dracula.
// Transparency is applied once to the UI canvas, independently of these palettes.
static Color ThemeHex(unsigned int rgb){return (Color){rgb>>16,(rgb>>8)&255,rgb&255,255};}
static void ApplyTheme(void){
 themeId=lab_option("ui_theme");if(themeId<0||themeId>10)themeId=0;bool light=lab_option("light_theme");
 static const unsigned int accents[]={0x99d8c3,0x99d8c3,0x8be9fd,0x9ccfd8,0xff79c6,0xcba6f7,0xbb9af7,0xa6e3a1,0x89b4fa,0x7dcfff,0xf6c177};
 static const unsigned int lightAccents[]={0x267762,0x267762,0x007d8e,0x286983,0xb13b79,0x8839ef,0x6846a5,0x426e35,0x2861b5,0x136e96,0x91621d};
 unsigned int bases[]={0x101216,0x101216,0x171c29,0x1b2431,0x211b2c,0x1e1e2e,0x1a1b26,0x171f1c,0x181a26,0x15232f,0x251f20};
 unsigned int surfaces[]={0x191c21,0x191c21,0x212736,0x252f3e,0x2c2439,0x262638,0x242638,0x222d27,0x242637,0x202f3d,0x30282a};
 unsigned int lightBases[]={0xf0f2f5,0xf0f2f5,0xecf3f6,0xeef3f6,0xf9eff5,0xeff1f5,0xefedf7,0xeff4ed,0xeff1f7,0xedf4f7,0xf8f2e9};
 unsigned int lightSurfaces[]={0xffffff,0xffffff,0xf8fcfd,0xfafcfe,0xfffafd,0xe6e9ef,0xf9f8ff,0xfafff8,0xfafbff,0xf8fdff,0xfffcf6};
 AppTheme.bg=ThemeHex(light?lightBases[themeId]:bases[themeId]);surface=ThemeHex(light?lightSurfaces[themeId]:surfaces[themeId]);
 AppTheme.text=ThemeHex(light?0x343748:themeId==5?0xcdd6f4:0xe7e9f1);muted=ThemeHex(light?0x626779:0xa4aec3);
 AppTheme.border=ThemeHex(light?lightAccents[themeId]:accents[themeId]);
 if(themeId==8&&!lab_option("reduce_motion")){Color cycle=ColorFromHSV(IsWindowReady()?fmodf(GetTime()*8,360):215,.42f,light?.63f:.96f);AppTheme.border=cycle;}
 control=ColorLerp(surface,AppTheme.text,light?.055f:.06f);controlHover=ColorLerp(surface,AppTheme.border,light?.13f:.14f);lineColor=ColorLerp(surface,AppTheme.text,light?.18f:.16f);
 themeRoundness=themeId==3?1.35f:themeId==5?1.5f:themeId==7?.8f:1;
 AppTheme.idle=control;AppTheme.hover=controlHover;
}
static void ThemePanel(Rectangle r,float roundness,int segments,Color color){
    float radius=Clamp(roundness*themeRoundness,0,1);
    if((themeId==2||themeId==4||themeId==7)&&r.width>28&&r.height>24)
        DrawRectangleRoundedLinesEx((Rectangle){r.x-1,r.y-1,r.width+2,r.height+2},radius,segments,1,Fade(AppTheme.border,.07f));
    DrawRectangleRounded(r,radius,segments,color);
}
static void InitBackdrop(void){
    const char* fs="#version 330 core\nin vec2 fragTexCoord;out vec4 finalColor;uniform vec2 resolution;uniform float time,style;uniform vec3 base,accent;\n"
      "void main(){vec2 uv=gl_FragCoord.xy/resolution;float t=time*.07;vec2 a=vec2(.24+.12*sin(t),.72+.09*cos(t));vec2 b=vec2(.78+.1*cos(t*.8),.27+.12*sin(t));float glow=exp(-dot(uv-a,uv-a)*9.)*.025+exp(-dot(uv-b,uv-b)*13.)*.015;vec3 col=base+accent*glow;"
      "if(style==3.)col+=vec3(.006,.004,.009)*sin(uv.y*6.+uv.x*3.);if(style==7.)col+=accent*(.002*pow(.5+.5*sin(uv.y*resolution.y*.55-time*.6),8.));finalColor=vec4(col,1.);}";
    backdropShader=LoadShaderFromMemory(NULL,fs);backdropResolution=GetShaderLocation(backdropShader,"resolution");backdropTime=GetShaderLocation(backdropShader,"time");backdropStyle=GetShaderLocation(backdropShader,"style");backdropBase=GetShaderLocation(backdropShader,"base");backdropAccent=GetShaderLocation(backdropShader,"accent");
}
static void DrawBackdrop(int width,int height){
    if(themeId<2||themeId==5||!backdropShader.id){DrawRectangle(0,0,width,height,AppTheme.bg);return;}
    Vector2 resolution={width,height};float time=lab_option("reduce_motion")?0:GetTime(),style=themeId;float base[]={AppTheme.bg.r/255.f,AppTheme.bg.g/255.f,AppTheme.bg.b/255.f};float accent[]={AppTheme.border.r/255.f,AppTheme.border.g/255.f,AppTheme.border.b/255.f};
    SetShaderValue(backdropShader,backdropResolution,&resolution,SHADER_UNIFORM_VEC2);SetShaderValue(backdropShader,backdropTime,&time,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropStyle,&style,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropBase,base,SHADER_UNIFORM_VEC3);SetShaderValue(backdropShader,backdropAccent,accent,SHADER_UNIFORM_VEC3);BeginShaderMode(backdropShader);DrawRectangle(0,0,width,height,WHITE);EndShaderMode();
}
