#include <time.h>
static void GeometryCenters(FILE*,const Rectangle*,int);
static int PageFooter(Rectangle,int,int,bool);
// Native login workspace. Catalog thumbnails are shared with the main library.
static int loginTab=0,loginWidget=2,loginCategory=0,loginPage=1;
static char loginSearch[256]="";static TextEdit loginSearchEdit;
static bool loginSearching=false,loginFiltersOpen=false;
static bool loginPaletteRequested=false,loginPresetEditing=false;static Rectangle loginPresetControls[4],loginPaletteButtons[2],loginSectionRows[8];
static bool loginSections[8]={true,true,true,false,false,false,false,true};
static Rectangle loginFilterBox,loginResetButtons[2],loginTextBoxes[24],loginColorButtons[24],loginInspectorArea;
static float loginScroll=0,loginInspectorScroll=0;
static Vector2 loginCoordinateSize;
static int loginDragging=-1;static Vector2 loginDragOffset;static Rectangle loginDragScreen;
static Rectangle loginInspectorTabs[2],loginModes[3],loginOutputTabs[32];
static Rectangle loginTabs[2],loginCards[200],loginWidgetRects[3],loginCanvasRect,loginPreviewButton,loginPublishButton,loginSearchBox;
static int loginCardIndices[200],loginCardCount,loginAvailableCount,loginUnsupportedCount;static Shader loginImageShader={0};
static const char* loginImageKeys[]={"brightness","contrast","saturation","temperature","hue","gamma","vignette","rotation","flip_x","flip_y","zoom","x","y","crop"};
typedef struct {Texture2D texture;unsigned revision;int width,height;long minute;} LoginOverlay;
static LoginOverlay loginOverlay[40];
static Texture2D loginStillTextures[2048];static char loginStillPaths[2048][512];
static void LoginShutdown(void){for(int i=0;i<40;i++)if(loginOverlay[i].texture.id)UnloadTexture(loginOverlay[i].texture);if(loginImageShader.id)UnloadShader(loginImageShader);for(int i=0;i<2048;i++)if(loginStillTextures[i].id)UnloadTexture(loginStillTextures[i]);}
static Texture2D LoginWallpaperTexture(int selected){
 if(selected<0||selected>=wallpaper_count)return (Texture2D){0};const char* path=lab_login_still(selected);
 if(path&&*path&&strcmp(loginStillPaths[selected],path)){Image image=LoadImage(path);if(image.data){if(loginStillTextures[selected].id)UnloadTexture(loginStillTextures[selected]);loginStillTextures[selected]=LoadTextureFromImage(image);UnloadImage(image);SetTextureFilter(loginStillTextures[selected],TEXTURE_FILTER_BILINEAR);snprintf(loginStillPaths[selected],512,"%s",path);}}
 if(loginStillTextures[selected].id)return loginStillTextures[selected];return atomic_load(&wallpapers[selected].loaded)?wallpapers[selected].texture:(Texture2D){0};
}
static const char* LoginWidgetPath(int i,const char* key){static char path[96];const char* names[]={"clock","date","login"};snprintf(path,sizeof(path),"/widgets/%s/%s",names[i],key);return path;}
static void LoginEditedPreview(Texture2D texture,Rectangle r,const float* values){
 DrawRectangleRec(r,BLACK);if(!texture.id)return;
 if(!loginImageShader.id)loginImageShader=LoadShader(NULL,lab_login_shader());
 Vector2 size={texture.width,texture.height},output={r.width,r.height};Vector3 basic={values[0],values[1],values[2]},geometry={values[7]*DEG2RAD,values[8]>.5?-1:1,values[9]>.5?-1:1},placement={values[11],values[12],values[10]};Vector4 more={values[4]*DEG2RAD,values[3],values[5],values[6]};float crop=values[13];
 SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"imageSize"),&size,SHADER_UNIFORM_VEC2);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"outputSize"),&output,SHADER_UNIFORM_VEC2);
 SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"basic"),&basic,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"more"),&more,SHADER_UNIFORM_VEC4);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"geometry"),&geometry,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"placement"),&placement,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"crop"),&crop,SHADER_UNIFORM_FLOAT);
 BeginShaderMode(loginImageShader);DrawTexturePro(texture,(Rectangle){0,0,texture.width,texture.height},r,(Vector2){0},0,WHITE);EndShaderMode();
}

