#include <time.h>
static void GeometryCenters(FILE*,const Rectangle*,int);
static int PageFooter(Rectangle,int,int,bool);
// Native login workspace. Catalog thumbnails are shared with the main library.
static int loginTab=0,loginWidget=5,loginCategory=0,loginPage=1,loginSource=0;
static char loginSearch[256]="",loginMessage[257]="";static TextEdit loginSearchEdit,loginMessageEdit;
static bool loginSearching=false,loginMessageEditing=false;
static float loginScroll=0,loginInspectorScroll=0;
static int loginDragging=-1;static Vector2 loginDragOffset;static Rectangle loginDragScreen;
static Rectangle loginTabs[3],loginCards[200],loginTemplateCards[5],loginWidgetRects[6],loginCanvasRect,loginPreviewButton,loginPublishButton,loginSearchBox;
static int loginCardIndices[200],loginCardCount;static Shader loginImageShader={0};
static const char* loginImageKeys[]={"brightness","contrast","saturation","temperature","hue","gamma","vignette","rotation","flip_x","flip_y","zoom","x","y","crop"};
typedef struct {Texture2D texture;unsigned revision;int width,height;long minute;} LoginOverlay;
static LoginOverlay loginOverlay[40];
static void LoginShutdown(void){for(int i=0;i<40;i++)if(loginOverlay[i].texture.id)UnloadTexture(loginOverlay[i].texture);if(loginImageShader.id)UnloadShader(loginImageShader);}
static const char* LoginWidgetPath(int i,const char* key){static char path[96];const char* names[]={"clock","date","calendar","media","message","login"};snprintf(path,sizeof(path),"/widgets/%s/%s",names[i],key);return path;}
static void LoginEditedPreview(Texture2D texture,Rectangle r,const float* values){
 DrawRectangleRec(r,BLACK);if(!texture.id)return;
 if(!loginImageShader.id)loginImageShader=LoadShader(NULL,lab_login_shader());
 Vector2 size={texture.width,texture.height},output={r.width,r.height};Vector3 basic={values[0],values[1],values[2]},geometry={values[7]*DEG2RAD,values[8]>.5?-1:1,values[9]>.5?-1:1},placement={values[11],values[12],values[10]};Vector4 more={values[4]*DEG2RAD,values[3],values[5],values[6]};float crop=values[13];
 SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"imageSize"),&size,SHADER_UNIFORM_VEC2);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"outputSize"),&output,SHADER_UNIFORM_VEC2);
 SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"basic"),&basic,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"more"),&more,SHADER_UNIFORM_VEC4);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"geometry"),&geometry,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"placement"),&placement,SHADER_UNIFORM_VEC3);SetShaderValue(loginImageShader,GetShaderLocation(loginImageShader,"crop"),&crop,SHADER_UNIFORM_FLOAT);
 BeginShaderMode(loginImageShader);DrawTexturePro(texture,(Rectangle){0,0,texture.width,texture.height},r,(Vector2){0},0,WHITE);EndShaderMode();
}

