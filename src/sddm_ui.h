// SDDM has its own catalog, selection, image profiles and preview cache.
// It reuses the library's themed cards, grid sizing and image-control widgets.
static int sddmViewTab=0,sddmPicked[2]={-1,-1},sddmLibraryPage=0,sddmOrigin=0,sddmType=0;
static bool sddmDetails=false,sddmFilters=false,sddmFavorites=false,sddmSearching=false,sddmDirty=false,sddmSaving=false,sddmResizing=false;
static char sddmSearch[256]="",sddmEditSource[4096]="";static TextEdit sddmSearchEdit;
static float sddmScroll=0,sddmScrollDraw=0,sddmInspectorScroll=0,sddmInspectorDraw=0,sddmInspectorWidth=330,sddmScale=1;
static float sddmDraft[14];
static const char* sddmImageKeys[]={"brightness","contrast","saturation","temperature","hue","gamma","vignette","rotation","flip_x","flip_y","zoom","x","y","crop"};
static Rectangle sddmTabs[3],sddmCards[50],sddmStars[50],sddmSearchBox,sddmFilterButton,sddmSourceButtons[4],sddmTypeButton,sddmFavoritesButton,sddmSaveButton,sddmPreviewButton,sddmResetButton,sddmDetailBody,sddmPreviewRect,sddmPrevious,sddmNext,sddmCloseButton,sddmCancelButton,sddmFlipButtons[2];
static int sddmCardIndices[50],sddmCardCount=0,sddmFilteredCount=0;
typedef struct {char path[4096];Texture2D texture;double used;} SddmTexture;
static SddmTexture sddmTextures[96];static int sddmUploads=0;static Shader sddmImageShader={0};
static void SddmShutdown(void){for(int i=0;i<96;i++)if(sddmTextures[i].texture.id)UnloadTexture(sddmTextures[i].texture);if(sddmImageShader.id)UnloadShader(sddmImageShader);}
static Texture2D SddmTextureFor(int kind,int index){
 char path[4096];snprintf(path,sizeof(path),"%s",lab_sddm_text(kind,index,"thumbnail"));if(!*path)return (Texture2D){0};
 int slot=0;double oldest=1e30;
 for(int i=0;i<96;i++){if(!strcmp(path,sddmTextures[i].path)){sddmTextures[i].used=GetTime();return sddmTextures[i].texture;}if(sddmTextures[i].used<oldest){oldest=sddmTextures[i].used;slot=i;}}
 if(lab_sddm_preview_request(kind,index)!=1||sddmUploads>=1)return (Texture2D){0};sddmUploads++;
 Image image=LoadImage(path);if(!image.data)return (Texture2D){0};
 if(sddmTextures[slot].texture.id){rlDrawRenderBatchActive();UnloadTexture(sddmTextures[slot].texture);}
 sddmTextures[slot].texture=LoadTextureFromImage(image);UnloadImage(image);SetTextureFilter(sddmTextures[slot].texture,TEXTURE_FILTER_BILINEAR);
 snprintf(sddmTextures[slot].path,sizeof(sddmTextures[slot].path),"%s",path);sddmTextures[slot].used=GetTime();return sddmTextures[slot].texture;
}
static const char* SddmTypeLabel(int i){const char* type=lab_sddm_text(0,i,"kind");return !strcmp(type,"image")?lab_tr("Immagine"):!strcmp(type,"video")?lab_tr("Video"):!strcmp(type,"gif")?"GIF":!strcmp(type,"scene")?lab_tr("Scena"):lab_tr("Web");}
static const char* SddmOriginLabel(int i){const char* origin=lab_sddm_text(0,i,"origin");return !strcmp(origin,"sddm")?"SDDM":!strcmp(origin,"local")?lab_tr("Locale"):"Steam";}
static void SddmLoadDraft(int i){for(int n=0;n<14;n++)sddmDraft[n]=lab_sddm_image(i,sddmImageKeys[n]);snprintf(sddmEditSource,sizeof(sddmEditSource),"%s",lab_sddm_text(0,i,"id"));sddmDirty=false;}
static void SddmEditedPreview(Texture2D texture,Rectangle r,const float* values){
 DrawRectangleRec(r,BLACK);if(!texture.id)return;
 if(!sddmImageShader.id)sddmImageShader=LoadShader(NULL,lab_sddm_shader());
 Vector2 size={texture.width,texture.height},output={r.width,r.height};Vector3 basic={values[0],values[1],values[2]},geometry={values[7]*DEG2RAD,values[8]>.5?-1:1,values[9]>.5?-1:1},placement={values[11],values[12],values[10]};Vector4 more={values[4]*DEG2RAD,values[3],values[5],values[6]};float crop=values[13];
 SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"imageSize"),&size,SHADER_UNIFORM_VEC2);SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"outputSize"),&output,SHADER_UNIFORM_VEC2);
 SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"basic"),&basic,SHADER_UNIFORM_VEC3);SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"more"),&more,SHADER_UNIFORM_VEC4);SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"geometry"),&geometry,SHADER_UNIFORM_VEC3);SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"placement"),&placement,SHADER_UNIFORM_VEC3);SetShaderValue(sddmImageShader,GetShaderLocation(sddmImageShader,"crop"),&crop,SHADER_UNIFORM_FLOAT);
 BeginShaderMode(sddmImageShader);DrawTexturePro(texture,(Rectangle){0,0,texture.width,texture.height},r,(Vector2){0},0,WHITE);EndShaderMode();
}
static void SddmPlaceholder(Rectangle r,int kind,int index){
 ThemePanel(r,.03f,8,control);Icon(UI_LIVE,(Vector2){r.x+r.width/2,r.y+r.height/2-10},muted,false);
 const char* text=lab_sddm_preview_request(kind,index)<0?lab_tr("Anteprima non disponibile"):lab_tr("Attendi la fine del caricamento");ClippedText(text,(Rectangle){r.x+12,r.y+r.height/2+14,r.width-24,20},12,muted);
}
static void SddmPick(int kind,int index){
 if(sddmDirty&&kind==0&&index!=sddmPicked[0]){sddmDetails=true;snprintf(notice,sizeof(notice),"%s",lab_tr("Salva o annulla le modifiche prima di cambiare sfondo."));return;}
 sddmPicked[kind]=index;sddmDetails=true;sddmInspectorScroll=sddmInspectorDraw=0;sddmSearching=false;
 if(!kind&&!sddmDirty)SddmLoadDraft(index);
 if((kind||lab_sddm_supported(index))&&index!=lab_sddm_selected(kind))lab_sddm_select(kind,index);
}
static void SddmScrollBar(Rectangle area,float total,float* scroll,float drawn){
 float max=fmaxf(0,total-area.height);*scroll=Clamp(*scroll,0,max);if(max<=0)return;
 Rectangle track={area.x+area.width-5,area.y+6,3,area.height-12};float height=fmaxf(28,track.height*area.height/total);
 if(Hot((Rectangle){track.x-6,track.y,15,track.height},true)&&IsMouseButtonDown(MOUSE_BUTTON_LEFT))*scroll=Clamp((GetMouseY()-track.y-height/2)/(track.height-height),0,1)*max;
 ThemePanel((Rectangle){track.x,track.y+Clamp(drawn/max,0,1)*(track.height-height),3,height},1,8,muted);
}
static int SddmImageControls(int x,int y,int w){
 int start=y;const char* names[]={lab_tr("Luminosità"),lab_tr("Contrasto"),lab_tr("Saturazione"),lab_tr("Temperatura"),lab_tr("Tonalità"),lab_tr("Gamma"),lab_tr("Vignettatura"),lab_tr("Rotazione")};float lo[]={-1,0,0,-1,-180,.25,0,-180},hi[]={1,2,2,1,180,3,1,180};
 for(int n=0;n<8;n++){float scale=n==4||n==5||n==7?1:100,number=sddmDraft[n]*scale;float old=sddmDraft[n];NumberSlider(140+n,x,&y,w,names[n],&number,lo[n]*scale,hi[n]*scale,n==4||n==7?"°":n==5?"×":"%");sddmDraft[n]=number/scale;if(fabsf(old-sddmDraft[n])>.00001f)sddmDirty=true;}
 const char* flips[]={lab_tr("Specchia orizzontalmente"),lab_tr("Specchia verticalmente")};
 for(int n=0;n<2;n++){sddmFlipButtons[n]=(Rectangle){x,y,w,40};int value=SwitchToggle(sddmFlipButtons[n],flips[n],sddmDraft[8+n]>.5f);if(value!=(sddmDraft[8+n]>.5f)){sddmDraft[8+n]=value;sddmDirty=true;}y+=43;}
 float zoom=sddmDraft[10]*100,old=zoom;NumberSlider(148,x,&y,w,lab_tr("Zoom"),&zoom,25,300,"%");sddmDraft[10]=zoom/100;if(old!=zoom)sddmDirty=true;
 const char* positions[]={lab_tr("Sinistra / destra"),lab_tr("Alto / basso")};
 for(int n=0;n<2;n++){float number=sddmDraft[11+n]*100,old=number;NumberSlider(149+n,x,&y,w,positions[n],&number,-100,100,"%");sddmDraft[11+n]=number/100;if(old!=number)sddmDirty=true;}
 int crop=SwitchToggle((Rectangle){x,y,w,40},lab_tr("Riempi lo schermo"),sddmDraft[13]>.5);if(crop!=(sddmDraft[13]>.5)){sddmDraft[13]=crop;sddmDirty=true;}y+=48;
 sddmResetButton=(Rectangle){x,y,w,34};if(DangerButton(sddmResetButton,lab_tr("Ripristina immagine"),true)){float defaults[]={0,1,1,0,0,1,0,0,0,0,1,0,0,1};memcpy(sddmDraft,defaults,sizeof(defaults));sddmDirty=true;}y+=44;
 return y-start;
}
static void SddmInspector(Rectangle area){
 int kind=sddmViewTab,index=sddmPicked[kind];if(index<0||index>=lab_sddm_count(kind))return;
 bool busy=lab_sddm_busy(),supported=kind||lab_sddm_supported(index);int x=area.x+18,w=area.width-36;
 DrawRectangleRec(area,surface);DrawLine(area.x,area.y,area.x,area.y+area.height,lineColor);
 TextLines(lab_sddm_text(kind,index,"title"),x,area.y+14,w-34,17,2,AppTheme.text);
 sddmCloseButton=(Rectangle){area.x+area.width-42,area.y+10,30,30};if(GlyphButton(sddmCloseButton,UI_CLOSE,false,true,""))sddmDetails=false;
 int actions=area.y+area.height-122;sddmDetailBody=(Rectangle){area.x+1,area.y+68,area.width-2,fmaxf(60,actions-area.y-74)};Rectangle saved=inputClip;inputClip=sddmDetailBody;
 if(!choiceWheel&&Hot(sddmDetailBody,true))sddmInspectorScroll=fmaxf(0,sddmInspectorScroll-ScrollInput(sddmDetailBody)*55);choiceWheel=false;
 sddmInspectorDraw=SmoothScroll(sddmInspectorDraw,sddmInspectorScroll);BeginScissorMode(sddmDetailBody.x,sddmDetailBody.y,sddmDetailBody.width,sddmDetailBody.height);int y=sddmDetailBody.y+4-sddmInspectorDraw;
 sddmPreviewRect=(Rectangle){x,y,w,w/(kind?16.f/9:lab_sddm_aspect())};Texture2D preview=SddmTextureFor(kind,index);
 if(preview.id){if(!kind&&supported)SddmEditedPreview(preview,sddmPreviewRect,sddmDraft);else Cover(preview,sddmPreviewRect,true);}else SddmPlaceholder(sddmPreviewRect,kind,index);y+=sddmPreviewRect.height+18;
 UiText(kind?lab_tr("UI di login"):SddmOriginLabel(index),x,y,13,muted);if(!kind)UiText(SddmTypeLabel(index),x+w-UiMeasure(SddmTypeLabel(index),13),y,13,muted);y+=30;
 bool blocked=inputBlocked;inputBlocked=blocked||busy;choiceEnabled=!busy;
 if(!supported)y+=TextLines(lab_tr("Questo progetto non è supportato da SDDM o contiene file mancanti."),x,y,w,14,8,muted)+20;
 else if(!kind){UiText(lab_tr("Immagine e ritaglio"),x,y,17,AppTheme.text);y+=32;y+=SddmImageControls(x,y,w);}
 else{
  const char* positions[]={lab_tr("Predefinito"),lab_tr("Sinistra"),lab_tr("Al centro"),lab_tr("Destra")};const char* values[]={"preset","left","center","right"};int current=lab_sddm_option("position"),selected=Choice(91,x,&y,w,lab_tr("Posizione"),positions,4,current);if(selected!=current)lab_sddm_action("position",values[selected]);
  current=lab_sddm_option("blur");selected=BoolChoice(93,x,&y,w,lab_tr("Sfocatura del login"),current);if(selected!=current)lab_sddm_action("blur",selected?"true":"false");
  y+=TextLines(lab_tr("La scheda mostra il preset originale. L’anteprima finale usa il wallpaper e le modifiche selezionate."),x,y,w,13,8,muted)+16;
 }
 choiceEnabled=true;inputBlocked=blocked;EndScissorMode();SddmScrollBar(sddmDetailBody,y+sddmInspectorDraw-sddmDetailBody.y+12,&sddmInspectorScroll,sddmInspectorDraw);inputClip=saved;
 UiText(lab_tr(busy?"Attendi la fine del caricamento":sddmDirty&&!kind?"Modifiche da salvare":lab_sddm_ok()?"Modifiche applicate e salvate.":"Operazione non riuscita. Dettagli nel registro."),x,actions+3,12,muted);
 sddmSaveButton=(Rectangle){x,actions+29,w,36};sddmPreviewButton=(Rectangle){0};
 if(!kind){
  sddmCancelButton=(Rectangle){x,actions+71,(w-8)/2,32};sddmPreviewButton=(Rectangle){x+(w+8)/2,actions+71,(w-8)/2,32};
  if(Button(sddmCancelButton,lab_tr("Annulla"),false,!busy&&sddmDirty))SddmLoadDraft(index);
  if(Button(sddmSaveButton,lab_tr("Salva e applica"),false,!busy&&supported&&sddmDirty&&index==lab_sddm_selected(0)))sddmSaving=lab_sddm_edit(index,sddmDraft,14);
  if(Button(sddmPreviewButton,lab_tr("Anteprima"),false,!busy&&supported&&!sddmDirty&&index==lab_sddm_selected(0)))lab_sddm_action("preview-wallpaper","");
 }else if(Button(sddmSaveButton,lab_tr("Anteprima finale"),false,!busy)){sddmViewTab=2;sddmSearching=false;}
}
static void SddmFinal(Rectangle area){
 int x=area.x+20,y=area.y+16,w=area.width-40;
 const char* modes[]={lab_tr("Duplicato"),lab_tr("Panoramico")};int mode=lab_sddm_option("display-mode");choiceEnabled=!lab_sddm_busy()&&!sddmDirty;int next=Choice(94,x,&y,w,lab_tr("Display"),modes,2,mode);choiceEnabled=true;if(next!=mode)lab_sddm_action("display-mode",next?"span":"mirror");y+=12;
 int wp=lab_sddm_selected(0),ui=lab_sddm_selected(1);if(wp>=0){UiText(lab_tr("Sfondo selezionato"),x,y,13,muted);ClippedText(lab_sddm_text(0,wp,"title"),(Rectangle){x,y+22,w*.53f,28},18,AppTheme.text);}if(ui>=0){UiText(lab_tr("UI di login"),x+w*.58f,y,13,muted);ClippedText(lab_sddm_text(1,ui,"title"),(Rectangle){x+w*.58f,y+22,w*.42f,28},18,AppTheme.text);}y+=64;
 float height=fminf(w*9.f/16,area.y+area.height-y-102),width=height*16.f/9;Rectangle preview={x+(w-width)/2,y,width,height};sddmPreviewRect=preview;Texture2D texture=SddmTextureFor(2,-1);if(texture.id)Cover(texture,preview,true);else SddmPlaceholder(preview,2,-1);
 y+=height+18;sddmPreviewButton=(Rectangle){x,y,w,38};if(Button(sddmPreviewButton,lab_tr("Anteprima SDDM · 60 secondi"),false,!lab_sddm_busy()&&lab_sddm_ready()))lab_sddm_action("preview","");y+=50;
 TextLines(lab_tr("Configurazione di prova. L’installazione nel login di sistema sarà un passaggio separato."),x,y,w,12,3,muted);
}
static void SddmLibrary(Rectangle area){
 // Catalog refreshes can reorder indices. Keep an open editor bound to its source.
 if(*sddmEditSource){int found=-1;for(int i=0;i<lab_sddm_count(0);i++)if(!strcmp(lab_sddm_text(0,i,"id"),sddmEditSource)){found=i;break;}if(found<0){sddmEditSource[0]=0;sddmDirty=false;sddmDetails=false;}sddmPicked[0]=found;}
 if(sddmPicked[0]<0&&lab_sddm_selected(0)>=0){sddmPicked[0]=lab_sddm_selected(0);SddmLoadDraft(sddmPicked[0]);}
 if(sddmPicked[1]<0)sddmPicked[1]=lab_sddm_selected(1);
 sddmUploads=0;sddmCardCount=0;memset(sddmCards,0,sizeof(sddmCards));memset(sddmStars,0,sizeof(sddmStars));
 if(sddmSaving&&!lab_sddm_busy()){sddmSaving=false;bool saved=true;for(int n=0;n<14;n++)if(fabsf(sddmDraft[n]-lab_sddm_image(sddmPicked[0],sddmImageKeys[n]))>.00001f)saved=false;if(saved)SddmLoadDraft(sddmPicked[0]);}
 int x=area.x+12,y=area.y+10,w=area.width-24;Rectangle saved=inputClip;inputClip=area;
 const char* titles[]={lab_tr("Sfondi"),lab_tr("UI di login"),lab_tr("Anteprima finale")};float tabWidth=fminf(182,(w-52)/3.f);
 for(int i=0;i<3;i++){sddmTabs[i]=(Rectangle){x+i*(tabWidth+6),y,tabWidth,36};if(Button(sddmTabs[i],titles[i],sddmViewTab==i,true)){sddmViewTab=i;sddmLibraryPage=0;sddmSearch[0]=0;sddmScroll=sddmScrollDraw=0;sddmSearching=false;choiceOpen=-1;}}
 Rectangle check={area.x+area.width-50,y,36,36};if(GlyphButton(check,UI_REFRESH,false,!lab_sddm_busy(),""))lab_sddm_action("check","");y+=50;
 if(sddmViewTab==2){SddmFinal((Rectangle){area.x,y,area.width,area.y+area.height-y});inputClip=saved;return;}
 sddmSearchBox=(Rectangle){x,y,w-152,40};sddmFilterButton=(Rectangle){x+w-144,y,40,40};
 ThemePanel(sddmSearchBox,.18f,8,sddmSearching?controlHover:control);Icon(UI_SEARCH,(Vector2){x+20,y+20},muted,false);
 bool hot=Hot(sddmSearchBox,true);if(hot){SetMouseCursor(MOUSE_CURSOR_IBEAM);if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){sddmSearching=true;EditFocus(&sddmSearchEdit,sddmSearch,true);}}
 if(!inputBlocked&&(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL))&&IsKeyPressed(KEY_L)){sddmSearching=true;EditFocus(&sddmSearchEdit,sddmSearch,true);}
 if(sddmSearching&&!inputBlocked){if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!hot)sddmSearching=false;if(EditInput(&sddmSearchEdit,sddmSearch,sizeof(sddmSearch))){sddmLibraryPage=0;sddmScroll=sddmScrollDraw=0;}}
 if(*sddmSearch||sddmSearching)EditDraw(&sddmSearchEdit,sddmSearch,(Rectangle){x+40,y+3,sddmSearchBox.width-50,34},15,sddmSearching);else UiText(lab_tr("Cerca"),x+40,y+12,15,muted);
 if(GlyphButton(sddmFilterButton,UI_FILTER,sddmFilters,sddmViewTab==0,""))sddmFilters=!sddmFilters;
 if(GlyphButton((Rectangle){x+w-96,y,40,40},UI_REFRESH,false,!lab_sddm_busy(),""))lab_sddm_action("status","");
 if(GlyphButton((Rectangle){x+w-48,y,40,40},UI_IMPORT,false,!lab_files_busy()&&sddmViewTab==0,""))lab_files_start(0);y+=50;
 if(sddmFilters&&sddmViewTab==0){
  const char* origins[]={lab_tr("Tutti"),"SDDM",lab_tr("Locale"),"Steam"};float chip=fminf(86,(w-220)/4);
  for(int i=0;i<4;i++){sddmSourceButtons[i]=(Rectangle){x+i*(chip+5),y,chip,32};if(Button(sddmSourceButtons[i],origins[i],sddmOrigin==i,true)){sddmOrigin=i;sddmLibraryPage=0;sddmScroll=sddmScrollDraw=0;}}
  const char* types[]={lab_tr("Tutti"),lab_tr("Immagine"),"GIF",lab_tr("Video"),lab_tr("Scena"),lab_tr("Web")};sddmTypeButton=(Rectangle){x+4*(chip+5),y,120,32};if(Button(sddmTypeButton,types[sddmType],sddmType>0,true)){sddmType=(sddmType+1)%6;sddmLibraryPage=0;sddmScroll=sddmScrollDraw=0;}
  sddmFavoritesButton=(Rectangle){x+4*(chip+5)+128,y,36,32};if(GlyphButton(sddmFavoritesButton,UI_STAR,sddmFavorites,true,"")){sddmFavorites=!sddmFavorites;sddmLibraryPage=0;sddmScroll=sddmScrollDraw=0;}y+=42;
 }
 int kind=sddmViewTab,total=lab_sddm_count(kind),visible[50],count=0,found=0;
 for(int i=0;i<total;i++){
  if(*sddmSearch&&!stristr(lab_sddm_text(kind,i,"title"),sddmSearch))continue;
  if(!kind){const char* origins[]={"","sddm","local","steam"},*types[]={"","image","gif","video","scene","web"};if(sddmOrigin&&strcmp(lab_sddm_text(0,i,"origin"),origins[sddmOrigin]))continue;if(sddmType&&strcmp(lab_sddm_text(0,i,"kind"),types[sddmType]))continue;if(sddmFavorites&&!lab_sddm_favorite(i,0))continue;}
  if(found/50==sddmLibraryPage)visible[count++]=i;found++;
 }
 sddmFilteredCount=found;int pages=fmaxf(1,ceilf(found/50.f));if(sddmLibraryPage>=pages){sddmLibraryPage=pages-1;count=0;}
 float bodyHeight=area.y+area.height-y-44,detail=sddmDetails?Clamp(sddmInspectorWidth,280,area.width*.52f):0;
 Rectangle grid={area.x,y,area.width-detail,bodyHeight},divider={area.x+area.width-detail-5,y,10,bodyHeight+44};
 if(sddmDetails){if(Hot(divider,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))sddmResizing=true;if(sddmResizing){sddmInspectorWidth=Clamp(area.x+area.width-GetMouseX(),280,area.width*.52f);if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT))sddmResizing=false;}if(Hot(divider,true)||sddmResizing)SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);}
 if(Hot(grid,true)&&!choiceWheel){float wheel=ScrollInput(grid);if(IsKeyDown(KEY_LEFT_CONTROL))sddmScale=Clamp(sddmScale+wheel*.05f,.7f,1.5f);else sddmScroll=fmaxf(0,sddmScroll-wheel*80);}choiceWheel=false;
 LibraryGridLayout layout=library_grid_layout(grid.width,Clamp(246*sddmScale,210,320));float tile=layout.tile,gap=22,imageHeight=kind?tile*9.f/16:tile,rowHeight=imageHeight+112,totalHeight=ceilf(count/(float)layout.columns)*rowHeight+30;
 sddmScroll=Clamp(sddmScroll,0,fmaxf(0,totalHeight-grid.height));sddmScrollDraw=SmoothScroll(sddmScrollDraw,sddmScroll);inputClip=grid;BeginScissorMode(grid.x,grid.y,grid.width,grid.height);
 for(int n=0;n<count;n++){
  int i=visible[n];Rectangle card={grid.x+20+(n%layout.columns)*(tile+gap),grid.y+16+(n/layout.columns)*rowHeight-sddmScrollDraw,tile,imageHeight};if(card.y+rowHeight<grid.y||card.y>grid.y+grid.height)continue;
  bool supported=kind||lab_sddm_supported(i),hot=Hot((Rectangle){card.x,card.y,tile,imageHeight+88},!sddmResizing);ThemePanel((Rectangle){card.x-6,card.y-6,tile+12,imageHeight+99},.035f,8,surface);
  Texture2D texture=SddmTextureFor(kind,i);if(texture.id)Cover(texture,card,kind);else SddmPlaceholder(card,kind,i);
  DrawRectangleLinesEx(card,sddmPicked[kind]==i?2.5f:1.3f,sddmPicked[kind]==i?AppTheme.border:Fade(WHITE,hot?1:.65));
  const char* origin=kind?"SDDM":SddmOriginLabel(i);float tag=UiMeasure(origin,11)+14;ThemePanel((Rectangle){card.x+tile-tag-7,card.y+imageHeight-27,tag,20},.2f,6,(Color){18,22,28,230});UiText(origin,card.x+tile-tag,card.y+imageHeight-23,11,WHITE);
  TextLines(lab_sddm_text(kind,i,"title"),card.x,card.y+imageHeight+10,tile,17,2,AppTheme.text);UiText(kind?lab_tr("UI di login"):SddmTypeLabel(i),card.x,card.y+imageHeight+66,13,muted);
  if(!supported)Icon(UI_CLOSE,(Vector2){card.x+tile-12,card.y+imageHeight+72},muted,false);
  if(lab_sddm_selected(kind)==i){DrawCircle(card.x+14,card.y+imageHeight-14,6.5f,Fade(BLACK,.65f));DrawCircle(card.x+14,card.y+imageHeight-14,3.8f,WHITE);}
  Rectangle star={card.x+tile-40,card.y+8,32,32};bool starred=!kind&&GlyphButton(star,UI_STAR,lab_sddm_favorite(i,0),!lab_sddm_busy(),"");
  if(starred)lab_sddm_favorite(i,1);else if(hot&&(kind||!CheckCollisionPointRec(GetMousePosition(),star))){SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!lab_sddm_busy())SddmPick(kind,i);}
  sddmCards[sddmCardCount]=card;sddmStars[sddmCardCount]=star;sddmCardIndices[sddmCardCount++]=i;
 }
 if(!count)TextLines(lab_tr("Nessuno sfondo corrisponde alla ricerca."),grid.x+24,grid.y+60,grid.width-48,18,4,muted);
 EndScissorMode();SddmScrollBar(grid,totalHeight,&sddmScroll,sddmScrollDraw);inputClip=area;
 int fy=area.y+area.height-36;sddmPrevious=(Rectangle){grid.x+12,fy,32,28};sddmNext=(Rectangle){grid.x+grid.width-44,fy,32,28};
 if(GlyphButton(sddmPrevious,UI_LEFT,false,sddmLibraryPage>0,"")){sddmLibraryPage--;sddmScroll=sddmScrollDraw=0;}UiText(TextFormat("%d / %d  ·  %d",sddmLibraryPage+1,pages,found),grid.x+56,fy+8,12,muted);if(GlyphButton(sddmNext,UI_RIGHT,false,sddmLibraryPage+1<pages,"")){sddmLibraryPage++;sddmScroll=sddmScrollDraw=0;}
 if(detail>0)SddmInspector((Rectangle){area.x+area.width-detail,y,detail,bodyHeight+44});inputClip=saved;
}
static void SddmGeometry(FILE* f){
 fprintf(f,",\"sddm_view\":%d,\"sddm_dirty\":%s,\"sddm_picked\":[%d,%d],\"sddm_filtered\":%d,\"sddm_tabs\":[",sddmViewTab,sddmDirty?"true":"false",sddmPicked[0],sddmPicked[1],sddmFilteredCount);
 for(int n=0;n<3;n++)fprintf(f,"%s[%.1f,%.1f]",n?",":"",sddmTabs[n].x+sddmTabs[n].width/2,sddmTabs[n].y+sddmTabs[n].height/2);fprintf(f,"],\"sddm_cards\":[");
 for(int n=0;n<sddmCardCount;n++){Rectangle r=sddmCards[n],star=sddmStars[n];fprintf(f,"%s{\"index\":%d,\"rect\":[%.1f,%.1f,%.1f,%.1f],\"star\":[%.1f,%.1f]}",n?",":"",sddmCardIndices[n],r.x,r.y,r.width,r.height,star.x+star.width/2,star.y+star.height/2);}fprintf(f,"],\"sddm_controls\":{");
 Rectangle controls[]={sddmSearchBox,sddmFilterButton,sddmTypeButton,sddmFavoritesButton,sddmSaveButton,sddmPreviewButton,sddmResetButton,sddmPrevious,sddmNext,sddmCloseButton,sddmCancelButton};const char* names[]={"search","filter","type","favorites","save","preview","reset","previous","next","close","cancel"};
 for(int n=0;n<11;n++){Rectangle r=controls[n];fprintf(f,"%s\"%s\":[%.1f,%.1f]",n?",":"",names[n],r.x+r.width/2,r.y+r.height/2);}fprintf(f,"},\"sddm_origins\":[");for(int n=0;n<4;n++)fprintf(f,"%s[%.1f,%.1f]",n?",":"",sddmSourceButtons[n].x+sddmSourceButtons[n].width/2,sddmSourceButtons[n].y+sddmSourceButtons[n].height/2);
 fprintf(f,"],\"sddm_inspector\":[%.1f,%.1f,%.1f,%.1f],\"sddm_preview_rect\":[%.1f,%.1f,%.1f,%.1f]",sddmDetailBody.x,sddmDetailBody.y,sddmDetailBody.width,sddmDetailBody.height,sddmPreviewRect.x,sddmPreviewRect.y,sddmPreviewRect.width,sddmPreviewRect.height);
}