static void LoginOverlayDraw(Rectangle r,int preset,int slot){
 LoginOverlay* cache=&loginOverlay[slot];unsigned rev=(preset<0?lab_login_revision():0)^((lab_option("language")+1)*2654435761u);int width=fmaxf(1,fminf(1280,r.width*1.5f)),height=fmaxf(1,width*r.height/r.width);if(height>1440){width=1440*r.width/r.height;height=1440;}long minute=time(NULL)/(lab_login_number("/widgets/clock/seconds")?1:60);
 if(!cache->texture.id||cache->revision!=rev||cache->width!=width||cache->height!=height||cache->minute!=minute){
  const unsigned char* pixels=lab_login_pixels(preset,width,height);if(pixels){Image image={(void*)pixels,width,height,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};if(cache->texture.id)UnloadTexture(cache->texture);cache->texture=LoadTextureFromImage(image);SetTextureFilter(cache->texture,TEXTURE_FILTER_BILINEAR);cache->revision=rev;cache->width=width;cache->height=height;cache->minute=minute;}
 }
 if(cache->texture.id){rlDrawRenderBatchActive();BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);DrawTexturePro(cache->texture,(Rectangle){0,0,width,height},r,(Vector2){0},0,WHITE);EndBlendMode();BeginBlendMode(BLEND_CUSTOM_SEPARATE);}
}
static int LoginOutputIndex(const char* key){char name[256];snprintf(name,sizeof(name),"%s",lab_login_text(key));for(int i=0;i<lab_monitor_count();i++)if(!strcmp(name,lab_monitor_name(i)))return i;return 0;}
static bool LoginUiOn(int i){char mode[32];snprintf(mode,sizeof(mode),"%s",lab_login_text("/ui_outputs"));int main=LoginOutputIndex("/ui_primary");return !strcmp(mode,"all")||(!strcmp(mode,"secondary")?(lab_monitor_count()<2||i!=main):i==main);}
static Rectangle LoginCanvas(Rectangle area,bool edit){
 int count=fminf(lab_monitor_count(),16);float x[16]={0},y[16]={0},w[16]={1920},h[16]={1080},left=1e9,top=1e9,right=-1e9,bottom=-1e9;
 if(area.width<1||area.height<1)return (Rectangle){0};if(!count)count=1;else for(int n=0;n<count;n++)lab_login_display_rect(n,&x[n],&y[n],&w[n],&h[n]);
 for(int n=0;n<count;n++){left=fminf(left,x[n]);top=fminf(top,y[n]);right=fmaxf(right,x[n]+w[n]);bottom=fmaxf(bottom,y[n]+h[n]);}
 float scale=fminf(area.width/(right-left),area.height/(bottom-top));Rectangle canvas={area.x+(area.width-(right-left)*scale)/2,area.y+(area.height-(bottom-top)*scale)/2,(right-left)*scale,(bottom-top)*scale};loginCanvasRect=canvas;
 int selected=lab_login_selected();Texture2D wallpaper=LoginWallpaperTexture(selected);
 float image[14];for(int i=0;i<14;i++)image[i]=lab_login_image(loginImageKeys[i]);bool span=!strcmp(lab_login_text("/mode"),"span"),single=!strcmp(lab_login_text("/mode"),"single");
 DrawRectangleRec(canvas,BLACK);if(span)LoginEditedPreview(wallpaper,canvas,image);memset(loginWidgetRects,0,sizeof(loginWidgetRects));
 for(int n=0;n<count;n++){
  Rectangle screen={canvas.x+(x[n]-left)*scale,canvas.y+(y[n]-top)*scale,w[n]*scale,h[n]*scale};
  if(!span){if(single){int target=lab_login_output_selected(n);Texture2D texture=LoginWallpaperTexture(target);float values[14];for(int k=0;k<14;k++)values[k]=lab_login_output_image(n,loginImageKeys[k]);LoginEditedPreview(texture,screen,values);}else LoginEditedPreview(wallpaper,screen,image);}
  if(LoginUiOn(n)){
   if(edit&&lab_login_number("/grid/visible")){float step=lab_login_number("/grid/step");for(float v=step;v<.999;v+=step){DrawLineEx((Vector2){screen.x+v*screen.width,screen.y},(Vector2){screen.x+v*screen.width,screen.y+screen.height},1,Fade(AppTheme.border,.22));DrawLineEx((Vector2){screen.x,screen.y+v*screen.height},(Vector2){screen.x+screen.width,screen.y+v*screen.height},1,Fade(AppTheme.border,.22));}}
   LoginOverlayDraw(screen,-1,5+n);
   if(edit)for(int widget=0;widget<3;widget++)if(lab_login_number(LoginWidgetPath(widget,"enabled"))){
    Rectangle box={screen.x+lab_login_number(LoginWidgetPath(widget,"x"))*screen.width,screen.y+lab_login_number(LoginWidgetPath(widget,"y"))*screen.height,lab_login_number(LoginWidgetPath(widget,"width"))*screen.width,lab_login_number(LoginWidgetPath(widget,"height"))*screen.height};
    loginWidgetRects[widget]=box;if(widget==loginWidget)DrawRectangleLinesEx(box,1.5,AppTheme.border);
    if(Hot(box,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginWidget=widget;loginDragging=widget;loginDragScreen=screen;loginDragOffset=(Vector2){GetMouseX()-box.x,GetMouseY()-box.y};}
   }
  }
  DrawRectangleLinesEx(screen,1,Fade(AppTheme.border,.65));
 }
 if(edit&&loginDragging>=0){if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){float px=Clamp((GetMouseX()-loginDragOffset.x-loginDragScreen.x)/loginDragScreen.width,0,1-lab_login_number(LoginWidgetPath(loginDragging,"width"))),py=Clamp((GetMouseY()-loginDragOffset.y-loginDragScreen.y)/loginDragScreen.height,0,1-lab_login_number(LoginWidgetPath(loginDragging,"height")));lab_login_move(loginDragging,px,py);}else{loginDragging=-1;lab_login_save();}}
 return canvas;
}
static void LoginNumber(int id,int x,int* y,int w,const char* label,const char* path,float lo,float hi,float scale,const char* suffix){float value=lab_login_number(path)*scale,old=value;NumberSlider(id,x,y,w,label,&value,lo*scale,hi*scale,suffix);if(value!=old)lab_login_set_number(path,value/scale);}
static int loginEditing=-1;static char loginFields[24][257];static TextEdit loginEdits[24];
static void LoginToggle(int id,int x,int* y,int w,const char* label,const char* path){bool old=lab_login_number(path),value=BoolChoice(id,x,y,w,label,old);if(value!=old)lab_login_set_number(path,value);}
static void LoginText(int id,int x,int* y,int w,const char* label,const char* path,bool color){
 UiText(label,x,*y,13,AppTheme.text);*y+=22;
 if(color){loginColorButtons[id]=(Rectangle){x,*y,w,36};unsigned c=strtoul(lab_login_text(path)+1,NULL,16);Color current={c>>16,(c>>8)&255,c&255,255},chosen=current;if(VisualColorPicker(2000+loginWidget*24+id,x,y,w,current,&chosen,&loginInspectorScroll)){lab_login_set_text(path,TextFormat("#%02x%02x%02x",chosen.r,chosen.g,chosen.b));}return;}
 Rectangle box={x,*y,w,36};loginTextBoxes[id]=box;char* value=loginFields[id];if(loginEditing!=id)snprintf(value,257,"%s",lab_login_text(path));
 bool focused=loginEditing==id,clicked=Hot(box,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
 if(clicked){loginEditing=id;loginSearching=false;EditFocus(&loginEdits[id],value,false);focused=true;}
 ThemePanel(box,.08f,6,focused?controlHover:control);DrawRectangleLinesEx(box,focused?2:1,focused?AppTheme.border:lineColor);if(Hot(box,true))SetMouseCursor(MOUSE_CURSOR_IBEAM);
 if(focused){if(EditInput(&loginEdits[id],value,257))lab_login_set_text(path,value);if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box)))loginEditing=-1;}
 if(*value||focused){EditDraw(&loginEdits[id],value,(Rectangle){x+10,*y+4,w-20,28},14,focused);BeginScissorMode(inputClip.x,inputClip.y,inputClip.width,inputClip.height);}else UiText(lab_tr("Scrivi un testo…"),x+10,*y+11,13,muted);*y+=48;
}
// The render target must be sampled outside BeginDrawing. Suggestions are explicit,
// so a later frame or wallpaper change cannot silently overwrite manual colors.
static void LoginPaletteUpdate(void){
 if(!loginPaletteRequested)return;int output=LoginOutputIndex("/ui_primary");if(!LoginUiOn(output))for(int n=0;n<lab_monitor_count();n++)if(LoginUiOn(n)){output=n;break;}
 bool single=!strcmp(lab_login_text("/mode"),"single"),span=!strcmp(lab_login_text("/mode"),"span");int item=single?lab_login_output_selected(output):lab_login_selected();Texture2D texture=LoginWallpaperTexture(item);if(!texture.id){loginPaletteRequested=false;return;}
 float values[14];for(int n=0;n<14;n++)values[n]=single?lab_login_output_image(output,loginImageKeys[n]):lab_login_image(loginImageKeys[n]);
 float ox=0,oy=0,ow=1920,oh=1080;lab_login_display_rect(output,&ox,&oy,&ow,&oh);float factor=64/fmaxf(ow,oh);int sw=fmaxf(1,roundf(ow*factor)),sh=fmaxf(1,roundf(oh*factor));Rectangle target={0,0,sw,sh};
 if(span&&lab_monitor_count()){float left=1e9,top=1e9,right=-1e9,bottom=-1e9;for(int n=0;n<lab_monitor_count();n++){float x,y,w,h;lab_login_display_rect(n,&x,&y,&w,&h);left=fminf(left,x);top=fminf(top,y);right=fmaxf(right,x+w);bottom=fmaxf(bottom,y+h);}target=(Rectangle){(left-ox)*sw/ow,(top-oy)*sh/oh,(right-left)*sw/ow,(bottom-top)*sh/oh};}
 RenderTexture2D sample=LoadRenderTexture(sw,sh);BeginTextureMode(sample);ClearBackground(BLACK);LoginEditedPreview(texture,target,values);EndTextureMode();Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8);lab_login_palette(image.data,sw,sh);UnloadImage(image);UnloadRenderTexture(sample);loginPaletteRequested=false;colorPickerOpen=-1;
}
static bool LoginSection(int id,int x,int* y,int w,const char* title){
 Rectangle row={x,*y,w,34};loginSectionRows[id]=row;ThemePanel(row,.08f,6,control);UiText(title,x+10,*y+10,13,AppTheme.text);UiText(loginSections[id]?"−":"+",x+w-22,*y+8,15,muted);
 if(Hot(row,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginSections[id]=!loginSections[id];loginEditing=-1;colorPickerOpen=-1;}*y+=44;return loginSections[id];
}
static int LoginPresets(Rectangle area,bool compact){
 static int selected[2]={-1,-1};static char names[2][97];static TextEdit edit[2];int full=!loginTab;
 if(choiceOpen<0&&Hot(choiceControls[111],true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){lab_login_preset_refresh(full);selected[full]=-1;for(int n=0;n<lab_login_preset_count(full);n++)if(!strcmp(names[full],lab_login_preset_name(full,n)))selected[full]=n;}
 int count=fminf(126,lab_login_preset_count(full));selected[full]=Clamp(selected[full],-1,count-1);
 UiText(lab_tr(full?"Preset completo":"Preset UI"),area.x,area.y+3,14,AppTheme.text);loginPresetControls[3]=(Rectangle){area.x+area.width-32,area.y-2,32,24};if(GlyphButton(loginPresetControls[3],UI_IMPORT,false,true,""))lab_login_preset_folder(full);
 int half=compact?area.width:(area.width-16)/2,x=area.x,y=area.y+28;int action=fminf(half*.46f,fmaxf(88,fmaxf(UiMeasure(lab_tr("Aggiorna"),14),UiMeasure(lab_tr("Carica"),14))+28));const char* options[128];options[0]=lab_tr("Seleziona un preset");for(int n=0;n<count;n++)options[n+1]=lab_login_preset_name(full,n);
 int next=Choice(111,x,&y,half-action-8,"",options,count+1,selected[full]+1)-1;if(next!=selected[full]){selected[full]=next;snprintf(names[full],97,"%s",next>=0?lab_login_preset_name(full,next):"");loginPresetEditing=false;}
 loginPresetControls[1]=(Rectangle){x+half-action,area.y+28,action,34};if(Button(loginPresetControls[1],lab_tr("Carica"),false,selected[full]>=0)){if(lab_login_preset_load(full,selected[full])){loginInspectorScroll=0;loginEditing=-1;colorPickerOpen=-1;loginPresetEditing=false;}}
 x=compact?area.x:area.x+half+16;y=area.y+(compact?70:28);Rectangle box={x,y,half-action-8,34};loginPresetControls[0]=box;
 if(Hot(box,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginPresetEditing=true;loginSearching=false;loginEditing=-1;colorPickerNumeric=-1;EditFocus(&edit[full],names[full],false);}
 ThemePanel(box,.08f,6,control);DrawRectangleLinesEx(box,loginPresetEditing?2:1,loginPresetEditing?AppTheme.border:lineColor);
 if(loginPresetEditing){EditInput(&edit[full],names[full],97);if(IsKeyPressed(KEY_ENTER)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box)))loginPresetEditing=false;}
 if(*names[full]||loginPresetEditing)EditDraw(&edit[full],names[full],(Rectangle){x+8,y+3,box.width-16,28},13,loginPresetEditing);else ClippedText(lab_tr("Nome del preset"),(Rectangle){x+8,y+8,box.width-16,22},13,muted);
 bool overwrite=false;for(int n=0;n<count;n++)if(!strcmp(names[full],lab_login_preset_name(full,n)))overwrite=true;
 loginPresetControls[2]=(Rectangle){x+half-action,y,action,34};if(Button(loginPresetControls[2],lab_tr(overwrite?"Aggiorna":"Salva"),false,*names[full]&&(!full||lab_login_selected()>=0))){if(lab_login_preset_save(full,names[full]))for(int n=0;n<lab_login_preset_count(full);n++)if(!strcmp(names[full],lab_login_preset_name(full,n)))selected[full]=n;loginPresetEditing=false;}
 return compact?114:72;
}
static void LoginPosition(int x,int* y,int w){
 float width=1920,height=1080;int output=LoginOutputIndex("/ui_primary");if(!LoginUiOn(output))for(int n=0;n<lab_monitor_count();n++)if(LoginUiOn(n)){output=n;break;}
 lab_login_display_pixels(output,&width,&height);loginCoordinateSize=(Vector2){width,height};
 ClippedText(TextFormat("%s · %.0f × %.0f px",lab_monitor_count()?lab_monitor_name(output):lab_tr("Display"),width,height),(Rectangle){x,*y,w,22},12,muted);*y+=28;
 float xmax=1-lab_login_number(LoginWidgetPath(loginWidget,"width")),ymax=1-lab_login_number(LoginWidgetPath(loginWidget,"height"));
 LoginNumber(151,x,y,w,"X",LoginWidgetPath(loginWidget,"x"),0,xmax,width,"px");LoginNumber(152,x,y,w,"Y",LoginWidgetPath(loginWidget,"y"),0,ymax,height,"px");
}
static void LoginFont(int x,int* y,int w,const char* path){
 const char* fonts[97];fonts[0]=lab_tr("Automatico");int count=fminf(lab_font_count(),96)+1,selected=0;char family[128];snprintf(family,128,"%s",lab_login_text(path));for(int i=1;i<count;i++){fonts[i]=lab_font_name(i-1);if(!strcmp(family,fonts[i]))selected=i;}
 int next=Choice(96,x,y,w,lab_tr("Font dell'interfaccia"),fonts,count,selected);if(next!=selected)lab_login_set_text(path,next?fonts[next]:"");
}
static void LoginOption(int id,int x,int* y,int w,const char* label,const char* path,const char** labels,const char** values,int count){int selected=0;for(int i=0;i<count;i++)if(!strcmp(lab_login_text(path),values[i]))selected=i;int next=Choice(id,x,y,w,label,labels,count,selected);if(next!=selected)lab_login_set_text(path,values[next]);}
static void LoginWallpaperInspector(int x,int* y,int w){
 int i=lab_login_selected();if(i<0){*y+=TextLines(lab_tr("Seleziona uno sfondo per vedere dettagli e azioni"),x,*y,w,14,3,muted);return;}
 UiText(lab_tr("Sfondo selezionato"),x,*y,15,muted);*y+=36;float h=Clamp(w*9.f/16,100,320);float values[14];for(int n=0;n<14;n++)values[n]=lab_login_image(loginImageKeys[n]);
 LoginEditedPreview(LoginWallpaperTexture(i),(Rectangle){x,*y,w,h},values);bool local=!strcmp(lab_origin(i),"local");const char* badge=lab_tr(loginStillTextures[i].id?"Fotogramma della scena":local?"Anteprima":"Miniatura del Workshop");ThemePanel((Rectangle){x+8,*y+8,UiMeasure(badge,12)+24,24},.3f,6,Fade(BLACK,.7));UiText(badge,x+20,*y+14,12,WHITE);*y+=h+18;*y+=TextLines(lab_title(i),x,*y,w,22,3,AppTheme.text)+12;UiText(TextFormat("%s · %s",TypeLabel(i),ResolutionLabel(i)),x,*y,14,muted);*y+=27;UiText(!strcmp(lab_origin(i),"local")?lab_tr("Locale"):"Steam",x,*y,12,AppTheme.border);*y+=28;
 if(InspectorSection(0,x,y,w,lab_tr("Display di destinazione"))){
  const char* modes[]={lab_tr("Monitor singolo"),lab_tr("Duplicato"),lab_tr("Panoramico")},*keys[]={"single","mirror","span"};LoginOption(110,x,y,w,lab_tr("Display"),"/mode",modes,keys,3);
  int variant=lab_login_variant_selected(i),count=fminf(94,lab_variant_count(i));const char* options[96];options[0]=lab_tr("Originale / automatica");for(int n=0;n<count;n++)options[n+1]=lab_variant_label(i,n);choiceEnabled=count>0;int next=Choice(70,x,y,w,lab_tr("Risoluzione dello sfondo"),options,count+1,variant+1);choiceEnabled=true;if(next!=variant+1)lab_login_variant_set(i,next-1);*y+=10;
 }
 wallpaperLogin=true;PlacementSync(i);if(InspectorSection(1,x,y,w,lab_tr("Immagine e ritaglio"))){*y+=ImageControls(i,x,*y,w);*y+=PlacementControls(i,x,*y,w);bool old=lab_login_image("crop")>.5,value=SwitchToggle((Rectangle){x,*y,w,40},lab_tr("Riempi lo schermo"),old);if(value!=old)lab_login_image_set("crop",value);*y+=52;}wallpaperLogin=false;
 if(InspectorSection(2,x,y,w,lab_tr("Riproduzione"))){LoginNumber(158,x,y,w,lab_tr("Limite FPS"),"/fps",15,120,1,"FPS");}
 if(InspectorSection(3,x,y,w,lab_tr("UI di login"))){const char* outputs[32];int count=fminf(lab_monitor_count(),32);for(int n=0;n<count;n++)outputs[n]=lab_monitor_name(n);if(count){int at=LoginOutputIndex("/ui_primary"),next=Choice(91,x,y,w,lab_tr("Monitor principale"),outputs,count,at);if(next!=at)lab_login_set_text("/ui_primary",outputs[next]);}const char* places[]={lab_tr("Principale"),lab_tr("Secondario"),lab_tr("Tutti")},*keys[]={"primary","secondary","all"};LoginOption(97,x,y,w,lab_tr("UI di login"),"/ui_outputs",places,keys,3);}
}
static void LoginInspector(Rectangle area){
 loginInspectorArea=area;ThemePanel(area,.025f,8,surface);Rectangle saved=inputClip;inputClip=area;
 if(!choiceWheel&&Hot(area,true))loginInspectorScroll=fmaxf(0,loginInspectorScroll-ScrollInput(area)*48);choiceWheel=false;
 int inset=loginTab?16:22,x=area.x+inset,w=area.width-2*inset;const char* kinds[]={lab_tr("Widget"),lab_tr("Aspetto")};int* category=&loginCategory;
 if(loginTab)for(int i=0;i<2;i++){loginInspectorTabs[i]=(Rectangle){x+i*(w+6)/2,area.y+12,(w-6)/2,34};if(Button(loginInspectorTabs[i],kinds[i],*category==i,true)){*category=i;loginInspectorScroll=0;choiceOpen=-1;loginEditing=-1;}}
 Rectangle content=area;if(loginTab){content.y+=58;content.height-=58;}
 if(loginTab){int y=content.y;const char* names[]={lab_tr("Orologio"),lab_tr("Data"),lab_tr("UI di login")};int next=Choice(92,x,&y,w,lab_tr("Widget"),names,3,loginWidget);if(next!=loginWidget){loginWidget=next;loginEditing=-1;loginInspectorScroll=0;}content.y+=43;content.height-=43;}
 inputClip=content;BeginScissorMode(content.x+1,content.y+1,content.width-2,content.height-2);int y=content.y+12-loginInspectorScroll;
 if(!loginTab)LoginWallpaperInspector(x,&y,w);
 else if(!loginCategory){
  if(loginWidget!=2)LoginToggle(93,x,&y,w,lab_tr("Attivo"),LoginWidgetPath(loginWidget,"enabled"));
  if(LoginSection(0,x,&y,w,lab_tr("Posizione e dimensioni"))){LoginPosition(x,&y,w);
  LoginNumber(153,x,&y,w,lab_tr("Larghezza"),LoginWidgetPath(loginWidget,"width"),.08,.8,100,"%");LoginNumber(154,x,&y,w,lab_tr("Altezza"),LoginWidgetPath(loginWidget,"height"),.03,.9,100,"%");
  const char* align[]={lab_tr("Sinistra"),lab_tr("Centro"),lab_tr("Destra")};for(int n=0;n<3;n++)if(Button((Rectangle){x+n*(w+6)/3,y,(w-12)/3,30},align[n],false,true)){float width=lab_login_number(LoginWidgetPath(loginWidget,"width"));lab_login_set_number(LoginWidgetPath(loginWidget,"x"),n==0?0:n==1?(1-width)/2:1-width);}y+=40;
  LoginNumber(155,x,&y,w,lab_tr("Dimensione"),LoginWidgetPath(loginWidget,"size"),10,140,1,"px");
  }
  if(LoginSection(1,x,&y,w,lab_tr("Contenuto"))){
  if(loginWidget==0){const char* formats[]={"24 h","12 h · AM/PM"},*codes[]={"24","12"};LoginOption(99,x,&y,w,lab_tr("Formato"),LoginWidgetPath(0,"format"),formats,codes,2);LoginToggle(100,x,&y,w,lab_tr("Secondi"),LoginWidgetPath(0,"seconds"));}
  if(loginWidget==1){const char* labels[]={lab_tr("Data estesa"),lab_tr("Data breve"),"ISO",lab_tr("Giorno della settimana")},*values[]={"long","short","iso","weekday"};LoginOption(98,x,&y,w,lab_tr("Formato"),LoginWidgetPath(1,"format"),labels,values,4);LoginToggle(100,x,&y,w,lab_tr("Maiuscolo"),LoginWidgetPath(1,"uppercase"));}
  if(loginWidget==2){LoginToggle(100,x,&y,w,lab_tr("Benvenuto"),LoginWidgetPath(2,"welcome_enabled"));LoginToggle(101,x,&y,w,lab_tr("Saluto in base all'ora"),LoginWidgetPath(2,"dynamic_welcome"));if(!lab_login_number(LoginWidgetPath(2,"dynamic_welcome")))LoginText(0,x,&y,w,lab_tr("Testo di benvenuto"),LoginWidgetPath(2,"welcome_text"),false);}
  }
  LoginNumber(160,x,&y,w,lab_tr("Passo della griglia"),"/grid/step",.01,.2,100,"%");
 }else{
  if(LoginSection(7,x,&y,w,lab_tr("Colori dallo sfondo"))){
   y+=TextLines(lab_tr("Propone solo i colori. Opacità, effetti e disposizione restano invariati."),x,y,w,12,3,muted)+10;
   loginPaletteButtons[0]=(Rectangle){x,y,w,34};if(Button(loginPaletteButtons[0],lab_tr("Suggerisci colori"),false,lab_login_selected()>=0)){loginPaletteRequested=true;loginEditing=-1;}
   y+=42;loginPaletteButtons[1]=(Rectangle){x,y,w,32};if(Button(loginPaletteButtons[1],lab_tr("Annulla colori suggeriti"),false,lab_login_palette_can_undo())){lab_login_palette_undo();colorPickerOpen=-1;}y+=44;
  }
  if(LoginSection(2,x,&y,w,lab_tr("Testo"))){
   LoginFont(x,&y,w,LoginWidgetPath(loginWidget,"font"));LoginToggle(103,x,&y,w,lab_tr("Grassetto"),LoginWidgetPath(loginWidget,"bold"));LoginToggle(104,x,&y,w,lab_tr("Corsivo"),LoginWidgetPath(loginWidget,"italic"));LoginToggle(105,x,&y,w,lab_tr("Sottolineato"),LoginWidgetPath(loginWidget,"underline"));
   const char* align[]={lab_tr("Sinistra"),lab_tr("Centro"),lab_tr("Destra")},*alignment[]={"left","center","right"};LoginOption(106,x,&y,w,lab_tr("Allineamento"),LoginWidgetPath(loginWidget,"align"),align,alignment,3);
   LoginNumber(162,x,&y,w,lab_tr("Spaziatura lettere"),LoginWidgetPath(loginWidget,"tracking"),-2,12,1,"px");LoginText(5,x,&y,w,lab_tr("Colore del testo"),LoginWidgetPath(loginWidget,"color"),true);
  }
  const char* effects[]={lab_tr("Nessuno"),lab_tr("Ombra"),lab_tr("Rilievo 3D")},*values[]={"none","shadow","extrude"};
  if(LoginSection(3,x,&y,w,lab_tr("Effetti del testo"))){
   LoginOption(107,x,&y,w,lab_tr("Effetto testo"),LoginWidgetPath(loginWidget,"effect"),effects,values,3);
   LoginNumber(161,x,&y,w,lab_tr("Profondità"),LoginWidgetPath(loginWidget,"effect_depth"),0,10,1,"px");LoginText(6,x,&y,w,lab_tr("Colore dell'effetto"),LoginWidgetPath(loginWidget,"effect_color"),true);
  }
  if(LoginSection(4,x,&y,w,lab_tr("Sfondo e bordi"))){
   LoginNumber(163,x,&y,w,lab_tr("Opacità del widget"),LoginWidgetPath(loginWidget,"opacity"),0,1,100,"%");LoginText(7,x,&y,w,lab_tr("Colore dello sfondo"),LoginWidgetPath(loginWidget,"background"),true);LoginNumber(164,x,&y,w,lab_tr("Opacità dello sfondo"),LoginWidgetPath(loginWidget,"background_opacity"),0,1,100,"%");LoginNumber(165,x,&y,w,lab_tr("Bordi arrotondati"),LoginWidgetPath(loginWidget,"radius"),0,80,1,"px");LoginNumber(166,x,&y,w,lab_tr("Spessore del bordo"),LoginWidgetPath(loginWidget,"border_width"),0,8,1,"px");LoginText(8,x,&y,w,lab_tr("Colore del bordo"),LoginWidgetPath(loginWidget,"border_color"),true);
   if(loginWidget==2)LoginNumber(170,x,&y,w,lab_tr("Margine interno"),LoginWidgetPath(2,"padding"),8,64,1,"px");
  }
  if(loginWidget==2){
   if(LoginSection(5,x,&y,w,lab_tr("Campi di accesso"))){
    LoginToggle(102,x,&y,w,lab_tr("Etichette dei campi"),LoginWidgetPath(2,"labels"));LoginText(1,x,&y,w,lab_tr("Utente"),LoginWidgetPath(2,"user_label"),false);LoginText(2,x,&y,w,lab_tr("Password"),LoginWidgetPath(2,"password_label"),false);LoginText(3,x,&y,w,lab_tr("Sessione"),LoginWidgetPath(2,"session_label"),false);
    LoginText(11,x,&y,w,lab_tr("Colore dei campi"),LoginWidgetPath(2,"field_color"),true);LoginNumber(167,x,&y,w,lab_tr("Opacità dei campi"),LoginWidgetPath(2,"field_opacity"),0,1,100,"%");LoginNumber(168,x,&y,w,lab_tr("Bordi dei campi"),LoginWidgetPath(2,"field_radius"),0,40,1,"px");LoginNumber(169,x,&y,w,lab_tr("Bordo del campo attivo"),LoginWidgetPath(2,"focus_width"),0,4,1,"px");LoginNumber(171,x,&y,w,lab_tr("Altezza dei campi"),LoginWidgetPath(2,"field_height"),.08,.16,100,"%");LoginNumber(173,x,&y,w,lab_tr("Dimensione etichette"),LoginWidgetPath(2,"label_size"),.45,1,100,"%");LoginNumber(174,x,&y,w,lab_tr("Animazione password"),LoginWidgetPath(2,"animation"),.05,.5,1000,"ms");LoginNumber(175,x,&y,w,lab_tr("Dimensione pallini"),LoginWidgetPath(2,"dot_size"),1,7,1,"px");
   }
   if(LoginSection(6,x,&y,w,lab_tr("Pulsante di accesso"))){
    LoginText(4,x,&y,w,lab_tr("Testo del pulsante"),LoginWidgetPath(2,"button_label"),false);LoginText(9,x,&y,w,lab_tr("Colore del pulsante"),LoginWidgetPath(2,"button_color"),true);LoginText(10,x,&y,w,lab_tr("Colore del testo"),LoginWidgetPath(2,"button_text_color"),true);
    LoginToggle(112,x,&y,w,lab_tr("Grassetto"),LoginWidgetPath(2,"button_bold"));LoginNumber(176,x,&y,w,lab_tr("Dimensione del testo"),LoginWidgetPath(2,"button_text_size"),.5,1.5,100,"%");LoginNumber(172,x,&y,w,lab_tr("Altezza del pulsante"),LoginWidgetPath(2,"button_height"),.08,.15,100,"%");
    LoginOption(113,x,&y,w,lab_tr("Effetto testo"),LoginWidgetPath(2,"button_effect"),effects,values,3);LoginNumber(177,x,&y,w,lab_tr("Profondità"),LoginWidgetPath(2,"button_effect_depth"),0,10,1,"px");LoginText(12,x,&y,w,lab_tr("Colore dell'effetto"),LoginWidgetPath(2,"button_effect_color"),true);
   }
  }
 }

 EndScissorMode();float extent=y+loginInspectorScroll-content.y+16;loginInspectorScroll=Clamp(loginInspectorScroll,0,fmaxf(0,extent-content.height));if(extent>content.height){float h=content.height*content.height/extent;DrawRectangle(content.x+content.width-5,content.y+(content.height-h)*loginInspectorScroll/(extent-content.height),3,h,Fade(AppTheme.border,.7));}inputClip=saved;
}
static void LoginGrid(Rectangle area){
 int x=area.x+16,y=area.y+8,w=area.width-32;loginFilterBox=(Rectangle){x,y,36,36};loginSearchBox=(Rectangle){x+44,y,w-88,36};ThemePanel(loginSearchBox,.1f,8,control);
 if(Hot(loginSearchBox,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginSearching=true;loginEditing=-1;EditFocus(&loginSearchEdit,loginSearch,false);}else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))loginSearching=false;
 if(loginSearching&&!inputBlocked){if(EditInput(&loginSearchEdit,loginSearch,sizeof(loginSearch)))loginPage=1;if(IsKeyPressed(KEY_ENTER))loginSearching=false;}
 if(*loginSearch||loginSearching)EditDraw(&loginSearchEdit,loginSearch,(Rectangle){loginSearchBox.x+10,y+3,loginSearchBox.width-20,30},14,loginSearching);else UiText(lab_tr("Cerca uno sfondo..."),loginSearchBox.x+12,y+9,14,muted);
 if(loginSearching)DrawRectangleLinesEx(loginSearchBox,2,AppTheme.border);
 if(GlyphButton((Rectangle){x+w-36,y,36,36},UI_IMPORT,false,!lab_files_busy(),""))lab_files_start(0);
 if(GlyphButton(loginFilterBox,UI_FILTER,loginFiltersOpen,true,"")){loginFiltersOpen=!loginFiltersOpen;choiceOpen=-1;}
 y+=48;if(loginFiltersOpen){float width=fminf(260,area.width*.43);FiltersPanel((Rectangle){area.x,y,width,area.height-106});area.x+=width;area.width-=width;}
 int codes[64],resolutions=ActiveResolutionCodes(codes),mask=0;for(int n=0;n<6;n++)if(wholeResGroups[n])mask|=1<<n;
 unsigned long query=filterType+filterAudio*13+filterSource*67+lab_option("filter_any")*37+mask*119;for(const char* p=loginSearch;*p;p++)query=query*33+(unsigned char)*p;for(int n=0;n<resolutions;n++)query=query*33+codes[n];static unsigned long previous=0;if(query!=previous){previous=query;loginPage=1;loginScroll=0;}
 int indices[MAX_POSSIBLE_WALLPAPERS],count=0,available=0;for(int pass=0;pass<2;pass++){for(int i=0;i<wallpaper_count;i++)if(!deleted[i]&&lab_login_supported(i)==!pass&&(!*loginSearch||stristr(lab_title(i),loginSearch))&&lab_matches_grouped(i,filterType,filterAudio,codes,resolutions,mask,filterSource))indices[count++]=i;if(!pass)available=count;}
 loginAvailableCount=available;loginUnsupportedCount=count-available;Rectangle grid={area.x,y,area.width,area.height-106};if(Hot(grid,true))loginScroll=fmaxf(0,loginScroll-ScrollInput(grid)*64);LibraryGridLayout layout=library_grid_layout(grid.width,180);int per=lab_option("page_size");if(!per)per=100;int pages=fmaxf(1,ceilf(count/(float)per));loginPage=Clamp(loginPage,1,pages);int begin=(loginPage-1)*per,shown=fminf(per,count-begin);float rowHeight=layout.tile+106;
 int good=fmaxf(0,fminf(shown,available-begin)),bad=shown-good;float split=ceilf(good/(float)layout.columns)*rowHeight+38,extent=(good?split:0)+(bad?44+bad*96:0);loginScroll=Clamp(loginScroll,0,fmaxf(0,extent-grid.height));loginCardCount=0;
 BeginScissorMode(grid.x,grid.y,grid.width,grid.height);if(good)UiText(lab_tr("Sfondi disponibili"),grid.x+20,grid.y+4-loginScroll,15,AppTheme.text);if(bad)UiText(lab_tr("Sfondi non supportati"),grid.x+20,grid.y+(good?split:0)+4-loginScroll,15,AppTheme.text);
 for(int n=0;n<shown;n++){int i=indices[begin+n],slot=n<good?n:n-good;float top=n<good?32:(good?split:0)+40;Rectangle card=n<good?(Rectangle){grid.x+20+(slot%layout.columns)*(layout.tile+22),grid.y+top+(slot/layout.columns)*rowHeight-loginScroll,layout.tile,layout.tile+94}:(Rectangle){grid.x+20,grid.y+top+slot*96-loginScroll,grid.width-40,86};if(card.y+card.height<grid.y||card.y>grid.y+grid.height)continue;if(loginCardCount<200){loginCards[loginCardCount]=card;loginCardIndices[loginCardCount++]=i;}bool supported=lab_login_supported(i),selected=i==lab_login_selected();ThemePanel(card,.02f,8,selected?AppTheme.border:control);if(!supported){Rectangle picture={card.x+6,card.y+6,74,74};if(atomic_load(&wallpapers[i].loaded))Cover(wallpapers[i].texture,picture,true);ClippedText(lab_title(i),(Rectangle){card.x+92,card.y+8,card.width-104,24},14,AppTheme.text);TextLines(lab_login_reason(i),card.x+92,card.y+34,card.width-104,12,3,muted);continue;}Rectangle thumb={card.x+5,card.y+5,card.width-10,layout.tile-10};if(atomic_load(&wallpapers[i].loaded))Cover(wallpapers[i].texture,thumb,true);else DrawRectangleRec(thumb,surface);ClippedText(lab_title(i),(Rectangle){card.x+10,card.y+layout.tile+4,card.width-20,24},15,AppTheme.text);TextLines(supported?(!strcmp(lab_origin(i),"local")?lab_tr("Locale"):"Steam"):lab_login_reason(i),card.x+10,card.y+layout.tile+32,card.width-20,12,3,muted);if(Hot(card,supported)&&CheckCollisionPointRec(GetMousePosition(),grid)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))lab_login_select(i);}
 EndScissorMode();int move=PageFooter(area,loginPage,pages,false);if(move){loginPage=move==2?1:loginPage+move;loginScroll=0;}
}
static void LoginWorkspace(Rectangle area){
 const char* tabs[]={lab_tr("Sfondi"),lab_tr("Editor UI di login")};float tx=area.x+16;for(int i=0;i<2;i++){float width=(area.width-48)/2;loginTabs[i]=(Rectangle){tx,area.y+8,width,36};if(Button(loginTabs[i],tabs[i],loginTab==i,true)){loginTab=i;loginInspectorScroll=0;choiceOpen=-1;loginSearching=false;loginEditing=-1;loginPresetEditing=false;colorPickerOpen=-1;}tx+=width+16;}
 int selected=lab_login_selected();Rectangle body={area.x,area.y+56,area.width,area.height-116};float side=Clamp(inspectorSize+12,300,fminf(460,area.width*.43));Rectangle main={body.x+12,body.y,body.width-side-26,body.height};
 if(!loginTab){float top=main.y+LoginPresets(main,false);memset(loginOutputTabs,0,sizeof(loginOutputTabs));if(!strcmp(lab_login_text("/mode"),"single")){int count=fminf(lab_monitor_count(),32);float width=fminf(180,(main.width-6*(count-1))/fmaxf(1,count));for(int i=0;i<count;i++){loginOutputTabs[i]=(Rectangle){main.x+i*(width+6),top,width,32};if(Button(loginOutputTabs[i],lab_monitor_name(i),i==LoginOutputIndex("/output"),true)){lab_login_set_text("/output",lab_monitor_name(i));loginPage=1;loginScroll=0;}}top+=40;}
  float height=loginFiltersOpen?fminf(108,main.height*.18f):fminf(160,main.height*.26f);LoginCanvas((Rectangle){main.x+8,top,main.width-16,height},false);top+=height+12;LoginGrid((Rectangle){main.x,top,main.width,main.y+main.height-top});
 }else{
  float tools=fminf(304,main.width*.49f),column=main.x+main.width-tools,half=(tools-8)/2;
  LoginPresets((Rectangle){main.x,main.y,main.width-tools-16,114},true);
  const char* labels[]={lab_tr("Griglia"),lab_tr("Magnete")},*paths[]={"/grid/visible","/grid/snap"};for(int n=0;n<2;n++)if(Button((Rectangle){column+n*(half+8),main.y+28,half,34},labels[n],lab_login_number(paths[n]),true))lab_login_set_number(paths[n],!lab_login_number(paths[n]));
  loginResetButtons[0]=(Rectangle){column,main.y+70,half,34};loginResetButtons[1]=(Rectangle){column+half+8,main.y+70,half,34};
  if(Button(loginResetButtons[0],lab_tr("Ripristina widget"),false,true)){lab_login_reset_ui(loginWidget);loginInspectorScroll=0;loginEditing=-1;colorPickerOpen=-1;}
  if(DangerButton(loginResetButtons[1],lab_tr("Ripristina UI"),true)){lab_login_reset_ui(-1);loginInspectorScroll=0;loginEditing=-1;colorPickerOpen=-1;}
  DrawLine(main.x,main.y+118,main.x+main.width,main.y+118,lineColor);
  LoginCanvas((Rectangle){main.x+8,main.y+130,main.width-16,fmaxf(80,main.height-170)},true);ClippedText(lab_tr("Trascina i widget per posizionarli"),(Rectangle){main.x+8,main.y+main.height-28,main.width-16,24},12,muted);
 }
 LoginInspector((Rectangle){body.x+body.width-side,body.y,side-12,body.height});
 int y=area.y+area.height-48;loginPreviewButton=(Rectangle){area.x+16,y,220,36};loginPublishButton=(Rectangle){area.x+246,y,250,36};
 if(Button(loginPreviewButton,lab_tr("Anteprima live · 60 secondi"),false,selected>=0&&!lab_login_busy()))lab_login_preview(0);
 if(Button(loginPublishButton,lab_tr("Applica al prossimo avvio"),false,selected>=0&&!lab_login_busy()))lab_login_publish();
 ClippedText(lab_tr(lab_login_notice()),(Rectangle){area.x+510,y+6,fmaxf(0,area.width-530),26},12,muted);
}
static void LoginGeometry(FILE* f){fprintf(f,",\"color_picking\":%d",colorPickerPicking);fprintf(f,",\"login_presets\":");GeometryCenters(f,loginPresetControls,4);fprintf(f,",\"login_palette\":");GeometryCenters(f,loginPaletteButtons,2);fprintf(f,",\"login_sections\":");GeometryCenters(f,loginSectionRows,8);fprintf(f,",\"color_rgb_inputs\":");GeometryCenters(f,colorPickerInputs,3);fprintf(f,",\"login_color_buttons\":");GeometryCenters(f,loginColorButtons,24);fprintf(f,",\"color_eye\":[%.1f,%.1f]",colorPickerEye.x+colorPickerEye.width/2,colorPickerEye.y+colorPickerEye.height/2);fprintf(f,",\"login_position_size\":[%.0f,%.0f]",loginCoordinateSize.x,loginCoordinateSize.y);fprintf(f,",\"login_inspector\":[%.1f,%.1f,%.1f,%.1f],\"login_color\":{\"open\":%d,\"swatch\":[%.1f,%.1f],\"square\":[%.1f,%.1f,%.1f,%.1f]}",loginInspectorArea.x,loginInspectorArea.y,loginInspectorArea.width,loginInspectorArea.height,colorPickerOpen,colorPickerSwatch.x+colorPickerSwatch.width/2,colorPickerSwatch.y+18,colorPickerBox.x,colorPickerBox.y,colorPickerBox.width,colorPickerBox.height);fprintf(f,",\"login_filter\":[%.1f,%.1f],\"login_filters_open\":%s,\"login_search_focus\":%s,\"login_reset\":",loginFilterBox.x+18,loginFilterBox.y+18,loginFiltersOpen?"true":"false",loginSearching?"true":"false");GeometryCenters(f,loginResetButtons,2);fprintf(f,",\"login_text_fields\":");GeometryCenters(f,loginTextBoxes,24);fprintf(f,",\"login_available\":%d,\"login_unsupported\":%d,\"login_search\":[%.1f,%.1f]",loginAvailableCount,loginUnsupportedCount,loginSearchBox.x+loginSearchBox.width/2,loginSearchBox.y+loginSearchBox.height/2);fprintf(f,",\"login_tab\":%d,\"login_enabled\":%s,\"login_selected\":%d,\"login_revision\":%u,\"login_tabs\":",loginTab,lab_login_enabled()?"true":"false",lab_login_selected(),lab_login_revision());GeometryCenters(f,loginTabs,2);fprintf(f,",\"login_inspector_tabs\":");GeometryCenters(f,loginInspectorTabs,2);fprintf(f,",\"login_modes\":");GeometryCenters(f,loginModes,3);fprintf(f,",\"login_outputs\":");GeometryCenters(f,loginOutputTabs,fminf(lab_monitor_count(),32));fprintf(f,",\"login_widgets\":");GeometryCenters(f,loginWidgetRects,3);fprintf(f,",\"login_preview\":[%.1f,%.1f],\"login_publish\":[%.1f,%.1f],\"login_cards\":[",loginPreviewButton.x+loginPreviewButton.width/2,loginPreviewButton.y+18,loginPublishButton.x+loginPublishButton.width/2,loginPublishButton.y+18);for(int n=0;n<loginCardCount;n++){Rectangle r=loginCards[n];fprintf(f,"%s{\"index\":%d,\"rect\":[%.1f,%.1f,%.1f,%.1f]}",n?",":"",loginCardIndices[n],r.x,r.y,r.width,r.height);}fprintf(f,"]");}