static void LoginOverlayDraw(Rectangle r,int preset,int slot){
 LoginOverlay* cache=&loginOverlay[slot];unsigned rev=(preset<0?lab_login_revision():0)^((lab_option("language")+1)*2654435761u);int width=fmaxf(1,fminf(1280,r.width*1.5f)),height=fmaxf(1,width*r.height/r.width);if(height>1440){width=1440*r.width/r.height;height=1440;}long minute=time(NULL)/60;
 if(!cache->texture.id||cache->revision!=rev||cache->width!=width||cache->height!=height||cache->minute!=minute){
  const unsigned char* pixels=lab_login_pixels(preset,width,height);if(pixels){Image image={(void*)pixels,width,height,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};if(cache->texture.id)UnloadTexture(cache->texture);cache->texture=LoadTextureFromImage(image);SetTextureFilter(cache->texture,TEXTURE_FILTER_BILINEAR);cache->revision=rev;cache->width=width;cache->height=height;cache->minute=minute;}
 }
 if(cache->texture.id){rlDrawRenderBatchActive();BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);DrawTexturePro(cache->texture,(Rectangle){0,0,width,height},r,(Vector2){0},0,WHITE);EndBlendMode();BeginBlendMode(BLEND_CUSTOM_SEPARATE);}
}
static int LoginOutputIndex(const char* key){char name[256];snprintf(name,sizeof(name),"%s",lab_login_text(key));for(int i=0;i<lab_monitor_count();i++)if(!strcmp(name,lab_monitor(i)))return i;return 0;}
static bool LoginUiOn(int i){const char* mode=lab_login_text("/ui_outputs");int main=LoginOutputIndex("/ui_primary");return !strcmp(mode,"all")||(!strcmp(mode,"secondary")?(lab_monitor_count()<2||i!=main):i==main);}
static Rectangle LoginCanvas(Rectangle area,bool edit){
 int count=fminf(lab_monitor_count(),16);float x[16]={0},y[16]={0},w[16]={1920},h[16]={1080},left=1e9,top=1e9,right=-1e9,bottom=-1e9;
 if(!count)count=1;else for(int n=0;n<count;n++)lab_display_rect(n,&x[n],&y[n],&w[n],&h[n]);
 for(int n=0;n<count;n++){left=fminf(left,x[n]);top=fminf(top,y[n]);right=fmaxf(right,x[n]+w[n]);bottom=fmaxf(bottom,y[n]+h[n]);}
 float scale=fminf(area.width/(right-left),area.height/(bottom-top));Rectangle canvas={area.x+(area.width-(right-left)*scale)/2,area.y+(area.height-(bottom-top)*scale)/2,(right-left)*scale,(bottom-top)*scale};loginCanvasRect=canvas;
 int selected=lab_login_selected();Texture2D wallpaper={0};if(selected>=0&&selected<wallpaper_count&&atomic_load(&wallpapers[selected].loaded))wallpaper=wallpapers[selected].texture;
 float image[14];for(int i=0;i<14;i++)image[i]=lab_login_image(loginImageKeys[i]);bool span=!strcmp(lab_login_text("/mode"),"span"),single=!strcmp(lab_login_text("/mode"),"single");int output=LoginOutputIndex("/output");
 DrawRectangleRec(canvas,BLACK);if(span)LoginEditedPreview(wallpaper,canvas,image);memset(loginWidgetRects,0,sizeof(loginWidgetRects));
 for(int n=0;n<count;n++){
  Rectangle screen={canvas.x+(x[n]-left)*scale,canvas.y+(y[n]-top)*scale,w[n]*scale,h[n]*scale};
  if(!span){if(single&&n!=output)DrawRectangleRec(screen,BLACK);else LoginEditedPreview(wallpaper,screen,image);}
  if(LoginUiOn(n)){
   LoginOverlayDraw(screen,-1,5+n);
   if(edit)for(int widget=0;widget<6;widget++)if(lab_login_number(LoginWidgetPath(widget,"enabled"))){
    Rectangle box={screen.x+lab_login_number(LoginWidgetPath(widget,"x"))*screen.width,screen.y+lab_login_number(LoginWidgetPath(widget,"y"))*screen.height,lab_login_number(LoginWidgetPath(widget,"width"))*screen.width,lab_login_number(LoginWidgetPath(widget,"height"))*screen.height};
    loginWidgetRects[widget]=box;if(widget==loginWidget)DrawRectangleLinesEx(box,1.5,AppTheme.border);
    if(Hot(box,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginWidget=widget;loginDragging=widget;loginDragScreen=screen;loginDragOffset=(Vector2){GetMouseX()-box.x,GetMouseY()-box.y};}
   }
  }
  DrawRectangleLinesEx(screen,1,Fade(AppTheme.border,.65));
 }
 if(edit&&loginDragging>=0){if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){float px=Clamp((GetMouseX()-loginDragOffset.x-loginDragScreen.x)/loginDragScreen.width,0,1-lab_login_number(LoginWidgetPath(loginDragging,"width"))),py=Clamp((GetMouseY()-loginDragOffset.y-loginDragScreen.y)/loginDragScreen.height,0,1-lab_login_number(LoginWidgetPath(loginDragging,"height")));lab_login_set_number(LoginWidgetPath(loginDragging,"x"),px);lab_login_set_number(LoginWidgetPath(loginDragging,"y"),py);}else{loginDragging=-1;lab_login_save();}}
 return canvas;
}
static void LoginNumber(int id,int x,int* y,int w,const char* label,const char* path,float lo,float hi,float scale,const char* suffix){float value=lab_login_number(path)*scale,old=value;NumberSlider(id,x,y,w,label,&value,lo*scale,hi*scale,suffix);if(value!=old)lab_login_set_number(path,value/scale);}
static int LoginImageControls(int x,int y,int w){
 int start=y;const char* labels[]={lab_tr("Luminosità"),lab_tr("Contrasto"),lab_tr("Saturazione"),lab_tr("Temperatura"),lab_tr("Tonalità"),lab_tr("Gamma"),lab_tr("Vignettatura"),lab_tr("Rotazione")};float low[]={-1,0,0,-1,-180,.25,0,-180},high[]={1,2,2,1,180,3,1,180};
 for(int n=0;n<8;n++){float factor=n==4||n==5||n==7?1:100,value=lab_login_image(loginImageKeys[n])*factor,old=value;NumberSlider(140+n,x,&y,w,labels[n],&value,low[n]*factor,high[n]*factor,n==4||n==7?"°":n==5?"×":"%");if(value!=old)lab_login_image_set(loginImageKeys[n],value/factor);}
 const char* flips[]={lab_tr("Specchia orizzontalmente"),lab_tr("Specchia verticalmente")};for(int i=0;i<2;i++){bool old=lab_login_image(loginImageKeys[8+i])>.5,value=SwitchToggle((Rectangle){x,y,w,40},flips[i],old);if(value!=old)lab_login_image_set(loginImageKeys[8+i],value);y+=43;}
 for(int i=10;i<13;i++){float value=lab_login_image(loginImageKeys[i])*100,old=value;NumberSlider(138+i,x,&y,w,lab_tr(i==10?"Zoom":i==11?"Sinistra / destra":"Alto / basso"),&value,i==10?25:-100,i==10?300:100,"%");if(value!=old)lab_login_image_set(loginImageKeys[i],value/100);}
 bool old=lab_login_image("crop")>.5,value=SwitchToggle((Rectangle){x,y,w,40},lab_tr("Riempi lo schermo"),old);if(value!=old)lab_login_image_set("crop",value);y+=47;
 if(Button((Rectangle){x,y,w,34},lab_tr("Ripristina immagine"),false,true)){float defaults[]={0,1,1,0,0,1,0,0,0,0,1,0,0,1};for(int n=0;n<14;n++)lab_login_image_set(loginImageKeys[n],defaults[n]);}return y-start+48;
}
static void LoginInspector(Rectangle area){
 ThemePanel(area,.025f,8,surface);Rectangle saved=inputClip;inputClip=area;
 if(!choiceWheel&&Hot(area,true))loginInspectorScroll=fmaxf(0,loginInspectorScroll-ScrollInput(area)*48);choiceWheel=false;
 BeginScissorMode(area.x+1,area.y+1,area.width-2,area.height-2);int x=area.x+16,y=area.y+16-loginInspectorScroll,w=area.width-32;
 if(loginTab==0){LoginCanvas((Rectangle){x,y,w,w*.6f},false);y+=w*.6f+20;y+=LoginImageControls(x,y,w);}
 else{
  const char* kinds[]={lab_tr("Widget"),lab_tr("Aspetto")};int category=Choice(91,x,&y,w,"",kinds,2,loginCategory);if(category!=loginCategory){loginCategory=category;loginInspectorScroll=0;}
  if(!loginCategory){
   const char* names[]={lab_tr("Orologio"),lab_tr("Data"),lab_tr("Calendario"),lab_tr("Media"),lab_tr("Messaggio"),lab_tr("UI di login")};loginWidget=Choice(92,x,&y,w,lab_tr("Widget"),names,6,loginWidget);
   bool old=lab_login_number(LoginWidgetPath(loginWidget,"enabled")),value=BoolChoice(93,x,&y,w,lab_tr("Attivo"),old);if(loginWidget!=5&&old!=value)lab_login_set_number(LoginWidgetPath(loginWidget,"enabled"),value);
   LoginNumber(151,x,&y,w,lab_tr("Sinistra / destra"),LoginWidgetPath(loginWidget,"x"),0,1,100,"%");LoginNumber(152,x,&y,w,lab_tr("Alto / basso"),LoginWidgetPath(loginWidget,"y"),0,1,100,"%");
   LoginNumber(153,x,&y,w,lab_tr("Larghezza"),LoginWidgetPath(loginWidget,"width"),.08,.8,100,"%");LoginNumber(154,x,&y,w,lab_tr("Altezza"),LoginWidgetPath(loginWidget,"height"),.03,.9,100,"%");LoginNumber(155,x,&y,w,lab_tr("Dimensione"),LoginWidgetPath(loginWidget,"size"),10,140,1,"px");
   if(loginWidget==4){Rectangle box={x,y,w,38};if(!loginMessageEditing)snprintf(loginMessage,sizeof(loginMessage),"%s",lab_login_text("/message"));if(Button(box,loginMessage,loginMessageEditing,true)){loginMessageEditing=true;EditFocus(&loginMessageEdit,loginMessage,false);}if(loginMessageEditing){if(EditInput(&loginMessageEdit,loginMessage,sizeof(loginMessage)))lab_login_set_text("/message",loginMessage);if(IsKeyPressed(KEY_ENTER))loginMessageEditing=false;}y+=48;}
  }else{
   const char* fonts[96];int count=fminf(lab_font_count(),96),selected=0;char family[128];snprintf(family,sizeof(family),"%s",lab_login_text("/font"));for(int i=0;i<count;i++){fonts[i]=lab_font_name(i);if(!strcmp(family,fonts[i]))selected=i;}
   int next=Choice(94,x,&y,w,lab_tr("Font dell'interfaccia"),fonts,count,selected);if(next!=selected)lab_login_set_text("/font",fonts[next]);
   bool light=lab_login_number("/light"),value=BoolChoice(95,x,&y,w,lab_tr("Tema chiaro"),light);if(value!=light)lab_login_set_number("/light",value);
   LoginNumber(156,x,&y,w,lab_tr("Opacità"),"/panel_opacity",0,1,100,"%");LoginNumber(157,x,&y,w,lab_tr("Bordi arrotondati"),"/radius",0,60,1,"px");
   const char* colors[]={"#87cfdf","#b58bff","#f2a7c6","#9edba2","#f5bc71","#e6e8ef"};for(int i=0;i<6;i++){Rectangle r={x+i*(w/6),y,w/6-5,30};unsigned color=strtoul(colors[i]+1,NULL,16);DrawRectangleRec(r,(Color){color>>16,(color>>8)&255,color&255,255});if(Hot(r,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))lab_login_set_text("/accent",colors[i]);}y+=44;
  }
 }
 EndScissorMode();float extent=y+loginInspectorScroll-area.y+16;loginInspectorScroll=Clamp(loginInspectorScroll,0,fmaxf(0,extent-area.height));inputClip=saved;
}
static void LoginGrid(Rectangle area){
 int x=area.x+16,y=area.y+8,w=area.width-32;loginSearchBox=(Rectangle){x,y,w-96,36};ThemePanel(loginSearchBox,.1f,8,control);
 if(Hot(loginSearchBox,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){loginSearching=true;EditFocus(&loginSearchEdit,loginSearch,false);}UiText(*loginSearch?loginSearch:lab_tr("Cerca uno sfondo..."),x+12,y+9,14,*loginSearch?AppTheme.text:muted);
 if(loginSearching){if(EditInput(&loginSearchEdit,loginSearch,sizeof(loginSearch)))loginPage=1;if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE))loginSearching=false;}
 if(GlyphButton((Rectangle){x+w-84,y,36,36},UI_IMPORT,false,!lab_files_busy(),""))lab_files_start(0);
 if(GlyphButton((Rectangle){x+w-40,y,36,36},UI_FILTER,loginSource>0,true,"")){loginSource=(loginSource+1)%3;loginPage=1;}
 y+=48;int indices[2048],count=0;for(int i=0;i<wallpaper_count;i++)if(!deleted[i]&&(!*loginSearch||stristr(lab_title(i),loginSearch))&&(!loginSource||!strcmp(lab_origin(i),loginSource==1?"local":"steam")))indices[count++]=i;
 Rectangle grid={area.x,y,area.width,area.height-106};if(Hot(grid,true))loginScroll=fmaxf(0,loginScroll-ScrollInput(grid)*64);LibraryGridLayout layout=library_grid_layout(grid.width,246);int per=lab_option("page_size");if(!per)per=100;int pages=fmaxf(1,ceilf(count/(float)per));loginPage=Clamp(loginPage,1,pages);int begin=(loginPage-1)*per,shown=fminf(per,count-begin);float rowHeight=layout.tile+70,extent=ceilf(shown/(float)layout.columns)*rowHeight;loginScroll=Clamp(loginScroll,0,fmaxf(0,extent-grid.height));loginCardCount=0;
 BeginScissorMode(grid.x,grid.y,grid.width,grid.height);for(int n=0;n<shown;n++){int i=indices[begin+n];Rectangle card={grid.x+20+(n%layout.columns)*(layout.tile+22),grid.y+(n/layout.columns)*rowHeight-loginScroll,layout.tile,layout.tile+58};if(card.y+card.height<grid.y||card.y>grid.y+grid.height)continue;loginCards[loginCardCount]=card;loginCardIndices[loginCardCount++]=i;bool supported=lab_login_supported(i),selected=i==lab_login_selected();ThemePanel(card,.02f,8,selected?AppTheme.border:control);Rectangle thumb={card.x+5,card.y+5,card.width-10,layout.tile-10};if(atomic_load(&wallpapers[i].loaded))Cover(wallpapers[i].texture,thumb,true);else DrawRectangleRec(thumb,surface);ClippedText(lab_title(i),(Rectangle){card.x+10,card.y+layout.tile+4,card.width-20,24},15,AppTheme.text);UiText(!supported?lab_tr("Non supportato"):!strcmp(lab_origin(i),"local")?lab_tr("Locale"):"Steam",card.x+10,card.y+layout.tile+32,12,muted);if(Hot(card,supported)&&CheckCollisionPointRec(GetMousePosition(),grid)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))lab_login_select(i);}
 EndScissorMode();int move=PageFooter(area,loginPage,pages,false);if(move){loginPage=move==2?1:loginPage+move;loginScroll=0;}
}
static void LoginDisplays(Rectangle area){
 int x=area.x+20,y=area.y+12,w=area.width-40;const char* modes[]={lab_tr("Monitor singolo"),lab_tr("Duplicato"),lab_tr("Panoramico")};const char* values[]={"single","mirror","span"};int mode=0;for(int i=0;i<3;i++)if(!strcmp(lab_login_text("/mode"),values[i]))mode=i;int next=Choice(89,x,&y,w,lab_tr("Display"),modes,3,mode);if(next!=mode)lab_login_set_text("/mode",values[next]);
 const char* outputs[32];int count=fminf(lab_monitor_count(),32);for(int i=0;i<count;i++)outputs[i]=lab_monitor(i);if(count){int at=LoginOutputIndex("/output"),n=Choice(90,x,&y,w,lab_tr("Monitor wallpaper"),outputs,count,at);if(n!=at)lab_login_set_text("/output",outputs[n]);at=LoginOutputIndex("/ui_primary");n=Choice(91,x,&y,w,lab_tr("Monitor principale"),outputs,count,at);if(n!=at)lab_login_set_text("/ui_primary",outputs[n]);}
 const char* places[]={lab_tr("Principale"),lab_tr("Secondario"),lab_tr("Tutti")},*keys[]={"primary","secondary","all"};int at=0;for(int i=0;i<3;i++)if(!strcmp(lab_login_text("/ui_outputs"),keys[i]))at=i;next=Choice(92,x,&y,w,lab_tr("UI di login"),places,3,at);if(next!=at)lab_login_set_text("/ui_outputs",keys[next]);
 LoginNumber(158,x,&y,w,lab_tr("Limite FPS"),"/fps",15,120,1,"FPS");y+=12;
 LoginCanvas((Rectangle){x,y,w,fmaxf(90,area.y+area.height-y-100)},false);
}
static void LoginWorkspace(Rectangle area){
 const char* tabs[]={lab_tr("Sfondi"),lab_tr("Template e widget"),lab_tr("Monitor e avvio")};for(int i=0;i<3;i++){loginTabs[i]=(Rectangle){area.x+16+i*202,area.y+8,194,38};if(Button(loginTabs[i],tabs[i],loginTab==i,true)){loginTab=i;loginInspectorScroll=0;choiceOpen=-1;loginSearching=false;}}
 int selected=lab_login_selected();Rectangle body={area.x,area.y+58,area.width,area.height-118};float side=Clamp(area.width*.29,300,380);
 if(loginTab==0){LoginGrid((Rectangle){body.x,body.y,body.width-side-12,body.height});LoginInspector((Rectangle){body.x+body.width-side,body.y,side-12,body.height});}
 else if(loginTab==1){Rectangle canvasArea={body.x+18,body.y+140,body.width-side-40,body.height-160};int w=(body.width-side-50)/5;for(int i=0;i<5;i++){Rectangle card={body.x+18+i*(w+6),body.y+5,w,112};loginTemplateCards[i]=card;ThemePanel(card,.03,8,lab_login_number("/template")==i?AppTheme.border:control);Rectangle thumb={card.x+4,card.y+4,card.width-8,78};if(selected>=0&&atomic_load(&wallpapers[selected].loaded))Cover(wallpapers[selected].texture,thumb,true);else DrawRectangleGradientV(thumb.x,thumb.y,thumb.width,thumb.height,(Color){22,39,55,255},(Color){65,37,81,255});LoginOverlayDraw(thumb,i,i);const char* names[]={"Orbit","Horizon","Neon","Paper","Canvas"};UiText(names[i],card.x+9,card.y+87,14,AppTheme.text);if(Hot(card,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))lab_login_template(i);}
 LoginCanvas(canvasArea,true);LoginInspector((Rectangle){body.x+body.width-side,body.y,side-12,body.height});}
 else LoginDisplays(body);
 int y=area.y+area.height-48;loginPreviewButton=(Rectangle){area.x+16,y,220,36};loginPublishButton=(Rectangle){area.x+246,y,250,36};
 if(Button(loginPreviewButton,lab_tr("Anteprima live · 60 secondi"),false,selected>=0&&!lab_login_busy()))lab_login_preview(0);
 if(Button(loginPublishButton,lab_tr("Applica al prossimo avvio"),false,selected>=0&&!lab_login_busy()))lab_login_publish();
 ClippedText(lab_tr(lab_login_notice()),(Rectangle){area.x+510,y+6,fmaxf(0,area.width-530),26},12,muted);
}
static void LoginGeometry(FILE* f){fprintf(f,",\"login_tab\":%d,\"login_enabled\":%s,\"login_selected\":%d,\"login_revision\":%u,\"login_tabs\":",loginTab,lab_login_enabled()?"true":"false",lab_login_selected(),lab_login_revision());GeometryCenters(f,loginTabs,3);fprintf(f,",\"login_templates\":");GeometryCenters(f,loginTemplateCards,5);fprintf(f,",\"login_widgets\":");GeometryCenters(f,loginWidgetRects,6);fprintf(f,",\"login_preview\":[%.1f,%.1f],\"login_publish\":[%.1f,%.1f],\"login_cards\":[",loginPreviewButton.x+loginPreviewButton.width/2,loginPreviewButton.y+18,loginPublishButton.x+loginPublishButton.width/2,loginPublishButton.y+18);for(int n=0;n<loginCardCount;n++){Rectangle r=loginCards[n];fprintf(f,"%s{\"index\":%d,\"rect\":[%.1f,%.1f,%.1f,%.1f]}",n?",":"",loginCardIndices[n],r.x,r.y,r.width,r.height);}fprintf(f,"]");}
