// Native library shell shared by Library, Carousel and Settings.
// Kept separate from the transient compositor popup renderer.
#include "font_fallback.h"
#include "library_layout.h"
static Font uiFont,brandFont,buttonFont,fallbackFont;
static RenderTexture2D uiCanvas;static bool uiCanvasActive=false;
static int iconIndex=-1;
static int displayDragging=-1,displaySelected=0;
static Vector2 displayMapOrigin,displayDragMouse,displayDragPosition;
static float displayMapScale=1;
static Color surface,control,controlHover,muted,lineColor;
static int page=0,settingsTab=4,deleteIndex=-1,filterType=0,filterAudio=0,filterSource=0;
static bool transitionPreview=false,switcherPreview=false,transitionDrawing=false,switcherPreviewBlocks=false;
static int animationCustomize=0;static double animationOpened=0;
static int choiceOpen=-1;static bool choiceRendering=false,choiceRegistering=false;
static bool deleteReady=false;
static double deleteOpenedAt=0;
static void RequestDelete(int index){deleteIndex=index;deleteReady=false;deleteOpenedAt=GetTime();}
static bool wholeResGroups[6]={0};
static bool selectedResolutions[64]={0},resolutionGroups[6]={true,true,false,false,false,false};
static int libraryPage=1,libraryTotal=0;static unsigned long libraryQuery=0;
static float filterSize=260;static bool filterResizing=false;
static float filterScroll=0,filterScrollDraw=0,filterWidth=0;
static Rectangle resolutionRows[64],resolutionHeaders[6],settingsTabs[8],themeChoices[64],languageChoices[32],densityChoices[3],fontChoices[96],fontControl,shortcutControl,languageControl,variantChoices[2];
static bool uiSmoke=false,detailResizing=false,variantOpen=false;
static float inspectorSize=390;

