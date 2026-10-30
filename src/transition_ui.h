#include "../vendor/engine/src/WallpaperEngine/Render/Helpers/TransitionShader.h"
static const char* transitionNames[]={"Nessuna","Dissolvenza incrociata","Dissolvenza al nero","Zoom avanti","Zoom indietro","Scorrimento","Tendina","Apertura dal puntatore","Chiusura e apertura dal puntatore"};
static double transitionStarted=0;static Shader transitionShader;static RenderTexture2D transitionCanvas;static Vector2 transitionOrigin;
static Rectangle switcherEye,transitionEye,switcherCustomize,transitionCustomize;
static void TransitionControls(int x,int* y,int w){
 const char* layouts[]={lab_tr("Griglia"),lab_tr("Fiume"),lab_tr("Verticale"),lab_tr("Onda")};int at=lab_option("carousel_layout"),v=Choice(20,x,y,w-94,lab_tr("Stile dello switcher"),layouts,4,at);if(v!=at)lab_set_option("carousel_layout",v);
 switcherCustomize=(Rectangle){x+w-82,*y-43,36,34};switcherEye=(Rectangle){x+w-38,*y-43,36,34};
 if(GlyphButton(switcherCustomize,UI_CUSTOM,false,true,"")){animationCustomize=1;animationOpened=GetTime();choiceOpen=-1;}
 if(GlyphButton(switcherEye,UI_EYE,false,!applying,"")){switcherPreview=true;animationOpened=GetTime();choiceOpen=-1;}
 const char* transitions[9];for(int n=0;n<9;n++)transitions[n]=lab_tr(transitionNames[n]);at=lab_option("transition_type");v=Choice(22,x,y,w-94,lab_tr("Transizione"),transitions,9,at);if(v!=at)lab_set_option("transition_type",v);
 transitionCustomize=(Rectangle){x+w-82,*y-43,36,34};transitionEye=(Rectangle){x+w-38,*y-43,36,34};
 if(GlyphButton(transitionCustomize,UI_CUSTOM,false,true,"")){animationCustomize=2;animationOpened=GetTime();choiceOpen=-1;}
 if(GlyphButton(transitionEye,UI_EYE,false,true,"")){transitionPreview=true;transitionStarted=GetTime();transitionOrigin=(Vector2){.5,.5};}
}
static void SwitcherPreview(void){
 if(!switcherPreview)return;bool modal=modalDrawing;modalDrawing=true;transitionDrawing=true;int sw=GetScreenWidth(),sh=GetScreenHeight();Rectangle box={(sw-720)/2.f,(sh-460)/2.f,720,460};DrawRectangle(0,0,sw,sh,Fade(BLACK,.65));ThemePanel(box,.06f,12,surface);UiText(lab_tr("Anteprima dello switcher"),box.x+20,box.y+18,20,AppTheme.text);
 if(GetTime()-animationOpened>.15&&((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box))||IsKeyPressed(KEY_ESCAPE)))switcherPreview=false;
 if(GlyphButton((Rectangle){box.x+670,box.y+12,34,34},UI_CLOSE,false,true,""))switcherPreview=false;
 DrawSwitcherDemo((Rectangle){box.x+20,box.y+60,680,360},GetTime()-animationOpened);modalDrawing=modal;transitionDrawing=false;
}
static void AnimationCustomization(void){
 if(!animationCustomize)return;bool modal=modalDrawing;modalDrawing=true;transitionDrawing=true;int sw=GetScreenWidth(),sh=GetScreenHeight(),layout=lab_option("carousel_layout"),type=lab_option("transition_type"),h=animationCustomize==1?420+(layout?43:0)+(layout==3?96:0):200+(type==5||type==6?43:type==3||type==4?48:0);Rectangle box={(sw-610)/2.f,(sh-h)/2.f,610,h};DrawRectangle(0,0,sw,sh,Fade(BLACK,.65));ThemePanel(box,.06f,12,surface);UiText(lab_tr(animationCustomize==1?"Personalizza switcher":"Personalizza transizione"),box.x+22,box.y+20,21,AppTheme.text);
 if(GetTime()-animationOpened>.15&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box)&&choiceOpen<0){animationCustomize=0;}
 if(GlyphButton((Rectangle){box.x+box.width-54,box.y+12,34,34},UI_CLOSE,false,true,"")){animationCustomize=0;choiceOpen=-1;}
 int x=box.x+24,y=box.y+66,w=box.width-48;const char* direction[]={lab_tr("Sinistra"),lab_tr("Destra"),lab_tr("Alto"),lab_tr("Basso")};
 if(animationCustomize==1){
 int design=lab_option("switcher_theme"),value=Choice(78,x,&y,w,"Theme",labDesignNames,LAB_DESIGN_COUNT,design);if(value!=design)lab_set_option("switcher_theme",value);
 const char* palettes[LAB_THEME_COUNT];int ids[LAB_THEME_COUNT],count=0,selected=0;
 for(int i=0;i<LAB_THEME_COUNT;i++)if(labThemes[i].collection==lab_option("switcher_theme")){ids[count]=i;palettes[count]=labThemes[i].name;if(i==lab_option("switcher_palette"))selected=count;count++;}
 value=Choice(79,x,&y,w,"Color palette",palettes,count,selected);if(value!=selected)lab_set_option("switcher_palette",ids[value]);
 SettingSlider(100,x,&y,w,lab_tr("Dimensione del carousel"),"switcher_size",70,140,"%");SettingSlider(101,x,&y,w,lab_tr("Ingrandimento della selezione"),"switcher_zoom",105,150,"%");SettingSlider(102,x,&y,w,lab_tr("Dissolvenza in entrata"),"switcher_fade_in",60,600," ms");SettingSlider(103,x,&y,w,lab_tr("Dissolvenza in uscita"),"switcher_fade_out",60,600," ms");SettingSlider(104,x,&y,w,lab_tr("Durata del movimento"),"switcher_motion",100,900," ms");
 if(lab_option("carousel_layout")!=0){int at=lab_option("switcher_reverse"),v=BoolChoice(75,x,&y,w,lab_tr("Inverti scorrimento"),at);if(v!=at)lab_set_option("switcher_reverse",v);}
 if(lab_option("carousel_layout")==3){SettingSlider(105,x,&y,w,lab_tr("Velocità dell’onda"),"wave_speed",0,300,"%");SettingSlider(106,x,&y,w,lab_tr("Ampiezza dell’onda"),"wave_height",0,40," px");}
 }else{
 SettingSlider(107,x,&y,w,lab_tr("Durata della transizione"),"transition_duration",100,2500," ms");const char* curves[]={lab_tr("Morbida"),lab_tr("Lineare"),lab_tr("Accelerazione e frenata")};int at=lab_option("transition_curve"),v=Choice(76,x,&y,w,lab_tr("Curva dell’animazione"),curves,3,at);if(v!=at)lab_set_option("transition_curve",v);
 if(lab_option("transition_type")==5||lab_option("transition_type")==6){at=lab_option("transition_direction");v=Choice(77,x,&y,w,lab_tr("Direzione"),direction,4,at);if(v!=at)lab_set_option("transition_direction",v);}
 if(lab_option("transition_type")==3||lab_option("transition_type")==4)SettingSlider(108,x,&y,w,lab_tr("Intensità dello zoom"),"transition_zoom",5,60,"%");
 }
 modalDrawing=modal;transitionDrawing=false;
}
static void TransitionPreview(void){
 if(!transitionPreview)return;bool wasModal=modalDrawing;modalDrawing=true;transitionDrawing=true;int sw=GetScreenWidth(),sh=GetScreenHeight();Rectangle box={(sw-720)/2.f,(sh-460)/2.f,720,460};DrawRectangle(0,0,sw,sh,Fade(BLACK,.65));ThemePanel(box,.06f,8,surface);UiText(lab_tr(transitionNames[lab_option("transition_type")]),box.x+20,box.y+18,20,AppTheme.text);
 if(IsKeyPressed(KEY_ESCAPE)||(GetTime()-transitionStarted>.15&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box))){transitionPreview=false;modalDrawing=wasModal;transitionDrawing=false;return;}
 if(GlyphButton((Rectangle){box.x+670,box.y+12,34,34},UI_CLOSE,false,true,""))transitionPreview=false;
 Rectangle image={box.x+20,box.y+60,680,360};if(CheckCollisionPointRec(GetMousePosition(),image)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){transitionOrigin=(Vector2){(GetMouseX()-image.x)/image.width,1-(GetMouseY()-image.y)/image.height};transitionStarted=GetTime();}
 int a=lab_carousel_count()>0?lab_carousel_item(0):0,b=lab_carousel_count()>1?lab_carousel_item(1):wallpaper_count>1?1:0;
 if(a>=0&&b>=0&&a<wallpaper_count&&b<wallpaper_count&&wallpapers[a].texture.id&&wallpapers[b].texture.id){
 if(!transitionCanvas.id){transitionCanvas=LoadRenderTexture(680,360);const char* vertex="#version 330 core\nin vec3 vertexPosition;in vec2 vertexTexCoord;out vec2 uv;uniform mat4 mvp;void main(){uv=vertexTexCoord;gl_Position=mvp*vec4(vertexPosition,1);}";transitionShader=LoadShaderFromMemory(vertex,LAB_TRANSITION_FRAGMENT);}
 BeginTextureMode(transitionCanvas);ClearBackground(BLACK);Cover(wallpapers[b].texture,(Rectangle){0,0,680,360},false);EndTextureMode();if(uiCanvasActive)rlEnableFramebuffer(uiCanvas.id);float duration=lab_option("transition_duration")/1000.f;float progress=fmod(GetTime()-transitionStarted,duration+1.2f)/duration;progress=Clamp(progress,0,1);int effect=lab_option("transition_type");Vector3 adjustments={0,1,1};SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"adjustments"),&adjustments,SHADER_UNIFORM_VEC3);Vector4 more={0,0,1,0};Vector3 geometry={0,1,1};float zoom=lab_option("transition_zoom")/100.f;int direction=lab_option("transition_direction"),curve=lab_option("transition_curve");SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"adjustMore"),&more,SHADER_UNIFORM_VEC4);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"geometry"),&geometry,SHADER_UNIFORM_VEC3);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"zoomAmount"),&zoom,SHADER_UNIFORM_FLOAT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"direction"),&direction,SHADER_UNIFORM_INT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"curve"),&curve,SHADER_UNIFORM_INT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"progress"),&progress,SHADER_UNIFORM_FLOAT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"effect"),&effect,SHADER_UNIFORM_INT);SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"origin"),&transitionOrigin,SHADER_UNIFORM_VEC2);Vector2 aspect={680.f/360,1};SetShaderValue(transitionShader,GetShaderLocation(transitionShader,"aspect"),&aspect,SHADER_UNIFORM_VEC2);BeginShaderMode(transitionShader);SetShaderValueTexture(transitionShader,GetShaderLocation(transitionShader,"previousImage"),wallpapers[a].texture);SetShaderValueTexture(transitionShader,GetShaderLocation(transitionShader,"currentImage"),transitionCanvas.texture);DrawTexturePro(transitionCanvas.texture,(Rectangle){0,0,680,-360},image,(Vector2){0},0,WHITE);EndShaderMode();
 }else UiText(lab_tr("Aggiungi due sfondi per vedere la transizione"),image.x+24,image.y+100,18,muted);
 modalDrawing=wasModal;transitionDrawing=false;
}
