#include "../vendor/engine/src/WallpaperEngine/Render/Helpers/TransitionShader.h"
static const char* transitionNames[]={"None","Cross fade","Fade out / in","Zoom in","Zoom out","Slide","Wipe","Iris from pointer","Close / open at pointer"};
static bool transitionPreview=false;static double transitionStarted=0;static Shader transitionShader;static RenderTexture2D transitionCanvas;static Vector2 transitionOrigin;
static void TransitionControls(int x,int* y,int w){
 const char* layouts[]={lab_tr("Griglia"),lab_tr("Fiume"),lab_tr("Verticale"),lab_tr("Onda")};int at=lab_option("carousel_layout"),v=Choice(20,x,y,w-48,lab_tr("Stile dello switcher"),layouts,4,at);if(v!=at)lab_set_option("carousel_layout",v);
 if(GlyphButton((Rectangle){x+w-38,*y-43,36,34},UI_EYE,false,!applying&&!lab_switcher_preview_active(),""))lab_preview_switcher();
 at=lab_option("transition_type");v=Choice(22,x,y,w-48,"Transition",transitionNames,9,at);if(v!=at)lab_set_option("transition_type",v);
 if(GlyphButton((Rectangle){x+w-38,*y-43,36,34},UI_EYE,false,true,"")){transitionPreview=true;transitionStarted=GetTime();transitionOrigin=(Vector2){.5,.5};}
}
static void TransitionPreview(void){
 if(!transitionPreview)return;bool wasModal=modalDrawing;modalDrawing=true;int sw=GetScreenWidth(),sh=GetScreenHeight();Rectangle box={(sw-720)/2.f,(sh-460)/2.f,720,460};DrawRectangle(0,0,sw,sh,Fade(BLACK,.65));ThemePanel(box,.06f,8,surface);UiText(transitionNames[lab_option("transition_type")],box.x+20,box.y+18,20,AppTheme.text);
 if(GlyphButton((Rectangle){box.x+670,box.y+12,34,34},UI_CLOSE,false,true,""))transitionPreview=false;
 Rectangle image={box.x+20,box.y+60,680,360};if(CheckCollisionPointRec(GetMousePosition(),image)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){transitionOrigin=(Vector2){(GetMouseX()-image.x)/image.width,1-(GetMouseY()-image.y)/image.height};transitionStarted=GetTime();}
 int a=lab_carousel_count()>0?lab_carousel_item(0):0,b=lab_carousel_count()>1?lab_carousel_item(1):wallpaper_count>1?1:0;
 if(a>=0&&b>=0&&a<wallpaper_count&&b<wallpaper_count&&wallpapers[a].texture.id&&wallpapers[b].texture.id){
 if(!transitionCanvas.id){transitionCanvas=LoadRenderTexture(680,360);const char* vertex="#version 330 core\nin vec3 vertexPosition;in vec2 vertexTexCoord;out vec2 uv;uniform mat4 mvp;void main(){uv=vec2(vertexTexCoord.x,1.-vertexTexCoord.y);gl_Position=mvp*vec4(vertexPosition,1);}";transitionShader=LoadShaderFromMemory(vertex,LAB_TRANSITION_FRAGMENT);}
 BeginTextureMode(transitionCanvas);ClearBackground(BLACK);Cover(wallpapers[b].texture,(Rectangle){0,0,680,360},false);EndTextureMode();float progress=fmod(GetTime()-transitionStarted,2)/.7;progress=Clamp(progress,0,1);int effect=lab_option("transition_type");SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"progress"),&progress,SHADER_UNIFORM_FLOAT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"effect"),&effect,SHADER_UNIFORM_INT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"origin"),&transitionOrigin,SHADER_UNIFORM_VEC2);Vector2 aspect={680.f/360,1};SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"aspect"),&aspect,SHADER_UNIFORM_VEC2);BeginShaderMode(transitionShader);SetShaderValueTexture(transitionShader,GetShaderLocation(transitionShader,"previousImage"),wallpapers[a].texture);SetShaderValueTexture(transitionShader,GetShaderLocation(transitionShader,"currentImage"),transitionCanvas.texture);DrawTexturePro(transitionCanvas.texture,(Rectangle){0,0,680,-360},image,(Vector2){0},0,WHITE);EndShaderMode();
 }else UiText("Add two wallpapers to preview transitions",image.x+24,image.y+100,18,muted);
 modalDrawing=wasModal;
}