static Texture2D liveTexture={0};static int liveItem=-1;
static double liveLastFrame=0;
static bool scanning=false,fontMenu=false,languageMenu=false;static float settingsExtent=900;static int seenFontRevision=-1;
static bool displayPopup=false;static float displayPopupScroll=0,displayPopupScrollDraw=0;
static bool settingsOpen=false,filtersOpen=false,detailOpen=false,recordingKey=false;
static bool deleted[MAX_POSSIBLE_WALLPAPERS]={0},modalDrawing=false,inputBlocked=false;
static float starPulse[MAX_POSSIBLE_WALLPAPERS]={0},detailWidth=0,detailScroll=0,settingsScroll=0,detailScrollDraw=0,settingsScrollDraw=0;
static Rectangle libraryCards[MAX_POSSIBLE_WALLPAPERS],inspectorPreviewRect;
static Rectangle inputClip,headerFilter,headerMoon,headerRefresh,headerDisplay,headerSettings,headerLibrary,headerCarousel,headerWorkshop,headerImport;
static char notice[256]="";
static void StartLibraryScan(void){if(lab_rescan_start()){scanning=true;snprintf(notice,sizeof(notice),lab_tr("Rilettura della libreria Steam..."));}}
static const int headerHeight=164,statusHeight=42;
#include "ui_theme.h"
static Font LoadSystemFont(const char* path,int size,int* glyphs,int count);
static void ReloadUiFonts(void);
static Font fontPreview[96];
static float SmoothScroll(float current,float target){if(lab_option("reduce_motion"))return target;return fabsf(current-target)<.1f?target:Lerp(current,target,1-expf(-GetFrameTime()*15));}
#include "shaped_text.h"
typedef struct {unsigned int texture;int cp,index;} UiGlyphCache;
static UiGlyphCache glyphCache[16384];
static int UiGlyphIndex(Font font,int cp){unsigned at=((unsigned)cp*2654435761u+font.texture.id*31u)&16383u;UiGlyphCache* c=&glyphCache[at];if(c->texture!=font.texture.id||c->cp!=cp){c->texture=font.texture.id;c->cp=cp;c->index=GetGlyphIndex(font,cp);}return c->index;}
static Font TextFont(int codepoint){
    int index=UiGlyphIndex(uiFont,codepoint);bool missing=uiFont.glyphs[index].value!=codepoint||(codepoint!=32&&uiFont.glyphs[index].advanceX==0&&uiFont.recs[index].width==0);
    return missing&&fallbackFont.texture.id?fallbackFont:uiFont;
}
static float GlyphAdvance(Font font,int cp,int size){int index=UiGlyphIndex(font,cp);return (font.glyphs[index].advanceX?font.glyphs[index].advanceX:font.recs[index].width+font.glyphs[index].offsetX)*size/(float)font.baseSize;}
static void UiText(const char* text,int x,int y,int size,Color color){
    if(ComplexText(text)){ShapedText* c=Shape(text,size);DrawTexture(c->texture,x,y,color);return;}
    float px=x;while(*text){int bytes=0,cp=GetCodepointNext(text,&bytes);if(bytes<1)break;Font f=TextFont(cp);int n=UiGlyphIndex(f,cp);float scale=size/(float)f.baseSize,pad=f.glyphPadding;
        Rectangle source={f.recs[n].x-pad,f.recs[n].y-pad,f.recs[n].width+2*pad,f.recs[n].height+2*pad};Rectangle dest={px+(f.glyphs[n].offsetX-pad)*scale,y+(f.glyphs[n].offsetY-pad)*scale,source.width*scale,source.height*scale};
        DrawTexturePro(f.texture,source,dest,(Vector2){0},0,color);px+=GlyphAdvance(f,cp,size);text+=bytes;}
}
static int UiMeasure(const char* text,int size){if(ComplexText(text))return Shape(text,size)->width;float width=0;while(*text){int bytes=0,cp=GetCodepointNext(text,&bytes);if(bytes<1)break;width+=GlyphAdvance(TextFont(cp),cp,size);text+=bytes;}return ceilf(width);}
static void ButtonText(const char* text,float x,float y,Color color){if(ComplexText(text)){UiText(text,x,y,15,color);return;}DrawTextEx(buttonFont,text,(Vector2){x,y},15,0,color);}
static bool Hot(Rectangle r,bool enabled){return enabled&&(!(switcherPreview||animationCustomize)||transitionDrawing||choiceRendering)&&!switcherPreviewBlocks&&(!transitionPreview||transitionDrawing||choiceRendering)&&(choiceOpen<0||choiceRendering||choiceRegistering)&&(!inputBlocked||modalDrawing)&&CheckCollisionPointRec(GetMousePosition(),inputClip)&&CheckCollisionPointRec(GetMousePosition(),r);}
static float ScrollInput(Rectangle area){
 static Rectangle owner;static Vector2 lastMiddle;bool over=CheckCollisionPointRec(GetMousePosition(),area);if(choiceOpen>=0&&!choiceRendering)return 0;
 if(over&&IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)){owner=area;lastMiddle=GetMousePosition();}
 float wheel=over?GetMouseWheelMove():0;
 if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)&&owner.x==area.x&&owner.y==area.y&&owner.width==area.width){SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);Vector2 mouse=GetMousePosition();wheel+=(mouse.y-lastMiddle.y)/48.f;lastMiddle=mouse;}
 return wheel;
}
static bool Button(Rectangle r,const char* label,bool active,bool enabled){
    bool hot=Hot(r,enabled);Color bg=active?AppTheme.border:hot?controlHover:control;if(active)bg.a=control.a;
    ThemePanel(r,0.22f,12,enabled?bg:Fade(bg,0.45f));
    Color ink=active?(lab_option("light_theme")?WHITE:(Color){17,30,25,255}):AppTheme.text;
    char shown[512];snprintf(shown,sizeof(shown),"%s",label);bool complex=ComplexText(label);float measured=complex?UiMeasure(label,15):MeasureTextEx(buttonFont,label,15,0).x;float fontSize=Clamp(15*(r.width-16)/fmaxf(1,measured),12,15);
    while(strlen(shown)>3&&(complex?UiMeasure(shown,fontSize):MeasureTextEx(buttonFont,shown,fontSize,0).x)>r.width-16){int len=strlen(shown);if(len>=3&&!strcmp(shown+len-3,"..."))shown[len-=3]=0;len--;while(len>0&&((unsigned char)shown[len]&0xc0)==0x80)len--;shown[len]=0;strcat(shown,"...");}
    if(ComplexText(shown))UiText(shown,r.x+(r.width-UiMeasure(shown,fontSize))/2,r.y+(r.height-fontSize)/2,fontSize,enabled?ink:Fade(ink,.35f));else DrawTextEx(buttonFont,shown,(Vector2){r.x+(r.width-MeasureTextEx(buttonFont,shown,fontSize,0).x)/2,r.y+(r.height-fontSize)/2-1},fontSize,0,enabled?ink:Fade(ink,0.35f));
    if(hot)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static bool DangerButton(Rectangle r,const char* label,bool enabled){
 Color before=control,hover=controlHover,ink=AppTheme.text;Color red=lab_option("light_theme")?(Color){181,45,65,255}:(Color){169,48,69,255};red.a=before.a;control=red;controlHover=ColorLerp(red,WHITE,.09f);AppTheme.text=(Color){255,245,247,255};bool clicked=Button(r,label,false,enabled);DrawRectangleRoundedLinesEx(r,.22f*themeRoundness,12,1,Fade((Color){247,131,145,255},.7f));control=before;controlHover=hover;AppTheme.text=ink;return clicked;
}
static void StarIcon(Vector2 c,float radius,Color color,bool filled){
    Vector2 points[10];for(int n=0;n<10;n++){float a=(-90+n*36)*DEG2RAD,r=radius*(n%2?0.46f:1);points[n]=(Vector2){c.x+cosf(a)*r,c.y+sinf(a)*r};}
    for(int n=0;n<10;n++){if(filled)DrawTriangle(c,points[(n+1)%10],points[n],color);DrawLineEx(points[n],points[(n+1)%10],1.6f,color);}
}
typedef enum {UI_STAR,UI_TRASH,UI_GEAR,UI_MOON,UI_SEARCH,UI_FILTER,UI_CLOSE,UI_PLAY,UI_LIVE,UI_LOGO,UI_KEY,UI_EYE,UI_LIBRARY,UI_CAROUSEL,UI_REFRESH,UI_DISPLAY,UI_DOWNLOAD,UI_CHECK,UI_IMPORT,UI_CUSTOM,UI_LEFT,UI_RIGHT} UiIcon;
static Texture2D iconAtlas;
static void IconRaw(UiIcon icon,Vector2 c,Color ink,bool fill){
    switch(icon){
    case UI_STAR:StarIcon(c,10,ink,fill);break;
    case UI_TRASH:
        DrawLineEx((Vector2){c.x-8,c.y-6},(Vector2){c.x+8,c.y-6},1.8f,ink);
        DrawLineEx((Vector2){c.x-3,c.y-10},(Vector2){c.x+3,c.y-10},1.8f,ink);
        DrawRectangleRoundedLinesEx((Rectangle){c.x-6,c.y-3,12,13},0.15f,4,1.8f,ink);
        DrawLineEx((Vector2){c.x-2,c.y},(Vector2){c.x-2,c.y+6},1.5f,ink);DrawLineEx((Vector2){c.x+2,c.y},(Vector2){c.x+2,c.y+6},1.5f,ink);break;
    case UI_GEAR:
        DrawRing(c,5,8,0,360,32,ink);
        for(int n=0;n<8;n++){float a=n*PI/4;DrawLineEx((Vector2){c.x+cosf(a)*8,c.y+sinf(a)*8},(Vector2){c.x+cosf(a)*11,c.y+sinf(a)*11},3,ink);}break;
    case UI_MOON:
        DrawCircleSector(c,10,50,310,36,ink);DrawCircleSector((Vector2){c.x+4,c.y-3},8,55,305,30,fill?controlHover:control);break;
    case UI_CHECK:DrawRing(c,9.2f,10.8f,0,360,48,ink);DrawLineEx((Vector2){c.x-5,c.y},(Vector2){c.x-1,c.y+4},2,ink);DrawLineEx((Vector2){c.x-1,c.y+4},(Vector2){c.x+6,c.y-4},2,ink);break;
    case UI_IMPORT:DrawRectangleRoundedLinesEx((Rectangle){c.x-10,c.y-6,20,15},.15f,6,1.7f,ink);DrawLineEx((Vector2){c.x-9,c.y-9},(Vector2){c.x-2,c.y-9},1.7f,ink);DrawLineEx((Vector2){c.x,c.y-2},(Vector2){c.x,c.y+5},1.7f,ink);DrawLineEx((Vector2){c.x-4,c.y+1},(Vector2){c.x+4,c.y+1},1.7f,ink);break;
    case UI_DOWNLOAD:DrawLineEx((Vector2){c.x,c.y-10},(Vector2){c.x,c.y+3},2,ink);DrawLineEx((Vector2){c.x-5,c.y-1},(Vector2){c.x,c.y+4},2,ink);DrawLineEx((Vector2){c.x,c.y+4},(Vector2){c.x+5,c.y-1},2,ink);DrawLineEx((Vector2){c.x-9,c.y+2},(Vector2){c.x-9,c.y+9},2,ink);DrawLineEx((Vector2){c.x-9,c.y+9},(Vector2){c.x+9,c.y+9},2,ink);DrawLineEx((Vector2){c.x+9,c.y+9},(Vector2){c.x+9,c.y+2},2,ink);break;
    case UI_SEARCH:DrawRing((Vector2){c.x-2,c.y-2},5.2f,6.8f,0,360,40,ink);DrawLineEx((Vector2){c.x+3,c.y+3},(Vector2){c.x+9,c.y+9},2,ink);break;
    case UI_FILTER:
        for(int n=0;n<3;n++){float y=c.y-7+n*7;DrawLineEx((Vector2){c.x-10,y},(Vector2){c.x+10,y},1.6f,ink);DrawCircle(c.x+(n%2?4:-4),y,3,ink);}break;
    case UI_CLOSE:DrawLineEx((Vector2){c.x-6,c.y-6},(Vector2){c.x+6,c.y+6},1.8f,ink);DrawLineEx((Vector2){c.x+6,c.y-6},(Vector2){c.x-6,c.y+6},1.8f,ink);break;
    case UI_PLAY:DrawTriangle((Vector2){c.x-5,c.y-8},(Vector2){c.x-5,c.y+8},(Vector2){c.x+8,c.y},ink);break;
    case UI_LIVE:DrawRectangleRoundedLinesEx((Rectangle){c.x-10,c.y-7,20,14},0.2f,4,1.8f,ink);DrawLineEx((Vector2){c.x-5,c.y+11},(Vector2){c.x+5,c.y+11},1.8f,ink);break;
    case UI_EYE:for(int n=0;n<48;n++){float a=n*2*PI/48,b=(n+1)*2*PI/48;DrawLineEx((Vector2){c.x+cosf(a)*11,c.y+sinf(a)*7},(Vector2){c.x+cosf(b)*11,c.y+sinf(b)*7},1.6f,ink);}DrawCircleV(c,3,ink);break;
    case UI_LIBRARY:for(int y=-1;y<=1;y+=2)for(int x=-1;x<=1;x+=2)DrawRectangleRoundedLinesEx((Rectangle){c.x+x*5-3,c.y+y*5-3,7,7},.2f,4,1.5f,ink);break;
    case UI_CAROUSEL:DrawRectangleRoundedLinesEx((Rectangle){c.x-6,c.y-9,12,18},.2f,6,1.6f,ink);DrawLineEx((Vector2){c.x-10,c.y-5},(Vector2){c.x-10,c.y+5},1.6f,ink);DrawLineEx((Vector2){c.x+10,c.y-5},(Vector2){c.x+10,c.y+5},1.6f,ink);break;
    case UI_DISPLAY:DrawRectangleRoundedLinesEx((Rectangle){c.x-10,c.y-8,20,13},.18f,6,1.8f,ink);DrawLineEx((Vector2){c.x,c.y+5},(Vector2){c.x,c.y+9},1.8f,ink);DrawLineEx((Vector2){c.x-6,c.y+10},(Vector2){c.x+6,c.y+10},1.8f,ink);break;
    case UI_REFRESH:{
        float rotation=(scanning||(page==3&&lab_ws_number(-1,"busy")))?GetTime()*220:0;
        for(int side=0;side<2;side++){
            float start=rotation-60+side*180,end=start+130;DrawRing(c,7,8.8f,start,end,24,ink);
            float a=end*DEG2RAD;Vector2 tip={c.x+7.9f*cosf(a),c.y+7.9f*sinf(a)},t={-sinf(a),cosf(a)},n={cosf(a),sinf(a)};
            DrawTriangle((Vector2){tip.x+t.x*3,tip.y+t.y*3},(Vector2){tip.x-t.x*3+n.x*3.6f,tip.y-t.y*3+n.y*3.6f},(Vector2){tip.x-t.x*3-n.x*3.6f,tip.y-t.y*3-n.y*3.6f},ink);
        }}break;
    case UI_KEY:DrawRectangleRoundedLinesEx((Rectangle){c.x-11,c.y-8,22,16},0.2f,4,1.8f,ink);for(int n=0;n<3;n++)DrawCircle(c.x-6+n*6,c.y-2,1.3f,ink);DrawLineEx((Vector2){c.x-5,c.y+4},(Vector2){c.x+5,c.y+4},1.5f,ink);break;
    case UI_CUSTOM:DrawLineEx((Vector2){c.x-8,c.y+8},(Vector2){c.x+5,c.y-5},3,ink);DrawCircleV((Vector2){c.x+7,c.y-7},3,ink);for(int n=0;n<4;n++){float a=n*PI/2;DrawLineEx((Vector2){c.x-7+cosf(a)*3,c.y-7+sinf(a)*3},(Vector2){c.x-7+cosf(a)*6,c.y-7+sinf(a)*6},1.5f,ink);}break;
    case UI_LEFT:case UI_RIGHT:{float sign=icon==UI_LEFT?-1:1;DrawLineEx((Vector2){c.x-sign*4,c.y-7},(Vector2){c.x+sign*3,c.y},2,ink);DrawLineEx((Vector2){c.x+sign*3,c.y},(Vector2){c.x-sign*4,c.y+7},2,ink);}break;
    case UI_LOGO:
        DrawRectangleRoundedLinesEx((Rectangle){c.x-16,c.y-16,32,32},0.3f,12,1.8f,ink);
        DrawLineEx((Vector2){c.x-11,c.y+9},(Vector2){c.x-2,c.y-3},2,ink);DrawLineEx((Vector2){c.x-2,c.y-3},(Vector2){c.x+4,c.y+3},2,ink);DrawLineEx((Vector2){c.x+4,c.y+3},(Vector2){c.x+10,c.y-5},2,ink);DrawCircle(c.x+6,c.y-9,2.5f,ink);break;
    }
}
static RenderTexture2D iconAtlasTarget;
static void InitIconAtlas(void){iconAtlasTarget=LoadRenderTexture(40*3*(UI_RIGHT+1),40*3*2);BeginTextureMode(iconAtlasTarget);ClearBackground(BLANK);rlPushMatrix();rlScalef(3,3,1);for(int row=0;row<2;row++)for(int n=0;n<=UI_RIGHT;n++)IconRaw(n,(Vector2){n*40+20,row*40+20},WHITE,row);rlPopMatrix();EndTextureMode();iconAtlas=iconAtlasTarget.texture;GenTextureMipmaps(&iconAtlas);SetTextureFilter(iconAtlas,TEXTURE_FILTER_TRILINEAR);}
static void Icon(UiIcon icon,Vector2 c,Color ink,bool fill){if(!iconAtlas.id){IconRaw(icon,c,ink,fill);return;}DrawTexturePro(iconAtlas,(Rectangle){icon*120,fill?0:120,120,-120},(Rectangle){c.x-20,c.y-20,40,40},(Vector2){0},0,ink);}
static bool GlyphButton(Rectangle r,UiIcon icon,bool active,bool enabled,const char* label){
    bool hot=Hot(r,enabled);bool star=icon==UI_STAR;
    Color bg=star?(Color){31,35,41,255}:hot?controlHover:control;if(icon==UI_KEY&&active&&recordingKey)bg=(Color){26,89,55,255};
    ThemePanel(r,0.24f,12,enabled?bg:Fade(bg,0.5f));
    Color ink=icon==UI_TRASH?(Color){236,105,115,255}:star?WHITE:AppTheme.text;
    if(!enabled)ink=Fade(ink,0.35f);
    Vector2 center={r.x+(label&&*label?23:r.width/2),r.y+r.height/2};
    int index=iconIndex>=0?iconIndex:selected_index;
    if(star&&index>=0){float p=starPulse[index];if(p<=0)Icon(UI_STAR,center,ink,active);else StarIcon(center,10+3*sinf(p*PI),ink,active);if(p>0)DrawCircleLines(center.x,center.y,12+(1-p)*8,Fade(ink,p*0.4f));}
    else Icon(icon,center,ink,active);
    if(label&&*label)ButtonText(label,r.x+44,r.y+(r.height-15)/2-1,enabled?(star?WHITE:AppTheme.text):Fade(star?WHITE:AppTheme.text,0.35f));
    if(active&&!star)DrawRectangleRoundedLinesEx(r,0.24f,12,1.2f,Fade(AppTheme.border,0.8f));
    if(hot)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static bool ActionGlyphButton(Rectangle r,UiIcon icon,bool enabled,const char* label){
 Color before=control,hover=controlHover;Color tone=icon==UI_TRASH?(Color){232,94,113,255}:AppTheme.border;control=ColorLerp(surface,tone,.2f);controlHover=ColorLerp(surface,tone,.34f);bool clicked=GlyphButton(r,icon,false,enabled,label);DrawRectangleRoundedLinesEx(r,.24f*themeRoundness,12,1,Fade(tone,.45f));control=before;controlHover=hover;return clicked;
}
static void ToggleStar(int i){if(lab_favorite_change(i,0))starPulse[i]=1;}
// Draw a bounded number of lines, preserving UTF-8 boundaries when truncating.
static int TextLines(const char* text,int x,int y,int width,int size,int maxLines,Color ink){
    const char* p=text;int row=0;
    while(*p&&row<maxLines){
        char line[1024]="";int n=0,lastSpace=-1;
        while(p[n]&&p[n]!='\n'&&n<1000){
            int bytes=1;unsigned char c=p[n];if(c>=0xc0)bytes=c<0xe0?2:c<0xf0?3:4;
            if(n+bytes>=1000)break;
            memcpy(line+n,p+n,bytes);line[n+bytes]=0;
            if(UiMeasure(line,size)>width){line[n]=0;break;}
            if(p[n]==' ')lastSpace=n;n+=bytes;
        }
        if(!n)n=1;
        if(p[n]&&p[n]!='\n'&&lastSpace>0)n=lastSpace;
        memcpy(line,p,n);line[n]=0;
        const char* next=p+n;while(*next==' '||*next=='\n')next++;
        if(row==maxLines-1&&*next){
            while(n>0&&UiMeasure(TextFormat("%s...",line),size)>width){do{n--;}while(n>0&&((unsigned char)line[n]&0xc0)==0x80);line[n]=0;}
            strcat(line,"...");
        }
        UiText(line,x,y+row*(size+7),size,ink);row++;p=next;
    }
    return row*(size+7);
}
static void WrappedText(const char* text,int x,int y,int width,int size,Color color){TextLines(text,x,y,width,size,8,color);}
static void ClippedText(const char* text,Rectangle r,int size,Color color){TextLines(text,r.x,r.y,r.width,size,1,color);}
static const char* TypeLabel(int i){const char* type=lab_type(i);return !strcmp(type,"scene")?lab_tr("Scena"):!strcmp(type,"video")?lab_tr("Video"):!strcmp(type,"web")?lab_tr("Web"):!strcmp(type,"preset")?lab_tr("Preset"):!strcmp(type,"image")?lab_tr("Immagine"):lab_tr("Non disponibile");}
static const char* ResolutionLabel(int i){return lab_width(i)>0?TextFormat("%d × %d",lab_width(i),lab_height(i)):lab_tr("Risoluzione da verificare");}
static void Cover(Texture2D t,Rectangle r,bool contain){
    if(!t.id)return;
    float aspect=(float)t.width/t.height;
    Rectangle source={0,0,t.width,t.height};
    if(contain){
        Rectangle dst=r;if(r.width/r.height>aspect){dst.width=r.height*aspect;dst.x+=(r.width-dst.width)/2;}else{dst.height=r.width/aspect;dst.y+=(r.height-dst.height)/2;}
        DrawTexturePro(t,source,dst,(Vector2){0},0,WHITE);
    }else{
        if(aspect>r.width/r.height){source.width=t.height*r.width/r.height;source.x=(t.width-source.width)/2;}else{source.height=t.width*r.height/r.width;source.y=(t.height-source.height)/2;}
        DrawTexturePro(t,source,r,(Vector2){0},0,WHITE);
    }
}
static void SetRecording(bool enabled){
    if(enabled){if(lab_record_shortcut(1))recordingKey=true;}
    else {lab_record_shortcut(0);recordingKey=false;}
}
static void SelectItem(int index,int visibleIndex){
    (void)visibleIndex;selected_index=index;g_hoveredIndex=index;detailOpen=true;detailScroll=detailScrollDraw=0;isSearching=false;variantOpen=false;
    if(!uiSmoke||getenv("WALLPAPER_LAB_TEST_APPLY"))applying=lab_select(index);
}
static void WorkshopEnter(int previous,int next);
static void ChangePage(int next){WorkshopEnter(page,next);page=next;isSearching=false;displayDragging=-1;SetRecording(false);settingsScroll=settingsScrollDraw=0;g_scroll=(Vector2){0};g_smoothScrollY=0;}
static const char* KeyName(int key){
    if(key>=KEY_A&&key<=KEY_Z)return TextFormat("%c",key);
    if(key>=KEY_ZERO&&key<=KEY_NINE)return TextFormat("%c",key);
    if(key>=KEY_F1&&key<=KEY_F12)return TextFormat("F%d",key-KEY_F1+1);
    switch(key){case KEY_TAB:return "Tab";case KEY_SPACE:return "space";case KEY_RIGHT:return "Right";case KEY_LEFT:return "Left";case KEY_UP:return "Up";case KEY_DOWN:return "Down";case KEY_PAGE_UP:return "Page_Up";case KEY_PAGE_DOWN:return "Page_Down";case KEY_HOME:return "Home";case KEY_END:return "End";default:return NULL;}
}
static void RecordShortcut(void){
    int key;while((key=GetKeyPressed())>0){
        if(key==KEY_ESCAPE){SetRecording(false);return;}
        const char* name=KeyName(key);if(!name)continue;
        int mods=(IsKeyDown(KEY_LEFT_SUPER)||IsKeyDown(KEY_RIGHT_SUPER)?64:0)|(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL)?4:0)|(IsKeyDown(KEY_LEFT_ALT)||IsKeyDown(KEY_RIGHT_ALT)?8:0)|(IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT)?1:0);
        if(!mods){snprintf(notice,sizeof(notice),lab_tr("Usa Ctrl, Alt, Shift o Super con un tasto."));continue;}
        char combo[128]="";if(mods&64)strcat(combo,"SUPER+");if(mods&4)strcat(combo,"CTRL+");if(mods&8)strcat(combo,"ALT+");if(mods&1)strcat(combo,"SHIFT+");strcat(combo,name);
        SetRecording(false);if(lab_set_shortcut(combo,1))snprintf(notice,sizeof(notice),lab_tr("Shortcut attivo: %s"),lab_shortcut());
        return;
    }
}
#include "text_edit.h"
static TextEdit searchEdit;
static void FocusSearch(bool select){choiceOpen=-1;isSearching=true;EditFocus(&searchEdit,searchBuffer,select);}
static float ShortcutInline(float x,float y,float available);
static float CompactGlyphWidth(const char* label);
#include "settings_ui.h"
#include "resolution_picker.h"
#include "display_editor.h"
#include "transition_ui.h"
static float opacityDraft=0;
static void SettingsBody(Rectangle area){
    int x=area.x+16,y=area.y+12,w=area.width-32;
    const char* tabs[]={lab_tr("Display"),lab_tr("Riproduzione"),lab_tr("Carousel"),lab_tr("Aspetto"),lab_tr("Generali"),"Workshop",lab_tr("Librerie"),lab_tr("Integrazioni")};
    const int order[]={4,6,0,1,2,3,7,5};for(int slot=0;slot<8;slot++){int i=order[slot];settingsTabs[i]=(Rectangle){x,y+slot*43,122,36};if(Button(settingsTabs[i],tabs[i],settingsTab==i,!applying)){settingsTab=i;settingsScroll=settingsScrollDraw=0;choiceOpen=-1;displayDragging=-1;SetRecording(false);}}
    x+=140;w-=140;Rectangle body={x,y-3,w+3,area.height-21};Rectangle saved=inputClip;inputClip=body;
    if(!choiceWheel&&displayDragging<0&&Hot(body,true))settingsScroll=Clamp(settingsScroll-ScrollInput(body)*48,0,fmaxf(0,settingsExtent-body.height));choiceWheel=false;
    settingsScrollDraw=SmoothScroll(settingsScrollDraw,settingsScroll);BeginScissorMode(body.x,body.y,body.width,body.height);y=body.y+6-settingsScrollDraw;
    if(settingsTab==0)y+=DisplayEditor(x,y,w);
    else if(settingsTab==1){
        const char* names[]={"15 FPS","30 FPS","60 FPS","90 FPS","120 FPS"};int values[]={15,30,60,90,120},at=2;for(int i=0;i<5;i++)if(lab_option("fps")==values[i])at=i;
        int next=Choice(10,x,&y,w,lab_tr("Fotogrammi al secondo"),names,5,at);if(next!=at)lab_set_option("fps",values[next]);
        const char* pauseNames[]={lab_tr("Continua a riprodurre"),lab_tr("Pausa a schermo intero"),lab_tr("Pausa solo se attivo")};int pause=lab_option("fullscreen_policy");next=Choice(16,x,&y,w,lab_tr("App a schermo intero"),pauseNames,3,pause);if(next!=pause)lab_set_option("fullscreen_policy",next);
        const char* keys[]={"muted","disable_audio_processing","disable_mouse","disable_parallax","disable_particles"};
        const char* labels[]={lab_tr("Audio dello sfondo"),lab_tr("Reazione alla musica"),lab_tr("Interazione con il mouse"),lab_tr("Parallasse"),lab_tr("Particelle")};
        for(int i=0;i<5;i++){int current=!lab_option(keys[i]),value=BoolChoice(11+i,x,&y,w,labels[i],current);if(current!=value)lab_set_option(keys[i],!value);}
        y+=TextLines(lab_tr("Modifiche salvate · usate al prossimo Applica"),x,y+8,w,14,3,muted)+20;
    }else if(settingsTab==2){
        TransitionControls(x,&y,w);y+=20;
        UiText(lab_tr("Shortcut del carousel"),x,y,18,AppTheme.text);y+=30;ShortcutInline(x,y,w);y+=50;
        int current=lab_shortcut_enabled();int value=BoolChoice(21,x,&y,w,lab_tr("Shortcut attivo"),current);if(value!=current)lab_set_shortcut(lab_shortcut(),value);
    }else if(settingsTab==3){
        const char* groups[LAB_COLLECTION_COUNT];for(int n=0;n<LAB_COLLECTION_COUNT;n++)groups[n]=lab_tr(labCollections[n]);int family=lab_option("theme_collection");int group=Choice(38,x,&y,w,lab_tr("Collezione"),groups,LAB_COLLECTION_COUNT,family);if(group!=family)lab_set_option("theme_collection",group);
        const char* themes[LAB_THEME_COUNT];int ids[LAB_THEME_COUNT],n=0,at=0;for(int i=0;i<LAB_THEME_COUNT;i++)if(labThemes[i].collection==lab_option("theme_collection")){ids[n]=i;themes[n]=labThemes[i].name;if(i==lab_option("ui_theme"))at=n;n++;}
        int value=Choice(30,x,&y,w,lab_tr("Temi"),themes,n,at);if(value!=at)lab_set_option("ui_theme",ids[value]);
        if(uiSliderDrag!=0)opacityDraft=lab_option("window_transparency")/100.f;
        if(EntrySlider(x,&y,w,lab_tr("Trasparenza della UI"),&opacityDraft))lab_set_option("window_transparency",roundf(opacityDraft*100));
        y+=TextLines(lab_tr("0% · opaca · immagini e scene restano opache"),x,y,w,12,2,muted)+14;
        int current;
        const char* densities[]={lab_tr("Comodo"),lab_tr("Compatto"),lab_tr("Galleria")};current=lab_option("layout_density");value=Choice(32,x,&y,w,lab_tr("Layout della libreria"),densities,3,current);if(current!=value)lab_set_option("layout_density",value);
        const char* fonts[96];for(int i=0;i<lab_font_count();i++)fonts[i]=i?lab_font_name(i):lab_tr("Predefinito");current=lab_font_selected();fontControl=(Rectangle){x+w-fminf(292,w*.6f),y,fminf(292,w*.6f),34};value=Choice(33,x,&y,w,lab_tr("Font dell'interfaccia"),fonts,lab_font_count(),current);fontMenu=choiceOpen==33;if(current!=value){lab_set_font(value);for(int i=0;i<96;i++)if(fontPreview[i].texture.id){UnloadFont(fontPreview[i]);fontPreview[i]=(Font){0};}}
        const char* weights[]={lab_tr("Medio"),lab_tr("Semigrassetto"),lab_tr("Grassetto")};current=lab_option("font_weight");value=Choice(34,x,&y,w,lab_tr("Peso del testo"),weights,3,current);if(current!=value)lab_set_option("font_weight",value);
        y+=TextLines(lab_tr("Le modifiche si applicano a tutta l'interfaccia e al carousel."),x,y+8,w,14,3,muted)+22;
    }else if(settingsTab==4){
        const char* languages[32];for(int i=0;i<lab_language_count();i++)languages[i]=lab_language_name(i);int current=lab_option("language");languageControl=(Rectangle){x+w-fminf(292,w*.6f),y,fminf(292,w*.6f),34};int value=Choice(40,x,&y,w,lab_tr("Lingua"),languages,lab_language_count(),current);languageMenu=choiceOpen==40;if(current!=value)lab_set_option("language",value);
        current=lab_option("reduce_motion");value=BoolChoice(41,x,&y,w,lab_tr("Riduci animazioni"),current);if(current!=value)lab_set_option("reduce_motion",value);
        const char* logic[]={lab_tr("Tutti i criteri (AND)"),lab_tr("Almeno un criterio (OR)")};current=lab_option("filter_any");value=Choice(42,x,&y,w,lab_tr("Filtri della libreria"),logic,2,current);if(current!=value)lab_set_option("filter_any",value);
        y+=TextLines("Ctrl + L",x,y+12,w,15,2,muted)+12;
        y+=TextLines(lab_tr("Cerca subito · Ctrl+A seleziona · Ctrl+C copia · Ctrl+V incolla"),x,y,w,14,3,muted)+24;
    }else if(settingsTab==6){
        UiText(lab_tr("Librerie e file locali"),x,y,19,AppTheme.text);y+=32;
        y+=TextLines(lab_tr("Importa immagini e video. Gli originali restano al loro posto."),x,y,w,13,3,muted)+14;
        if(ActionGlyphButton((Rectangle){x,y,w,34},UI_IMPORT,!applying&&!lab_files_busy(),lab_tr("Importa file")))lab_files_start(0);y+=49;
        for(int local=0;local<2;local++){UiText(lab_tr(local?"Sfondi locali":"Workshop Steam"),x,y,15,AppTheme.text);y+=25;y+=TextLines(lab_library_path(local),x,y,w,12,3,muted)+10;if(Button((Rectangle){x,y,w,32},lab_tr("Scegli cartella"),false,!applying&&!lab_files_busy()))lab_files_start(local?2:1);y+=48;}
        y+=TextLines(lab_tr("Cambiare cartella non sposta né elimina i file esistenti."),x,y,w,12,3,muted)+16;
    }else if(settingsTab==7){
        UiText("Caelestia",x,y,20,AppTheme.text);y+=34;
        int current=lab_option("caelestia_enabled"),value=BoolChoice(73,x,&y,w,lab_tr("Colori dal wallpaper"),current);if(current!=value)lab_set_option("caelestia_enabled",value);
        const char* palettes[]={lab_tr("Vivace"),lab_tr("Toni morbidi"),lab_tr("Espressiva"),lab_tr("Fedele al wallpaper")};int palette=lab_option("caelestia_palette"),next=Choice(78,x,&y,w,lab_tr("Palette dinamica"),palettes,4,palette);if(next!=palette)lab_set_option("caelestia_palette",next);
        const char* modes[]={lab_tr("Automatica"),lab_tr("Scura"),lab_tr("Chiara")};int mode=lab_option("caelestia_mode");next=Choice(79,x,&y,w,lab_tr("Luminosità della palette"),modes,3,mode);if(next!=mode)lab_set_option("caelestia_mode",next);
        y+=TextLines(lab_tr("L’attivazione prepara solo i componenti colore opzionali. La shell Caelestia resta separata."),x,y,w,12,4,muted)+14;
        y+=TextLines(lab_tr("Aggiorna barre e widget di Caelestia dai colori dell’anteprima. Non cambia le altre applicazioni."),x,y,w,13,4,muted)+18;
        y+=TextLines(lab_caelestia_available()?lab_caelestia_status():lab_tr("Caelestia non rilevato"),x,y,w,12,4,muted)+14;
    }else{
        UiText("Steam Workshop",x,y,22,AppTheme.text);y+=40;
        y+=TextLines(lab_tr("I download usano Steam e restano sottoscritti per ricevere gli aggiornamenti. La libreria si aggiorna al termine."),x,y,w,15,5,muted)+24;
        int current=lab_option("workshop_mature"),value=BoolChoice(50,x,&y,w,lab_tr("Contenuti per adulti"),current);if(current!=value)lab_set_option("workshop_mature",value);
        y+=TextLines(lab_tr("Steam deve essere aperto con un account che possiede Wallpaper Engine."),x,y+10,w,14,3,muted)+22;
    }
    settingsExtent=fmaxf(250,y+settingsScrollDraw-body.y+20);EndScissorMode();inputClip=saved;
    settingsScroll=Clamp(settingsScroll,0,fmaxf(0,settingsExtent-body.height));
    if(settingsExtent>body.height){float height=fmaxf(24,body.height*body.height/settingsExtent);DrawRectangleRounded((Rectangle){body.x+body.width-3,body.y+settingsScrollDraw/(settingsExtent-body.height)*(body.height-height),3,height},1,8,muted);}
}
static void FilterRow(int x,int y,const char* title,const char** labels,int count,int* selected,int available){
    UiText(title,x,y+10,14,muted);float px=x+100;
    for(int i=0;i<count;i++){int width=MeasureTextEx(buttonFont,labels[i],15,0).x+28;Rectangle r={px,y,width,34};
        if(r.x+r.width>x+available)break;
        if(Button(r,labels[i],*selected==i,!applying)){*selected=i;g_scroll.y=g_smoothScrollY=0;}px+=width+8;
    }
}
static int ActiveResolutionCodes(int* codes){int n=0;for(int i=1;i<64;i++)if(selectedResolutions[i])codes[n++]=i;return n;}
static void FiltersPanel(Rectangle area){
 DrawRectangleRec(area,surface);Rectangle old=inputClip;inputClip=area;if(Hot(area,true))filterScroll=fmaxf(0,filterScroll-ScrollInput(area)*48);filterScrollDraw=SmoothScroll(filterScrollDraw,filterScroll);BeginScissorMode(area.x,area.y,area.width,area.height);int x=area.x+16,y=area.y+18-filterScrollDraw,w=area.width-32;
 UiText(lab_tr("Filtri"),x,y,20,AppTheme.text);y+=36;
 if(Button((Rectangle){x,y,w,32},lab_tr("Azzera filtri"),true,true)){filterType=filterAudio=filterSource=0;memset(selectedResolutions,0,sizeof(selectedResolutions));memset(wholeResGroups,0,sizeof(wholeResGroups));}y+=44;
 const char* logic[]={lab_tr("Tutti i criteri (AND)"),lab_tr("Almeno un criterio (OR)")};int at=lab_option("filter_any"),next=Choice(54,x,&y,w,"AND / OR",logic,2,at);if(next!=at)lab_set_option("filter_any",next);
 const char* sources[]={lab_tr("Tutti"),"Steam",lab_tr("Locale")};filterSource=Choice(58,x,&y,w,lab_tr("Origine"),sources,3,filterSource);
 const char* types[]={lab_tr("Tutti"),lab_tr("Scene"),lab_tr("Video"),lab_tr("Web"),lab_tr("Preset"),lab_tr("Immagine")};filterType=Choice(51,x,&y,w,lab_tr("Tipo di sfondo"),types,6,filterType);
 const char* audio[]={lab_tr("Tutti"),lab_tr("Reattivi"),lab_tr("Non reattivi"),lab_tr("Da verificare")};filterAudio=Choice(52,x,&y,w,lab_tr("Audio"),audio,4,filterAudio);y+=14;
 UiText(lab_tr("Risoluzioni"),x,y,18,AppTheme.text);y+=30;
 const char* labels[64];int codes[64],groups[64],count=0;bool selected[64];for(int n=0;n<lab_resolution_count();n++){int code=lab_resolution_code(n);if(code>0&&code<64){codes[count]=code;groups[count]=lab_resolution_group(n);labels[count]=lab_resolution_label(n);selected[count]=selectedResolutions[code];count++;}}
 ResolutionPicker(x,&y,w,labels,groups,selected,count,wholeResGroups);for(int n=0;n<count;n++)selectedResolutions[codes[n]]=selected[n];float total=y+filterScrollDraw-area.y+20;filterScroll=Clamp(filterScroll,0,fmaxf(0,total-area.height));EndScissorMode();inputClip=old;
 if(total>area.height){float h=fmaxf(20,area.height*area.height/total);DrawRectangle(area.x+area.width-4,area.y+filterScrollDraw/(total-area.height)*(area.height-h),3,h,muted);}
}
static int PageFooter(Rectangle area,int current,int pages,bool busy){
 int y=area.y+area.height-40;Rectangle left={area.x+12,y,32,32},right={area.x+area.width-44,y,32,32};int change=0;
 if(GlyphButton(left,UI_LEFT,false,!busy&&current>1,""))change=-1;if(GlyphButton(right,UI_RIGHT,false,!busy&&current<pages,""))change=1;
 int per=lab_option("page_size"),at=per==50?0:per==200?2:1;char labels[3][80];const char* options[3];for(int n=0;n<3;n++){snprintf(labels[n],80,area.width<420?"%d":"%d %s",n==0?50:n==1?100:200,lab_tr("per pagina"));options[n]=labels[n];}
 int cy=y;int fw=area.width<420?114:234,cx=area.x+area.width-52-fw;int v=Choice(57,cx,&cy,fw,"",options,3,at);if(v!=at){lab_set_option("page_size",v==0?50:v==1?100:200);change=2;}
 const char* label=area.width<470?TextFormat("%d / %d",current,pages):TextFormat(lab_tr("Pagina %d di %d"),current,pages);UiText(label,area.x+54,y+9,13,muted);return change;
}
static float CompactGlyphWidth(const char* label){return MeasureTextEx(buttonFont,label,15,0).x+54;}
static float ShortcutInline(float x,float y,float available){
    const char* label=lab_tr(recordingKey?lab_tr("Inserisci shortcut"):lab_tr("Shortcut"));float width=CompactGlyphWidth(label);
    shortcutControl=(Rectangle){x,y,width,36};Color old=AppTheme.border;if(recordingKey)AppTheme.border=(Color){113,223,158,255};
    if(recordingKey)ThemePanel((Rectangle){x,y,width,36},.22f,8,(Color){26,89,55,255});
    if(GlyphButton((Rectangle){x,y,width,36},UI_KEY,recordingKey,!applying,label)){SetRecording(!recordingKey);isSearching=false;notice[0]=0;}
    if(recordingKey){DrawRectangleRoundedLinesEx((Rectangle){x,y,width,36},.22f,8,1.5f,AppTheme.border);}
    AppTheme.border=old;TextLines(lab_shortcut(),x+width+12,y+9,fmaxf(40,available-width-12),14,2,muted);return width;
}
static void SwitcherBar(int sw){
    int y=154;TransitionControls(28,&y,700);ShortcutInline(760,154,sw-780);

}
static void SearchBar(Rectangle r){
    bool hot=Hot(r,!applying);ThemePanel(r,0.27f,12,control);
    DrawRectangleRoundedLinesEx(r,0.27f,12,1,isSearching?AppTheme.border:lineColor);
    Icon(UI_SEARCH,(Vector2){r.x+23,r.y+r.height/2},muted,false);
    if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))FocusSearch(false);
    if(hot)SetMouseCursor(MOUSE_CURSOR_IBEAM);
    if(searchBufferCount||isSearching)EditDraw(&searchEdit,searchBuffer,(Rectangle){r.x+44,r.y+5,r.width-(page==3?194:140),r.height-10},17,isSearching);
    else UiText(lab_tr("Cerca uno sfondo..."),r.x+44,r.y+13,17,muted);
    UiText("Ctrl + L",r.x+r.width-(page==3?178:96),r.y+16,12,muted);
    if(searchBufferCount&&GlyphButton((Rectangle){r.x+r.width-(page==3?112:36),r.y+7,30,30},UI_CLOSE,false,true,"")){searchBufferCount=0;searchBuffer[0]=0;g_scroll.y=0;}
}
#include "workshop_ui.h"
static void Header(int sw,int top,int count){
    DrawRectangle(0,0,sw,top,AppTheme.bg);
    Icon(UI_LOGO,(Vector2){44,40},AppTheme.text,false);
    DrawTextEx(brandFont,"Wallpaper Lab",(Vector2){72,21},27,0,AppTheme.text);
    float navX=fmaxf(285,72+MeasureTextEx(brandFont,"Wallpaper Lab",27,0).x+30);const char* navNames[]={lab_tr("Libreria"),lab_tr("Carousel"),"Workshop"};UiIcon navIcons[]={UI_LIBRARY,UI_CAROUSEL,UI_DOWNLOAD};
    for(int i=0;i<3;i++){int navPage=i==2?3:i;float width=MeasureTextEx(buttonFont,lab_tr(navNames[i]),15,0).x+62;if(i==0)headerLibrary=(Rectangle){navX,22,width,38};else if(i==1)headerCarousel=(Rectangle){navX,22,width,38};else headerWorkshop=(Rectangle){navX,22,width,38};if(GlyphButton((Rectangle){navX,22,width,38},navIcons[i],page==navPage,!applying,lab_tr(navNames[i])))ChangePage(navPage);navX+=width+8;}
    headerSettings=(Rectangle){navX,22,40,38};if(GlyphButton(headerSettings,UI_GEAR,settingsOpen||page==2,!applying,"")){settingsOpen=!settingsOpen;choiceOpen=-1;settingsScroll=settingsScrollDraw=0;SetRecording(false);}
    UiText(page==0?lab_tr("Libreria"):page==1?lab_tr("Carousel"):page==3?"Workshop":lab_tr("Impostazioni"),28,94,23,AppTheme.text);
    UiText(page==2?lab_tr("Tutto in un posto"):TextFormat(lab_tr("%d sfondi"),page==3?(int)lab_ws_number(-1,"total"):libraryTotal),28,126,13,muted);
    if(page!=2){
        int width=fminf(page==3?440:530,fmaxf(200,sw-(page==3?610:520)));Rectangle search={fmaxf(190,(sw-width)/2.f),89,width,46};SearchBar(search);
        Rectangle filter={search.x-52,89,44,46},moon={search.x+search.width+8,89,44,46},refresh={search.x+search.width+8,89,44,46},display={search.x+search.width+60,89,44,46};
        headerImport=(Rectangle){search.x+search.width+112,89,44,46};if(page!=3&&GlyphButton(headerImport,UI_IMPORT,false,!applying&&!scanning&&!lab_files_busy(),""))lab_files_start(0);
        headerFilter=filter;headerMoon=moon;headerRefresh=refresh;headerDisplay=display;
        if(GlyphButton(filter,UI_FILTER,filtersOpen,!scanning,"")){filtersOpen=!filtersOpen;g_scroll.y=g_smoothScrollY=0;}
        headerMoon=(Rectangle){0};
        if(GlyphButton(display,UI_DISPLAY,displayPopup,!applying&&!scanning,"")){displayPopup=true;displaySelected=lab_screen();choiceOpen=-1;displayPopupScroll=displayPopupScrollDraw=0;displayDragging=-1;SetRecording(false);}
        if(GlyphButton(refresh,UI_REFRESH,scanning||(page==3&&lab_ws_number(-1,"busy")),!applying&&!scanning&&!(page==3&&lab_ws_number(-1,"busy")),"")){if(page==3)WorkshopSearch(wsPage);else StartLibraryScan();}
        if(page==3){wsSearchButton=(Rectangle){search.x+search.width-81,94,74,36};if(Button(wsSearchButton,lab_tr("Cerca"),false,!lab_ws_number(-1,"busy")))WorkshopSearch(1);}
        if(page==1)SwitcherBar(sw);

        const char* hint=page!=3&&Hot(headerImport,true)?lab_tr("Importa immagini o video"):Hot(filter,true)?lab_tr("Filtri della libreria"):Hot(refresh,true)?lab_tr("Rileggi gli sfondi scaricati da Steam"):Hot(display,true)?lab_tr("Disposizione dei monitor"):NULL;
        if(!hint&&page==3)hint=lab_ws_number(-1,"busy")?lab_tr("Ricerca su Steam..."):wsFiltersDirty?lab_tr("Filtri modificati · premi Cerca"):NULL;
        if(hint)UiText(lab_tr(hint),search.x+search.width-UiMeasure(lab_tr(hint),12),142,12,muted);

    }
    DrawLine(28,top-1,sw-28,top-1,lineColor);
}
static void Grid(Rectangle area,int count,int* indices,float dt){
    int pad=20,gap=22;float desired=Clamp((lab_option("layout_density")==1?220:lab_option("layout_density")==2?300:246)*masterScale,210,320);
    LibraryGridLayout layout=library_grid_layout(area.width,desired);int cols=layout.columns;float tile=layout.tile;
    float rowHeight=tile+(lab_option("layout_density")==1?100:112),maxScroll=fmaxf(0,ceilf(count/(float)cols)*rowHeight+24-area.height);
    if(Hot(area,true)){
        float wheel=ScrollInput(area);
        if(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL))masterScale=Clamp(masterScale+wheel*0.05f,0.7f,1.5f);
        else g_scroll.y=Clamp(g_scroll.y-wheel*80,0,maxScroll);
    }
    g_scroll.y=Clamp(g_scroll.y,0,maxScroll);float previousScroll=g_smoothScrollY;g_smoothScrollY=Lerp(g_smoothScrollY,g_scroll.y,1-expf(-dt*15));
    Rectangle saved=inputClip;inputClip=area;BeginScissorMode(area.x,area.y,area.width,area.height);
    if(!count){
        Icon(UI_SEARCH,(Vector2){area.x+area.width/2,area.y+100},muted,false);
        TextLines(page==1?lab_tr("Nessuno sfondo nel carousel con questi filtri."):lab_tr("Nessuno sfondo corrisponde alla ricerca."),area.x+48,area.y+140,area.width-96,20,3,AppTheme.text);
    }
    for(int n=0;n<count;n++){
        int i=indices[n];Vector2 target={area.x+pad+(n%cols)*(tile+gap),area.y+20+(n/cols)*rowHeight-g_smoothScrollY};
        wallpapers[i].animPos.y+=previousScroll-g_smoothScrollY;
        float f=1-expf(-dt*18);wallpapers[i].animPos=Vector2Lerp(wallpapers[i].animPos,target,f);wallpapers[i].animSize=Vector2Lerp(wallpapers[i].animSize,(Vector2){tile,tile},f);
        Rectangle r={wallpapers[i].animPos.x,wallpapers[i].animPos.y,wallpapers[i].animSize.x,wallpapers[i].animSize.y};
        libraryCards[i]=r;
        if(r.y+r.height+72<area.y||r.y>area.y+area.height)continue;
        bool hot=Hot((Rectangle){r.x,r.y,r.width,r.height+65},true);
        ThemePanel((Rectangle){r.x-6,r.y-6,r.width+12,r.height+99},0.035f,8,surface);
        DrawRectangleRoundedLinesEx((Rectangle){r.x-6,r.y-6,r.width+12,r.height+99},0.035f,8,1,lineColor);
        DrawRectangleRec(r,BLACK);if(atomic_load(&wallpapers[i].loaded))Cover(wallpapers[i].texture,r,false);else Icon(UI_LOGO,(Vector2){r.x+r.width/2,r.y+r.height/2},muted,false);
        DrawRectangleLinesEx(r,detailOpen&&i==selected_index?2.5f:1.3f,detailOpen&&i==selected_index?AppTheme.border:Fade(WHITE,hot?1:0.75f));
        const char* origin=!strcmp(lab_origin(i),"local")?lab_tr("Locale"):"Steam";float tagW=UiMeasure(origin,11)+14;ThemePanel((Rectangle){r.x+r.width-tagW-7,r.y+r.height-27,tagW,20},.2f,6,(Color){18,22,28,230});UiText(origin,r.x+r.width-tagW,r.y+r.height-23,11,WHITE);
        TextLines(lab_title(i),r.x,r.y+r.height+10,r.width,18,2,AppTheme.text);
        UiText(TypeLabel(i),r.x,r.y+r.height+66,14,muted);
        if(lab_width(i)>0)UiText(TextFormat("%d × %d",lab_width(i),lab_height(i)),r.x+r.width-UiMeasure(TextFormat("%d × %d",lab_width(i),lab_height(i)),14),r.y+r.height+66,14,muted);
        Rectangle fav={r.x+r.width-40,r.y+8,32,32},trash={r.x+8,r.y+8,32,32};
        iconIndex=i;
        bool favoriteClick=GlyphButton(fav,UI_STAR,lab_favorite(i),!applying,"");
        iconIndex=-1;
        // All downloaded projects expose the same recoverable delete action.
        bool trashClick=GlyphButton(trash,UI_TRASH,false,!applying,"");
        if(lab_current()==i){Vector2 dot={r.x+14,r.y+r.height-14};DrawCircleV(dot,6.5f,Fade(BLACK,.65f));DrawCircleV(dot,3.8f,WHITE);}
        if(favoriteClick)ToggleStar(i);
        else if(trashClick){RequestDelete(i);isSearching=false;}
        else if(hot&&!CheckCollisionPointRec(GetMousePosition(),fav)&&!CheckCollisionPointRec(GetMousePosition(),trash)){
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))SelectItem(i,n);
            if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))SelectItem(i,n);
        }
    }
    EndScissorMode();inputClip=saved;
    if(maxScroll>0){
        Rectangle track={area.x+area.width-10,area.y+16,4,area.height-32};float thumb=fmaxf(36,track.height*area.height/(area.height+maxScroll));
        if(Hot((Rectangle){track.x-5,track.y,14,track.height},true)&&IsMouseButtonDown(MOUSE_BUTTON_LEFT))g_scroll.y=Clamp((GetMouseY()-track.y-thumb/2)/(track.height-thumb),0,1)*maxScroll;
        ThemePanel(track,1,8,control);ThemePanel((Rectangle){track.x,track.y+g_smoothScrollY/maxScroll*(track.height-thumb),4,thumb},1,8,muted);
    }
}
#include "placement_editor.h"
#include "project_controls.h"
#include "wallpaper_controls.h"

