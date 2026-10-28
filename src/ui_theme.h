#pragma once
// Theme effects are local to the app; compositor and desktop styling stay untouched.
static Shader backdropShader;
static int backdropResolution,backdropTime,backdropStyle,backdropBase,backdropAccent;
static float themeRoundness=1;
static int themeId=0;
static const char* themeNames[]={"Standard","Standard","Neon ciano","Glass","Neon rosa","Pastello","Viola profondo","LED","RGB","Oceano","Ambra"};
static const Color themeAccents[]={ {153,216,195,255},{38,119,98,255},{47,231,245,255},{163,206,248,255},{255,113,209,255},{139,105,159,255},{187,151,255,255},{153,246,99,255},{108,223,255,255},{84,197,220,255},{244,183,94,255} };
static void ApplyTheme(void){
    themeId=lab_option("ui_theme");if(themeId<0||themeId>10)themeId=0;
    bool light=lab_option("light_theme");
    AppTheme.bg=light?(Color){240,242,244,255}:(Color){14,16,19,255};
    AppTheme.text=light?(Color){30,34,40,255}:(Color){236,238,241,255};
    surface=light?(Color){252,253,254,255}:(Color){22,25,29,255};
    control=light?(Color){231,234,238,255}:(Color){32,36,42,255};
    controlHover=light?(Color){218,224,230,255}:(Color){43,49,56,255};
    muted=light?(Color){99,108,119,255}:(Color){151,161,173,255};
    lineColor=light?(Color){211,217,224,255}:(Color){49,55,63,255};
    AppTheme.border=themeAccents[light&&themeId==0?1:themeId];themeRoundness=1;
    if(themeId==2){AppTheme.bg=(Color){8,16,25,255};surface=(Color){12,26,37,245};control=(Color){19,39,51,255};lineColor=(Color){35,91,106,255};themeRoundness=.65f;}
    if(themeId==3){AppTheme.bg=(Color){16,26,40,255};surface=(Color){31,47,65,190};control=(Color){81,106,132,100};controlHover=(Color){112,142,175,150};lineColor=(Color){164,203,233,90};themeRoundness=1.6f;}
    if(themeId==4){AppTheme.bg=(Color){25,11,27,255};surface=(Color){40,18,43,242};control=(Color){59,28,61,255};controlHover=(Color){85,38,80,255};lineColor=(Color){117,52,104,255};themeRoundness=1.35f;}
    if(themeId==5){AppTheme.bg=(Color){235,228,224,255};surface=(Color){249,241,235,255};control=(Color){225,213,221,255};controlHover=(Color){210,197,214,255};lineColor=(Color){204,187,195,255};muted=(Color){106,91,111,255};AppTheme.text=(Color){61,49,66,255};themeRoundness=2;}
    if(themeId==6){AppTheme.bg=(Color){17,13,29,255};surface=(Color){30,23,48,250};control=(Color){47,35,69,255};controlHover=(Color){66,47,93,255};lineColor=(Color){80,60,109,255};themeRoundness=1.25f;}
    if(themeId==7){AppTheme.bg=(Color){9,17,13,255};surface=(Color){16,30,24,245};control=(Color){25,44,33,255};controlHover=(Color){39,62,44,255};lineColor=(Color){68,100,65,255};themeRoundness=.45f;}
    if(themeId==8){AppTheme.bg=(Color){12,13,22,255};surface=(Color){23,23,37,235};control=(Color){34,33,53,255};AppTheme.border=ColorFromHSV(IsWindowReady()?fmodf((lab_option("reduce_motion")?12:GetTime())*16,360):190,.55f,1);themeRoundness=.85f;}
    if(themeId==9){AppTheme.bg=(Color){10,24,36,255};surface=(Color){18,40,55,245};control=(Color){28,58,75,255};controlHover=(Color){42,77,94,255};lineColor=(Color){55,91,113,255};themeRoundness=1.35f;}
    if(themeId==10){AppTheme.bg=(Color){24,20,16,255};surface=(Color){38,31,24,250};control=(Color){58,46,32,255};controlHover=(Color){78,59,38,255};lineColor=(Color){99,77,47,255};themeRoundness=.85f;}
    if(light&&themeId>=2){
        Color tint=themeId==8?AppTheme.border:themeAccents[themeId];
        AppTheme.bg=ColorLerp((Color){240,243,247,255},tint,.11f);
        surface=ColorLerp((Color){253,254,255,255},tint,.05f);
        control=ColorLerp((Color){230,235,242,255},tint,.13f);
        controlHover=ColorLerp((Color){216,224,234,255},tint,.23f);
        lineColor=ColorLerp((Color){191,204,217,255},tint,.2f);
        AppTheme.text=(Color){28,35,48,255};muted=(Color){78,91,111,255};
        AppTheme.border=ColorLerp(tint,(Color){18,35,70,255},.48f);
        if(themeId==3){surface.a=220;control.a=195;}
    }else if(!light&&themeId==5){
        AppTheme.bg=(Color){30,27,36,255};surface=(Color){42,37,48,255};control=(Color){60,51,67,255};controlHover=(Color){78,66,85,255};lineColor=(Color){94,80,100,255};AppTheme.text=(Color){243,233,242,255};muted=(Color){188,171,189,255};AppTheme.border=(Color){204,174,211,255};
    }
    AppTheme.idle=control;AppTheme.hover=controlHover;
}
static void ThemePanel(Rectangle r,float roundness,int segments,Color color){
    float radius=Clamp(roundness*themeRoundness,0,1);
    if((themeId==2||themeId==4||themeId==7||themeId==8)&&r.width>28&&r.height>24){
        float pulse=themeId==7&&!lab_option("reduce_motion")?.8f+.2f*sinf(GetTime()*2):1;
        for(int n=3;n>0;n--)DrawRectangleRoundedLinesEx((Rectangle){r.x-n,r.y-n,r.width+2*n,r.height+2*n},radius,segments,1,Fade(AppTheme.border,.035f*pulse));
    }
    DrawRectangleRounded(r,radius,segments,color);
}
static void InitBackdrop(void){
    const char* fs="#version 330 core\nin vec2 fragTexCoord;out vec4 finalColor;uniform vec2 resolution;uniform float time,style;uniform vec3 base,accent;\n"
      "void main(){vec2 uv=gl_FragCoord.xy/resolution;float t=time*.07;vec2 a=vec2(.24+.12*sin(t),.72+.09*cos(t));vec2 b=vec2(.78+.1*cos(t*.8),.27+.12*sin(t));float glow=exp(-dot(uv-a,uv-a)*9.)*.2+exp(-dot(uv-b,uv-b)*13.)*.13;vec3 col=base+accent*glow;"
      "if(style==3.)col+=vec3(.04,.025,.065)*sin(uv.y*6.+uv.x*3.);if(style==7.)col+=accent*(.018*pow(.5+.5*sin(uv.y*resolution.y*.55-time*.6),8.));finalColor=vec4(col,1.);}";
    backdropShader=LoadShaderFromMemory(NULL,fs);backdropResolution=GetShaderLocation(backdropShader,"resolution");backdropTime=GetShaderLocation(backdropShader,"time");backdropStyle=GetShaderLocation(backdropShader,"style");backdropBase=GetShaderLocation(backdropShader,"base");backdropAccent=GetShaderLocation(backdropShader,"accent");
}
static void DrawBackdrop(int width,int height){
    if(themeId<2||themeId==5||!backdropShader.id){DrawRectangle(0,0,width,height,AppTheme.bg);return;}
    Vector2 resolution={width,height};float time=lab_option("reduce_motion")?0:GetTime(),style=themeId;float base[]={AppTheme.bg.r/255.f,AppTheme.bg.g/255.f,AppTheme.bg.b/255.f};float accent[]={AppTheme.border.r/255.f,AppTheme.border.g/255.f,AppTheme.border.b/255.f};
    SetShaderValue(backdropShader,backdropResolution,&resolution,SHADER_UNIFORM_VEC2);SetShaderValue(backdropShader,backdropTime,&time,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropStyle,&style,SHADER_UNIFORM_FLOAT);SetShaderValue(backdropShader,backdropBase,base,SHADER_UNIFORM_VEC3);SetShaderValue(backdropShader,backdropAccent,accent,SHADER_UNIFORM_VEC3);BeginShaderMode(backdropShader);DrawRectangle(0,0,width,height,WHITE);EndShaderMode();
}