static bool inspectorSections[4]={true,true,false,false};static Rectangle inspectorSectionRows[4];
static bool InspectorSection(int section,int x,int* y,int w,const char* label){
 Rectangle row={x,*y,w,32};inspectorSectionRows[section]=row;ThemePanel(row,.12f,8,control);UiText(label,x+10,*y+9,14,AppTheme.text);UiText(inspectorSections[section]?"−":"+",x+w-22,*y+8,15,muted);if(Hot(row,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))inspectorSections[section]=!inspectorSections[section];*y+=inspectorSections[section]?44:40;return inspectorSections[section];
}
static void Inspector(Rectangle area){
    if(selected_index<0||deleted[selected_index])return;
    int i=selected_index;PlacementSync(i);float actual=fmaxf(280,area.width);
    Rectangle panel={area.x,area.y,actual,area.height};
    DrawRectangleRec(panel,surface);DrawLine(panel.x,panel.y,panel.x,panel.y+panel.height,lineColor);
    int x=panel.x+22,w=actual-44;
    UiText(lab_tr("Sfondo selezionato"),x,panel.y+22,15,muted);
    if(GlyphButton((Rectangle){panel.x+actual-50,panel.y+12,32,32},UI_CLOSE,false,true,""))detailOpen=false;
    int actions=panel.y+panel.height-108;
    Rectangle body={panel.x+1,panel.y+58,actual-2,fmaxf(75,actions-panel.y-68)};Rectangle saved=inputClip;inputClip=body;
    float previewHeight=Clamp(fminf(body.height*0.45f,w*9.f/16),100,320);
    if(Hot(body,true))detailScroll=fmaxf(0,detailScroll-ScrollInput(body)*55);
    detailScrollDraw=SmoothScroll(detailScrollDraw,detailScroll);
    BeginScissorMode(body.x,body.y,body.width,body.height);int y=body.y+4-detailScrollDraw;
    Rectangle preview={x,y,w,previewHeight};inspectorPreviewRect=preview;ThemePanel(preview,0.04f,8,control);
    if(liveItem!=i){if(liveTexture.id)UnloadTexture(liveTexture);liveTexture=(Texture2D){0};liveItem=i;liveLastFrame=0;}
    static unsigned char livePixels[640*360*3];int fw=0,fh=0;
    if((!uiSmoke||getenv("WALLPAPER_LAB_TEST_APPLY"))&&lab_live_frame(i,livePixels,sizeof(livePixels),&fw,&fh)){
        if(liveTexture.id&&(liveTexture.width!=fw||liveTexture.height!=fh)){UnloadTexture(liveTexture);liveTexture=(Texture2D){0};}
        if(!liveTexture.id){Image frame={livePixels,fw,fh,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8};liveTexture=LoadTextureFromImage(frame);SetTextureFilter(liveTexture,TEXTURE_FILTER_BILINEAR);}else UpdateTexture(liveTexture,livePixels);
        liveLastFrame=GetTime();
    }
    if(liveTexture.id&&GetTime()-liveLastFrame<3)Cover(liveTexture,preview,false);
    else{Icon(UI_LIVE,(Vector2){preview.x+preview.width/2,preview.y+preview.height/2-12},muted,false);const char* state=*lab_reason(i)?lab_tr("Scena non supportata"):applying?lab_tr("Caricamento scena..."):lab_tr("In attesa della scena live");UiText(state,preview.x+12,preview.y+preview.height/2+14,14,muted);}
    const char* badge=lab_screen()<lab_monitor_count()?TextFormat("LIVE · %s",lab_monitor_name(lab_screen())):"LIVE";
    ThemePanel((Rectangle){x+8,y+8,UiMeasure(badge,12)+24,24},0.3f,6,Fade(BLACK,0.7f));UiText(badge,x+20,y+14,12,WHITE);
    y+=previewHeight+18;
    y+=TextLines(lab_title(i),x,y,w,22,3,AppTheme.text)+12;
    UiText(TextFormat("%s · %s",TypeLabel(i),ResolutionLabel(i)),x,y,14,muted);y+=27;
    if(lab_supported_count(i)>1)y+=TextLines(TextFormat(lab_tr("Risoluzioni dichiarate: %s"),lab_supported_resolutions(i)),x,y,w,13,3,muted)+10;
    const char* source=!strcmp(lab_origin(i),"local")?lab_tr("Locale"):"Steam";UiText(source,x,y,12,AppTheme.border);y+=28;
    if(InspectorSection(0,x,&y,w,lab_tr("Display di destinazione"))){
        if(lab_mode()==2&&lab_monitor_count())TextLines(lab_monitor(lab_screen()),x,y,w,13,2,muted);else TextLines(lab_mode()==0?lab_tr("Tutti i display · Panoramico"):lab_tr("Tutti i display · Duplicato"),x,y,w,12,2,muted);y+=28;
        int next;
        int variant=lab_variant_selected(i),variants=fminf(94,lab_variant_count(i));const char* options[96];options[0]=lab_tr("Originale / automatica");for(int v=0;v<variants;v++)options[v+1]=lab_variant_label(i,v);choiceEnabled=variants>0;next=Choice(70,x,&y,w,lab_tr("Risoluzione dello sfondo"),options,variants+1,variant+1);choiceEnabled=true;if(next!=variant+1&&lab_variant_set(i,next-1)&&!uiSmoke)applying=lab_start(i,0);y+=10;
    }
    if(InspectorSection(1,x,&y,w,lab_tr("Immagine e ritaglio"))){y+=ImageControls(i,x,y,w);y+=PlacementControls(i,x,y,w)+12;}
    if(InspectorSection(2,x,&y,w,lab_tr("Riproduzione"))){UiText(TextFormat("%d FPS",lab_option("fps")),x,y,12,muted);y+=25;y+=WallpaperPlayback(i,x,y,w)+10;}
    if(lab_property_count(i)&&InspectorSection(3,x,&y,w,lab_tr("Impostazioni del progetto"))){y+=CreatorControls(i,x,y,w)+12;}
    if(page==1){
        if(Button((Rectangle){x,y,(w-8)/2,36},lab_tr("Precedente"),false,!applying))applying=lab_next(-1);
        if(Button((Rectangle){x+(w+8)/2,y,(w-8)/2,36},lab_tr("Successivo"),false,!applying))applying=lab_next(1);y+=46;
    }
    if(page==1&&lab_favorite(i)){
        if(Button((Rectangle){x,y,(w-8)/2,36},lab_tr("Sposta prima"),false,!applying))lab_favorite_change(i,-1);
        if(Button((Rectangle){x+(w+8)/2,y,(w-8)/2,36},lab_tr("Sposta dopo"),false,!applying))lab_favorite_change(i,1);y+=52;
    }

    y+=CreatorReset(i,x,y,w);
    if(*lab_reason(i))y+=TextLines(lab_reason(i),x,y,w,14,4,(Color){228,134,133,255});
    float maxScroll=fmaxf(0,y+20+detailScrollDraw-body.y-body.height);detailScroll=Clamp(detailScroll,0,maxScroll);
    EndScissorMode();inputClip=saved;
    if(maxScroll>0){float thumb=fmaxf(28,body.height*body.height/(body.height+maxScroll));ThemePanel((Rectangle){panel.x+actual-7,body.y+(body.height-thumb)*detailScrollDraw/maxScroll,3,thumb},1,8,muted);}
    // Keep the actions visible even on short, tiled windows.
    DrawRectangle(panel.x+1,actions-9,actual-2,118,surface);DrawLine(x,actions-9,x+w,actions-9,lineColor);
    if(GlyphButton((Rectangle){x,actions,w,42},UI_STAR,lab_favorite(i),!applying,lab_favorite(i)?lab_tr("Rimuovi dal carousel"):lab_tr("Aggiungi al carousel")))ToggleStar(i);
    actions+=50;
    if(ActionGlyphButton((Rectangle){x,actions,w,42},UI_TRASH,!applying,lab_tr("Elimina sfondo")))RequestDelete(i);
}
static void StatusBar(int sw,int sh){
    DrawRectangle(0,sh-statusHeight,sw,statusHeight,surface);DrawLine(0,sh-statusHeight,sw,sh-statusHeight,lineColor);
    int active=lab_current();
    const char* message=lab_files_busy()?lab_tr("Selezione o importazione file in corso..."):applying?lab_tr("Caricamento della scena..."):*lab_error()?lab_error():*notice?notice:active>=0?TextFormat(lab_tr("In riproduzione · %s"),lab_title(active)):lab_tr("Seleziona uno sfondo per vedere dettagli e azioni");
    TextLines(message,28,sh-27,sw-360,14,1,*lab_error()?(Color){236,133,140,255}:muted);
    TextLines(lab_status(),sw-322,sh-26,294,14,1,muted);
}
static void Modals(int sw,int sh){
    if(displayPopup){
        modalDrawing=true;DrawRectangle(0,0,sw,sh,Fade(BLACK,.64f));
        float width=fminf(720,sw-64),height=fminf(700,sh-90);Rectangle box={(sw-width)/2,(sh-height)/2,width,height};ThemePanel(box,.04f,12,surface);
        if(GlyphButton((Rectangle){box.x+width-48,box.y+12,32,32},UI_CLOSE,false,!applying,"")){displayPopup=false;displayDragging=-1;}
        Rectangle savedClip=inputClip;Rectangle body={box.x+20,box.y+18,width-40,height-36};inputClip=body;if(displayDragging<0&&CheckCollisionPointRec(GetMousePosition(),body))displayPopupScroll=Clamp(displayPopupScroll-ScrollInput(body)*48,0,fmaxf(0,displayContentHeight-body.height));displayPopupScrollDraw=SmoothScroll(displayPopupScrollDraw,displayPopupScroll);BeginScissorMode(body.x,body.y,body.width,body.height);
        DisplayEditor(body.x,body.y-displayPopupScrollDraw,body.width);EndScissorMode();inputClip=savedClip;modalDrawing=false;
    }
    if(settingsOpen){
        modalDrawing=true;DrawRectangle(0,0,sw,sh,Fade(BLACK,0.64f));
        float mw=fminf(860,sw-48),mh=fminf(650,sh-64);Rectangle box={(sw-mw)/2,(sh-mh)/2,mw,mh};ThemePanel(box,0.04f,12,surface);
        UiText(lab_tr("Impostazioni"),box.x+24,box.y+23,25,AppTheme.text);
        if(GlyphButton((Rectangle){box.x+box.width-58,box.y+15,36,36},UI_CLOSE,false,!applying,"")){settingsOpen=false;displayDragging=-1;SetRecording(false);}
        SettingsBody((Rectangle){box.x,box.y+66,box.width,box.height-125});
        int y=box.y+box.height-56;
        if(settingsTab!=0){
            TextLines(settingsTab==1?lab_tr("Modifiche salvate · usate al prossimo Applica"):lab_tr("Modifiche applicate e salvate."),box.x+24,y+12,box.width-290,14,1,muted);
            if(settingsTab==1&&Button((Rectangle){box.x+box.width-254,y,230,36},lab_tr("Applica allo sfondo attuale"),true,!applying&&lab_current()>=0))applying=lab_start(lab_current(),0);
        }
        modalDrawing=false;
    }
    if(deleteIndex>=0){
        if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&GetTime()-deleteOpenedAt>0.12)deleteReady=true;
        modalDrawing=true;DrawRectangle(0,0,sw,sh,Fade(BLACK,0.68f));
        bool deletingActive=deleteIndex==lab_current();int h=deletingActive?350:300;int w=560,x=(sw-w)/2,y=(sh-h)/2;
        ThemePanel((Rectangle){x,y,w,h},0.06f,12,surface);
        Icon(UI_TRASH,(Vector2){x+40,y+42},(Color){236,105,115,255},false);
        UiText(lab_tr("Eliminare questo sfondo?"),x+66,y+29,25,AppTheme.text);
        TextLines(lab_title(deleteIndex),x+28,y+82,w-56,20,2,AppTheme.text);
        TextLines(lab_tr("Spostamento nel cestino, recuperabile. Steam può riscaricare gli sfondi ancora sottoscritti."),x+28,y+148,w-56,16,3,muted);
        if(deletingActive)TextLines(lab_tr("Passerà al primo altro preferito disponibile, oppure al primo sfondo della libreria."),x+28,y+222,w-56,14,2,AppTheme.border);
        if(Button((Rectangle){x+28,y+h-62,150,40},lab_tr("Annulla"),false,deleteReady))deleteIndex=-1;
        Color saved=AppTheme.border;AppTheme.border=(Color){218,72,84,255};
        if(Button((Rectangle){x+204,y+h-62,328,40},lab_tr("Sposta nel cestino"),true,deleteReady)){
            int target=deleteIndex;deleteIndex=-1;
            if(lab_delete(target)){applying=true;snprintf(notice,sizeof(notice),lab_tr("Sostituzione ed eliminazione in corso..."));}
        }
        AppTheme.border=saved;modalDrawing=false;
    }
}
static void UploadImages(void){
    for(int i=0,n=0;i<wallpaper_count&&n<6;i++)if(atomic_load(&imagePending[i])){
        wallpapers[i].texture=LoadTextureFromImage(pendingImages[i]);SetTextureFilter(wallpapers[i].texture,TEXTURE_FILTER_BILINEAR);
        UnloadImage(pendingImages[i]);atomic_store(&wallpapers[i].loaded,true);atomic_store(&imagePending[i],false);n++;
    }
}
static void SearchInput(void){
    if(EditInput(&searchEdit,searchBuffer,sizeof(searchBuffer))){g_scroll.y=0;if(page==3)wsFiltersDirty=true;}
    searchBufferCount=strlen(searchBuffer);if(IsKeyPressed(KEY_ENTER))isSearching=false;
}
static void GeometryCenters(FILE* f,const Rectangle* rects,int count){fprintf(f,"[");for(int i=0;i<count;i++)fprintf(f,"%s[%.1f,%.1f]",i?",":"",rects[i].x+rects[i].width/2,rects[i].y+rects[i].height/2);fprintf(f,"]");}
static void CustomizationGeometry(FILE* f){
    fprintf(f,"},\"workshop\":{\"selected\":%d,\"count\":%d,\"busy\":%s,\"revision\":%d,\"search\":[%.1f,%.1f],\"download\":[%.1f,%.1f],\"author\":[%.1f,%.1f],\"logic\":[%.1f,%.1f],\"cards\":",wsSelected,lab_ws_count(),lab_ws_number(-1,"busy")?"true":"false",wsSeenRevision,wsSearchButton.x+wsSearchButton.width/2,wsSearchButton.y+wsSearchButton.height/2,wsDownloadButton.x+wsDownloadButton.width/2,wsDownloadButton.y+wsDownloadButton.height/2,wsAuthorControl.x+wsAuthorControl.width/2,wsAuthorControl.y+wsAuthorControl.height/2,wsLogicControl.x+wsLogicControl.width/2,wsLogicControl.y+wsLogicControl.height/2);GeometryCenters(f,wsResultCards,lab_ws_count());
    fprintf(f,",\"tags\":");GeometryCenters(f,wsTagRows,wsTagCount);
    fprintf(f,"},\"choice_open\":%d,\"choice_scroll\":%d,\"choices\":{",choiceOpen,choiceScroll);bool choiceComma=false;
    for(int n=0;n<96;n++)if(choiceControls[n].width>0){fprintf(f,"%s\"%d\":{\"control\":[%.1f,%.1f],\"options\":",choiceComma?",":"",n,choiceControls[n].x+choiceControls[n].width/2,choiceControls[n].y+choiceControls[n].height/2);GeometryCenters(f,choiceRows[n],choiceCounts[n]);fprintf(f,",\"kind\":\"%s\",\"value\":%d}",choiceToggle[n]?"toggle":"dropdown",choiceToggleValue[n]);choiceComma=true;}
    fprintf(f,"},\"header\":{");Rectangle rects[]={headerFilter,headerMoon,headerRefresh,headerDisplay,headerSettings,headerLibrary,headerCarousel,headerWorkshop,headerImport};const char* names[]={"filter","moon","refresh","display","settings","library","carousel","workshop","import"};
    for(int i=0;i<9;i++)fprintf(f,"%s\"%s\":[%.1f,%.1f]",i?",":"",names[i],rects[i].x+rects[i].width/2,rects[i].y+rects[i].height/2);
    fprintf(f,"},\"shortcut\":[%.1f,%.1f],\"settings_tabs\":",shortcutControl.x+shortcutControl.width/2,shortcutControl.y+shortcutControl.height/2);GeometryCenters(f,settingsTabs,8);
    fprintf(f,",\"preview_rect\":[%.1f,%.1f,%.1f,%.1f],\"library_cards\":[",inspectorPreviewRect.x,inspectorPreviewRect.y,inspectorPreviewRect.width,inspectorPreviewRect.height);for(int n=0;n<wallpaper_count;n++){Rectangle r=libraryCards[n];fprintf(f,"%s[%.1f,%.1f,%.1f,%.1f]",n?",":"",r.x,r.y,r.width,r.height);}fprintf(f,"]");
    fprintf(f,",\"inspector_sections\":");GeometryCenters(f,inspectorSectionRows,4);
    fprintf(f,",\"choice_list\":[%.1f,%.1f,%.1f,%.1f],\"choice_thumb\":[%.1f,%.1f,%.1f,%.1f]",choiceList.x,choiceList.y,choiceList.width,choiceList.height,choiceThumb.x,choiceThumb.y,choiceThumb.width,choiceThumb.height);
    fprintf(f,",\"creator_binary\":[");for(int n=0;n<creatorBinaryCount;n++){Rectangle r=creatorBinaryRows[n];fprintf(f,"%s{\"property\":%d,\"value\":%d,\"control\":[%.1f,%.1f]}",n?",":"",creatorBinaryProps[n],creatorBinaryValues[n],r.x+r.width-30,r.y+r.height/2);}fprintf(f,"]");
    fprintf(f,",\"mirror_toggles\":");GeometryCenters(f,mirrorToggles,2);fprintf(f,",\"image_resets\":");GeometryCenters(f,imageResetButtons,1);
    fprintf(f,",\"image_tracks\":[");for(int n=0;n<3;n++)fprintf(f,"%s[%.1f,%.1f,%.1f]",n?",":"",adjustmentTracks[n].x,adjustmentTracks[n].y,adjustmentTracks[n].width);fprintf(f,"],\"opacity_track\":[%.1f,%.1f,%.1f],\"library_total\":%d,\"library_page\":%d",opacityTrack.x,opacityTrack.y,opacityTrack.width,libraryTotal,libraryPage);
    fprintf(f,",\"resolution_groups\":");GeometryCenters(f,groupChecks,6);
    fprintf(f,",\"theme_variants\":");GeometryCenters(f,variantChoices,2);
    fprintf(f,",\"themes\":");GeometryCenters(f,themeChoices,LAB_THEME_COUNT);fprintf(f,",\"languages\":");GeometryCenters(f,languageChoices,lab_language_count());fprintf(f,",\"density\":");GeometryCenters(f,densityChoices,3);
    fprintf(f,",\"zoom_track\":[%.1f,%.1f,%.1f],\"creator_reset\":[%.1f,%.1f],\"color_swatch\":[%.1f,%.1f],\"color_box\":[%.1f,%.1f,%.1f,%.1f]",zoomTrack.x,zoomTrack.y,zoomTrack.width,creatorReset.x+creatorReset.width/2,creatorReset.y+creatorReset.height/2,creatorColorSwatch.x+creatorColorSwatch.width/2,creatorColorSwatch.y+creatorColorSwatch.height/2,creatorColorBox.x,creatorColorBox.y,creatorColorBox.width,creatorColorBox.height);
    fprintf(f,",\"color_hue\":[%.1f,%.1f,%.1f,%.1f]",creatorHue.x,creatorHue.y,creatorHue.width,creatorHue.height);
    fprintf(f,",\"display_cards\":");GeometryCenters(f,displayCards,lab_monitor_count());fprintf(f,",\"display_selected\":%d,\"screen\":%d,\"animation_controls\":",displaySelected,lab_screen());Rectangle animations[]={switcherEye,transitionEye,switcherCustomize,transitionCustomize};GeometryCenters(f,animations,4);
    fprintf(f,",\"number_tracks\":{");bool firstNumber=true;for(int id=0;id<160;id++)if(numberTracks[id].width){Rectangle r=numberTracks[id];fprintf(f,"%s\"%d\":[%.1f,%.1f,%.1f]",firstNumber?"":",",id,r.x,r.y,r.width);firstNumber=false;}fprintf(f,"}");
    fprintf(f,",\"display_save\":[%.1f,%.1f]",displaySaveButton.x+displaySaveButton.width/2,displaySaveButton.y+displaySaveButton.height/2);
    fprintf(f,",\"display_control\":[%.1f,%.1f]",headerDisplay.x+headerDisplay.width/2,headerDisplay.y+headerDisplay.height/2);
    fprintf(f,",\"language_control\":[%.1f,%.1f]",languageControl.x+languageControl.width/2,languageControl.y+languageControl.height/2);
    fprintf(f,",\"font_control\":[%.1f,%.1f],\"fonts\":",fontControl.x+fontControl.width/2,fontControl.y+fontControl.height/2);GeometryCenters(f,fontChoices,lab_font_count());
}
static Font LoadSystemFont(const char* path,int size,int* glyphs,int count){
    int latin[992];if(!glyphs){count=992;for(int i=0;i<992;i++)latin[i]=32+i;glyphs=latin;}
    return LoadFallbackFont(path,size,glyphs,count);
}
static void ReloadUiFonts(void){
    memset(glyphCache,0,sizeof(glyphCache));
    for(int i=0;i<96;i++)if(fontPreview[i].texture.id){UnloadFont(fontPreview[i]);fontPreview[i]=(Font){0};}
    int glyphs[8192],count=992;for(int i=0;i<992;i++)glyphs[i]=32+i;
    int symbols[]={0x2212,0x2026,0x2013,0x2014,0x20ac,0x2190,0x2191,0x2192,0x2193,0x2019};for(int i=0;i<10;i++)glyphs[count++]=symbols[i];
    for(int i=0;i<wallpaper_count;i++){const char* p=lab_title(i);while(*p&&count<8192){int bytes=0,cp=GetCodepointNext(p,&bytes);if(bytes<1)break;bool found=false;for(int n=0;n<count;n++)if(glyphs[n]==cp){found=true;break;}if(!found)glyphs[count++]=cp;p+=bytes;}}
    for(int i=0;i<lab_ws_count();i++)for(int field=0;field<3;field++){const char* text=lab_ws_text(i,field==0?"title":field==1?"author":"description");while(*text&&count<8192){int bytes=0,cp=GetCodepointNext(text,&bytes);if(bytes<1)break;bool found=false;for(int n=0;n<count;n++)if(glyphs[n]==cp){found=true;break;}if(!found)glyphs[count++]=cp;text+=bytes;}}
    Font old[]={uiFont,buttonFont,brandFont,fallbackFont};for(int i=0;i<4;i++)if(old[i].texture.id&&old[i].texture.id!=GetFontDefault().texture.id)UnloadFont(old[i]);
    uiFont=LoadSystemFont(lab_font(),36,glyphs,count);if(!uiFont.texture.id)uiFont=GetFontDefault();
    buttonFont=LoadSystemFont(lab_button_font(),32,glyphs,count);if(!buttonFont.texture.id)buttonFont=GetFontDefault();
    brandFont=LoadSystemFont(lab_brand_font(),40,glyphs,count);if(!brandFont.texture.id)brandFont=GetFontDefault();
    fallbackFont=LoadFallbackFont(lab_fallback_font(),36,glyphs,count);if(!fallbackFont.texture.id)fallbackFont=GetFontDefault();
    Font loaded[]={uiFont,buttonFont,brandFont,fallbackFont};for(int i=0;i<4;i++)SetTextureFilter(loaded[i].texture,TEXTURE_FILTER_BILINEAR);
    seenFontRevision=lab_font_revision();
}
static void CommitLibraryScan(pthread_t* loaders){
    for(int i=0;i<96;i++){if(fontPreview[i].texture.id&&fontPreview[i].texture.id!=GetFontDefault().texture.id)UnloadFont(fontPreview[i]);fontPreview[i]=(Font){0};}
    char selectedPath[4096]="";if(selected_index>=0&&selected_index<lab_count())snprintf(selectedPath,sizeof(selectedPath),"%s",lab_path(selected_index));
    atomic_store(&loader_running,false);for(int n=0;n<4;n++)pthread_join(loaders[n],NULL);
    for(int i=0;i<wallpaper_count;i++){if(atomic_load(&wallpapers[i].loaded))UnloadTexture(wallpapers[i].texture);if(atomic_load(&imagePending[i]))UnloadImage(pendingImages[i]);free(wallpapers[i].path);free(wallpapers[i].filename);}
    memset(wallpapers,0,sizeof(wallpapers));memset(pendingImages,0,sizeof(pendingImages));memset(deleted,0,sizeof(deleted));
    lab_rescan_commit();LoadWallpapers(NULL);selected_index=-1;
    for(int i=0;i<wallpaper_count;i++)if(!strcmp(selectedPath,lab_path(i))){selected_index=i;break;}
    if(selected_index<0)detailOpen=false;placementItem=-1;liveItem=-1;creatorItem=-1;creatorDrag=creatorCombo=creatorEdit=-1;g_hoveredIndex=selected_index;memset(starPulse,0,sizeof(starPulse));
    if(liveTexture.id){UnloadTexture(liveTexture);liveTexture=(Texture2D){0};}
    atomic_store(&next_load_index,0);atomic_store(&loader_running,true);for(int n=0;n<4;n++)pthread_create(&loaders[n],NULL,LoaderThread,NULL);
    ReloadUiFonts();snprintf(notice,sizeof(notice),lab_tr("Libreria aggiornata · %d sfondi"),wallpaper_count);
}
int selector_main(int argc,char**argv){
    LoadDefaultConfig();ParseConfigFile();g_base_thumb_size=420;g_max_threads=4;g_max_fps=90;
    SetTraceLogLevel(LOG_WARNING);LoadWallpapers(NULL);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT|FLAG_WINDOW_TRANSPARENT);InitWindow(1440,900,"Wallpaper Lab");SetWindowMinSize(1000,720);SetExitKey(KEY_NULL);SetTargetFPS(g_max_fps);
    ReloadUiFonts();InitBackdrop();InitIconAtlas();
    selected_index=lab_current();detailOpen=false;
    if(argc>1&&!strcmp(argv[1],"--display-settings")){settingsOpen=true;settingsTab=0;}
    if(argc>1&&!strcmp(argv[1],"--appearance-settings")){settingsOpen=true;settingsTab=3;}
    if(argc>1&&!strcmp(argv[1],"--language-settings")){settingsOpen=true;settingsTab=4;}
    filterSize=lab_option("filter_width")>0?lab_option("filter_width"):260;
    if(argc>1&&(!strcmp(argv[1],"--workshop")||!strcmp(argv[1],"--workshop-ui-test"))){ChangePage(3);filtersOpen=true;}
    bool uiTest=argc>1&&!strcmp(argv[1],"--ui-test");uiSmoke=uiTest||getenv("WALLPAPER_LAB_UI_READ_ONLY");inspectorSize=lab_option("inspector_width")>0?lab_option("inspector_width"):390;if(uiTest){smokeStage=0;smokeStart=GetTime();}
    pthread_t loaders[4];for(int n=0;n<4;n++)pthread_create(&loaders[n],NULL,LoaderThread,NULL);
    float refreshClock=0,stateClock=0;
    while(!WindowShouldClose()){
        if(argc>1&&!strcmp(argv[1],"--workshop-ui-test")&&GetTime()>6)break;
        if(seenFontRevision!=lab_font_revision())ReloadUiFonts();
        if(IsFileDropped()){FilePathList dropped=LoadDroppedFiles();if(!lab_files_busy()&&!applying)lab_import_files((const char* const*)dropped.paths,dropped.count);UnloadDroppedFiles(dropped);}
        int filesResult=lab_files_poll();if(filesResult>0&&!scanning){StartLibraryScan();snprintf(notice,sizeof(notice),"%s",lab_files_notice());}
        WorkshopTick();if(!scanning&&!applying&&lab_ws_installed())StartLibraryScan();
        if(scanning){int ready=lab_rescan_ready();if(ready){scanning=false;if(ready>0)CommitLibraryScan(loaders);}}
        ApplyTheme();int sw=GetScreenWidth(),sh=GetScreenHeight();float dt=fminf(GetFrameTime(),0.05f);SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        inputClip=(Rectangle){0,0,sw,sh};switcherPreviewBlocks=lab_switcher_preview_active();inputBlocked=transitionPreview||switcherPreview||animationCustomize||switcherPreviewBlocks||lab_files_busy()||displayPopup||settingsOpen||deleteIndex>=0||scanning;
        for(int i=0;i<wallpaper_count;i++)starPulse[i]=fmaxf(0,starPulse[i]-dt*2.8f);
        int completed=lab_poll();
        if(completed==1&&smokeApplyStarted)smokeApplySucceeded=true;
        if(smokeApplySucceeded)smokeFramesAfterApply++;
        if(uiTest&&getenv("WALLPAPER_LAB_TEST_APPLY")&&!smokeApplyStarted&&GetTime()-smokeStart>1){
            int target=selected_index>=0?selected_index:wallpaper_count?0:-1;
            if(target>=0){smokeApplyStarted=true;applying=lab_start(target,0);}
        }
        int removed=lab_deleted();if(removed>=0){deleted[removed]=true;if(selected_index==removed){selected_index=lab_current();detailOpen=selected_index>=0;}snprintf(notice,sizeof(notice),lab_tr("Sfondo spostato nel cestino del sistema."));}
        if(completed){applying=false;if(completed>0&&removed<0)snprintf(notice,sizeof(notice),"%s",completed==1?lab_tr("Sfondo applicato"):lab_tr("Scena live aperta in una finestra separata"));}
        refreshClock+=dt;if(refreshClock>1&&!applying){lab_refresh();refreshClock=0;}
        bool escape=IsKeyPressed(KEY_ESCAPE);
        if(lab_switcher_preview_active()&&(escape||IsMouseButtonPressed(MOUSE_BUTTON_LEFT))){lab_cancel_switcher_preview();escape=false;}
        if(escape&&(transitionPreview||switcherPreview||animationCustomize)){if(choiceOpen>=0)choiceOpen=-1;else{transitionPreview=switcherPreview=false;animationCustomize=0;}escape=false;}
        if(escape){if(choiceOpen>=0)choiceOpen=-1;else if(creatorEdit>=0)creatorEdit=-1;else if(wsAuthorEditing)wsAuthorEditing=false;else if(placementAxis>=0)placementAxis=-1;else if(recordingKey)SetRecording(false);else if(deleteIndex>=0)deleteIndex=-1;else if(displayPopup){displayPopup=false;displayDragging=-1;}else if(settingsOpen&&choiceOpen>=0)choiceOpen=-1;else if(settingsOpen)settingsOpen=false;else if(isSearching)isSearching=false;else if(detailOpen)detailOpen=false;else if(page==3&&wsSelected>=0)wsSelected=-1;else if(page!=0)ChangePage(0);}
        if(recordingKey)RecordShortcut();
        else if(!inputBlocked&&(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL))&&IsKeyPressed(KEY_L)){FocusSearch(true);creatorEdit=-1;wsAuthorEditing=false;}
        else if(isSearching&&!inputBlocked){SearchInput();if(page==3&&IsKeyPressed(KEY_ENTER))WorkshopSearch(1);}
        else if(!inputBlocked&&page!=2){
            if(IsKeyPressed(KEY_SLASH))FocusSearch(false);

        }
        int resolutionCodes[64],resolutionCount=ActiveResolutionCodes(resolutionCodes);
        int indices[MAX_POSSIBLE_WALLPAPERS],count=0,available=page==1?lab_carousel_count():wallpaper_count;
        for(int n=0;n<available;n++){int i=page==1?lab_carousel_item(n):n;if(i>=0&&!deleted[i]&&(!searchBufferCount||stristr(lab_title(i),searchBuffer))&&lab_matches_grouped(i,filterType,filterAudio,resolutionCodes,resolutionCount,(wholeResGroups[0]?1:0)|(wholeResGroups[1]?2:0)|(wholeResGroups[2]?4:0)|(wholeResGroups[3]?8:0)|(wholeResGroups[4]?16:0)|(wholeResGroups[5]?32:0),filterSource))indices[count++]=i;}
        libraryTotal=count;
        if(page==0){unsigned long hash=filterType+filterAudio*13+filterSource*67+lab_option("filter_any")*37;for(const char* p=searchBuffer;*p;p++)hash=hash*33+(unsigned char)*p;for(int n=0;n<resolutionCount;n++)hash=hash*33+resolutionCodes[n];if(hash!=libraryQuery){libraryPage=1;libraryQuery=hash;}int per=lab_option("page_size");if(!per)per=100;int pages=fmaxf(1,ceilf(count/(float)per));libraryPage=Clamp(libraryPage,1,pages);int begin=(libraryPage-1)*per,countOnPage=fminf(per,count-begin);memmove(indices,indices+begin,countOnPage*sizeof(int));count=countOnPage;}
        if(!inputBlocked&&!isSearching&&!wsAuthorEditing&&creatorEdit<0&&!recordingKey&&placementAxis<0&&page!=2&&page!=3){
            int dir=IsKeyPressed(KEY_RIGHT)?1:IsKeyPressed(KEY_LEFT)?-1:0;
            if(dir&&count){int at=-1;for(int n=0;n<count;n++)if(indices[n]==selected_index)at=n;at=(at+dir+count)%count;SelectItem(indices[at],at);}
        }
        filterWidth=filtersOpen&&page!=2?Clamp(filterSize,220,fminf(600,sw*.45f)):0;
        Rectangle filterDivider={filterWidth-6,headerHeight,12,sh-headerHeight-statusHeight};
        if(filtersOpen&&!inputBlocked&&page!=2){bool hot=CheckCollisionPointRec(GetMousePosition(),filterDivider);if(hot||filterResizing)SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))filterResizing=true;if(filterResizing){filterSize=Clamp(GetMouseX(),220,fminf(600,sw*.45f));filterWidth=filterSize;if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){filterResizing=false;lab_set_option("filter_width",filterSize);}}}
        if(filterResizing)inputBlocked=true;
        float maxInspector=fmaxf(280,fminf(sw*0.65f,sw-filterWidth-250));
        inspectorSize=Clamp(inspectorSize,280,maxInspector);
        Rectangle divider={sw-detailWidth-6,page==1?252:164,12,sh-210};
        if((page==3?wsSelected>=0:detailOpen)&&!inputBlocked&&page!=2){
            bool hot=CheckCollisionPointRec(GetMousePosition(),divider);
            if(hot||detailResizing)SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
            if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))detailResizing=true;
            if(detailResizing){inspectorSize=Clamp(sw-GetMouseX(),280,maxInspector);if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){detailResizing=false;lab_set_option("inspector_width",(int)inspectorSize);}}
        }
        if(detailResizing)inputBlocked=true;
        float target=(page==3?wsSelected>=0:detailOpen&&selected_index>=0)&&page!=2?inspectorSize:0;
        detailWidth=lab_option("reduce_motion")?target:Lerp(detailWidth,target,1-expf(-dt*15));if(fabsf(detailWidth-target)<0.5f)detailWidth=target;
        int top=page==1?252:headerHeight;
        Rectangle content={filterWidth,top,fmaxf(100,sw-detailWidth-filterWidth),sh-top-statusHeight};
        UploadImages();int transparency=uiSliderDrag==0?roundf(opacityDraft*100):lab_option("window_transparency");uiCanvasActive=transparency>0;
        if(uiCanvasActive&&(uiCanvas.texture.width!=sw||uiCanvas.texture.height!=sh)){if(uiCanvas.id)UnloadRenderTexture(uiCanvas);uiCanvas=LoadRenderTexture(sw,sh);}
        BeginDrawing();if(uiCanvasActive)BeginTextureMode(uiCanvas);rlSetBlendFactorsSeparate(RL_SRC_ALPHA,RL_ONE_MINUS_SRC_ALPHA,RL_ONE,RL_ONE_MINUS_SRC_ALPHA,RL_FUNC_ADD,RL_FUNC_ADD);BeginBlendMode(BLEND_CUSTOM_SEPARATE);ClearBackground(uiCanvasActive?BLANK:AppTheme.bg);DrawBackdrop(sw,sh);
        if(page==2)SettingsBody((Rectangle){4,headerHeight+8,sw-8,sh-headerHeight-statusHeight-16});
        else{
            if(page==3){if(filtersOpen)WorkshopFilters((Rectangle){0,top,filterWidth,sh-top-statusHeight});WorkshopGrid(content);if(detailWidth>1)WorkshopInspector((Rectangle){sw-detailWidth,top,detailWidth,sh-top-statusHeight});}
            else{if(filtersOpen)FiltersPanel((Rectangle){0,top,filterWidth,sh-top-statusHeight});Grid((Rectangle){content.x,content.y,content.width,content.height-(page==0?48:0)},count,indices,dt);if(page==0){int per=lab_option("page_size");if(!per)per=100;int pages=fmaxf(1,ceilf(libraryTotal/(float)per));int move=PageFooter(content,libraryPage,pages,false);if(move){libraryPage=move==2?1:libraryPage+move;g_scroll.y=g_smoothScrollY=0;}}if(detailWidth>1)Inspector((Rectangle){sw-detailWidth,top,detailWidth,sh-top-statusHeight});}
        }
        if(filterWidth>1)ThemePanel((Rectangle){filterWidth-2,top+(sh-top-statusHeight)/2-24,4,48},1,8,filterResizing?AppTheme.border:muted);
        if(detailWidth>1){float dx=sw-detailWidth;ThemePanel((Rectangle){dx-2,top+(sh-top-statusHeight)/2-24,4,48},1,8,detailResizing?AppTheme.border:muted);}
        Header(sw,top,count);StatusBar(sw,sh);ThemeWindowTrim(sw,sh);Modals(sw,sh);TransitionPreview();SwitcherPreview();AnimationCustomization();ChoiceOverlay();EndBlendMode();if(uiCanvasActive){EndTextureMode();ClearBackground(BLANK);BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);DrawTexturePro(uiCanvas.texture,(Rectangle){0,0,sw,-sh},(Rectangle){0,0,sw,sh},(Vector2){0},0,WHITE);EndBlendMode();}EndDrawing();
        // Opt-in state evidence for isolated desktop interaction tests.
        stateClock+=dt;const char* stateOutput=getenv("WALLPAPER_LAB_UI_STATE");
        if(stateOutput&&stateClock>0.1f){stateClock=0;char temporary[4096];snprintf(temporary,sizeof(temporary),"%s.tmp.%d",stateOutput,getpid());FILE* f=fopen(temporary,"w");if(f){fprintf(f,"{\"page\":%d,\"detail\":%s,\"selection\":%d,\"results\":%d,\"settings\":%s,\"delete_confirmation\":%s,\"recording\":%s}\n",page,detailOpen?"true":"false",selected_index,count,settingsOpen?"true":"false",deleteIndex>=0?"true":"false",recordingKey?"true":"false");fclose(f);rename(temporary,stateOutput);FILE* style=fopen(TextFormat("%s.style",stateOutput),"w");if(style){fprintf(style,"{\"search_focus\":%s,\"search_count\":%d,\"search_cursor\":%d,\"search_anchor\":%d,\"applying\":%s,",isSearching?"true":"false",searchBufferCount,searchEdit.cursor,searchEdit.anchor,applying?"true":"false");fprintf(style,"\"transition_preview\":%s,\"switcher_preview\":%s,\"customize\":%d,\"transparency\":%d,",transitionPreview?"true":"false",switcherPreview?"true":"false",animationCustomize,lab_option("window_transparency"));fprintf(style,"\"display_popup\":%s,\"light\":%s,\"fps\":%d,\"creator_ms\":%.3f,\"language_menu\":%s,",displayPopup?"true":"false",lab_option("light_theme")?"true":"false",GetFPS(),creatorTimeMs,languageMenu?"true":"false");fprintf(style,"\"theme\":%d,\"language\":%d,\"font\":%d,\"font_revision\":%d,\"settings_tab\":%d,\"scanning\":%s,\"font_menu\":%s,\"shortcut\":\"%s\"}\n",lab_option("ui_theme"),lab_option("language"),lab_font_selected(),seenFontRevision,settingsTab,scanning?"true":"false",fontMenu?"true":"false",lab_shortcut());fclose(style);}}FILE* geometry=fopen(TextFormat("%s.geometry",stateOutput),"w");if(geometry){LibraryGridLayout layout=library_grid_layout(content.width,Clamp((lab_option("layout_density")==1?220:lab_option("layout_density")==2?300:246)*masterScale,210,320));fprintf(geometry,"{\"filter_width\":%.1f,\"detail_width\":%.1f,\"tile\":%.1f,\"columns\":%d,\"groups\":{",filterWidth,detailWidth,layout.tile,layout.columns);
            for(int n=0;n<6;n++)fprintf(geometry,"%s\"%d\":[%.1f,%.1f]",n?",":"",n,resolutionHeaders[n].x+resolutionHeaders[n].width/2,resolutionHeaders[n].y+18);
            fprintf(geometry,"},\"resolutions\":{");bool comma=false;
            for(int n=0;n<64;n++)if(resolutionRows[n].width>0){fprintf(geometry,"%s\"%d\":[%.1f,%.1f]",comma?",":"",n,resolutionRows[n].x+resolutionRows[n].width/2,resolutionRows[n].y+25);comma=true;}
            CustomizationGeometry(geometry);fprintf(geometry,",\"mouse\":[%d,%d],\"drag_axis\":%d,\"offset\":[%.3f,%.3f],\"position\":[[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]],\"center\":[%.1f,%.1f]}\n",GetMouseX(),GetMouseY(),placementAxis,placementX,placementY,placementTracks[0].x,placementTracks[0].y+2,placementTracks[0].width,placementTracks[1].x,placementTracks[1].y+2,placementTracks[1].width,placementReset.x+placementReset.width/2,placementReset.y+17);fclose(geometry);}}

        if(uiTest&&!applying&&GetTime()-smokeStart>2+smokeStage*1.2){
            const char* out=getenv("WALLPAPER_LAB_TEST_OUTPUT");if(out){Image shot=LoadImageFromScreen();ExportImage(shot,TextFormat("%s/view-%d.png",out,smokeStage));UnloadImage(shot);}
            smokeStage++;
            if(smokeStage==1&&wallpaper_count)SelectItem(lab_current()>=0?lab_current():0,0);
            else if(smokeStage==2)filtersOpen=true;
            else if(smokeStage==3){filtersOpen=false;ChangePage(1);}
            else if(smokeStage==4){settingsOpen=true;settingsTab=2;}
            else if(smokeStage==5){settingsTab=0;}
            else if(smokeStage==6){settingsTab=1;}
            else if(smokeStage==7){settingsOpen=false;ChangePage(2);settingsTab=3;}
            else if(smokeStage==8){ChangePage(0);lab_set_option("light_theme",1);detailOpen=false;}
            else if(smokeStage==9&&wallpaper_count){SelectItem(0,0);}
            else if(smokeStage==10&&wallpaper_count){filtersOpen=true;resolutionGroups[0]=resolutionGroups[1]=true;}
            else if(smokeStage==11&&wallpaper_count){RequestDelete(0);}
            else break;
        }
    }
    if(uiTest&&getenv("WALLPAPER_LAB_TEST_APPLY")){
        const char* out=getenv("WALLPAPER_LAB_TEST_OUTPUT");if(out){FILE* f=fopen(TextFormat("%s/persistent-window.json",out),"w");if(f){fprintf(f,"{\"applied\":%s,\"frames_after_apply\":%d}\n",smokeApplySucceeded?"true":"false",smokeFramesAfterApply);fclose(f);}}
    }
    WorkshopCleanup();if(uiCanvas.id)UnloadRenderTexture(uiCanvas);if(transitionCanvas.id)UnloadRenderTexture(transitionCanvas);if(transitionShader.id)UnloadShader(transitionShader);for(int n=0;n<1024;n++){if(shapedCache[n].texture.id)UnloadTexture(shapedCache[n].texture);free(shapedCache[n].text);}if(backdropShader.id)UnloadShader(backdropShader);if(liveTexture.id)UnloadTexture(liveTexture);lab_shutdown();atomic_store(&loader_running,false);for(int n=0;n<4;n++)pthread_join(loaders[n],NULL);
    for(int i=0;i<wallpaper_count;i++){if(atomic_load(&wallpapers[i].loaded))UnloadTexture(wallpapers[i].texture);if(atomic_load(&imagePending[i]))UnloadImage(pendingImages[i]);free(wallpapers[i].path);free(wallpapers[i].filename);}
    for(int i=0;i<96;i++)if(fontPreview[i].texture.id&&fontPreview[i].texture.id!=GetFontDefault().texture.id)UnloadFont(fontPreview[i]);
    if(uiFont.texture.id!=GetFontDefault().texture.id)UnloadFont(uiFont);if(brandFont.texture.id!=GetFontDefault().texture.id)UnloadFont(brandFont);if(buttonFont.texture.id!=GetFontDefault().texture.id)UnloadFont(buttonFont);if(fallbackFont.texture.id!=GetFontDefault().texture.id)UnloadFont(fallbackFont);if(iconAtlasTarget.id)UnloadRenderTexture(iconAtlasTarget);CloseWindow();return smokeApplyStarted&&(!smokeApplySucceeded||smokeFramesAfterApply<30)?1:0;
}
