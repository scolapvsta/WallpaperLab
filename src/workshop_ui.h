// Native Workshop browser. Only static CDN images are decoded here; remote
// scenes run only after Steam has downloaded and catalogued their project.
static int wsSelected=-1,wsSeenRevision=-1,wsNamesRevision=-1,wsSort=0,wsPage=1;static double wsNamesChanged=0;
static double wsDirtyAt=0;
static bool wsStarted=false,wsAny=false,wsAuthorEditing=false,wsFiltersDirty=false;
static char wsAuthor[256]="",librarySearch[256]="",workshopSearch[256]="";
static TextEdit wsAuthorEdit;
static float wsScroll=0,wsScrollDraw=0,wsDetailScroll=0,wsDetailDraw=0,wsFilterScroll=0,wsFilterDraw=0;
static Texture2D wsTextures[200];static char wsTextureIDs[200][80],wsTexturePaths[200][4096];
static Rectangle wsSearchButton,wsAuthorControl,wsDownloadButton,wsPrevButton,wsNextButton,wsResultCards[200],wsTagRows[96],wsLogicControl;
static const char* wsTags[]={
 "Scene","Video","Web","Application",
 "Audio responsive","Customizable","Approved","Media Integration","3D","HDR",
 "Anime","Nature","Landscape","Cyberpunk","Game","Abstract","Fantasy","Relaxing","Sci-Fi","Music","Technology","Animal","Retro","Pixel art",
 "1280 x 720","1366 x 768","1920 x 1080","2560 x 1440","3840 x 2160",
 "Dual 3840 x 1080","Dual 5120 x 1440","Dual 7680 x 2160","Ultrawide 2560 x 1080","Ultrawide 3440 x 1440",
 "Triple 5760 x 1080","Triple 7680 x 1440","Portrait 720 x 1280","Portrait 1080 x 1920","Portrait 1440 x 2560","Portrait 2160 x 3840","Everyone","Questionable","Mature"};
static const int wsTagCount=sizeof(wsTags)/sizeof(wsTags[0]);
static bool wsWholeGroups[6]={0};
static bool wsTagSelected[96]={0},wsGroups[6]={true,true,false,false,false,false};
static void WorkshopSearch(int pageNumber){
 char tags[4096]="";for(int i=0;i<wsTagCount;i++)if(wsTagSelected[i]){if(*tags)strcat(tags,"|");strcat(tags,wsTags[i]);}
 int sorts[]={3,0,12,1,19,11};lab_ws_search(searchBuffer,wsAuthor,tags,wsAny,sorts[wsSort],pageNumber,(wsWholeGroups[0]?1:0)|(wsWholeGroups[1]?2:0)|(wsWholeGroups[2]?4:0)|(wsWholeGroups[3]?8:0)|(wsWholeGroups[4]?16:0));wsPage=pageNumber;wsStarted=true;wsFiltersDirty=false;wsAuthorEditing=false;isSearching=false;
}
static void WorkshopEnter(int previous,int next){
 if(previous==3)snprintf(workshopSearch,sizeof(workshopSearch),"%s",searchBuffer);else if(previous!=2)snprintf(librarySearch,sizeof(librarySearch),"%s",searchBuffer);
 if(next==3){snprintf(searchBuffer,sizeof(searchBuffer),"%s",workshopSearch);if(!wsStarted){lab_ws_tick();WorkshopSearch(1);}}
 else if(next!=2)snprintf(searchBuffer,sizeof(searchBuffer),"%s",librarySearch);
 searchBufferCount=strlen(searchBuffer);EditFocus(&searchEdit,searchBuffer,false);
}
static const char* WsBytes(double bytes){if(bytes>=1024*1024*1024)return TextFormat("%.2f GB",bytes/(1024*1024*1024));if(bytes>=1024*1024)return TextFormat("%.1f MB",bytes/(1024*1024));if(bytes>0)return TextFormat("%.0f KB",bytes/1024);return lab_tr("Dimensione non dichiarata");}
static const char* WsDate(double stamp){time_t t=(time_t)stamp;struct tm* v=localtime(&t);static char value[32];if(v)strftime(value,sizeof(value),"%d/%m/%Y",v);else value[0]=0;return value;}
static void WorkshopTick(void){
 if(page==3&&wsFiltersDirty){wsFiltersDirty=false;wsDirtyAt=GetTime();}if(page==3&&wsDirtyAt>0&&GetTime()-wsDirtyAt>.45){wsDirtyAt=0;bool edit=wsAuthorEditing,searching=isSearching;WorkshopSearch(1);wsAuthorEditing=edit;isSearching=searching;}
 static double lastTick=0;if(GetTime()-lastTick>.1){lab_ws_tick();lastTick=GetTime();}int revision=lab_ws_number(-1,"revision");if(revision!=wsSeenRevision){wsSeenRevision=revision;wsSelected=-1;wsScroll=wsScrollDraw=wsDetailScroll=wsDetailDraw=0;memset(wsResultCards,0,sizeof(wsResultCards));if(lab_ws_count())ReloadUiFonts();}
 int namesRevision=lab_ws_number(-1,"names_revision");if(namesRevision!=wsNamesRevision){wsNamesRevision=namesRevision;wsNamesChanged=GetTime();}if(wsNamesChanged>0&&GetTime()-wsNamesChanged>.5&&!lab_ws_number(-1,"busy")){wsNamesChanged=0;if(lab_ws_count())ReloadUiFonts();}
 int loaded=0;
 for(int i=0;i<200;i++){
  char id[80]="",path[4096]="";if(i<lab_ws_count()){snprintf(id,sizeof(id),"%s",lab_ws_text(i,"id"));snprintf(path,sizeof(path),"%s",lab_ws_text(i,"thumbnail"));}
  if(strcmp(id,wsTextureIDs[i])||strcmp(path,wsTexturePaths[i])){
   if(wsTextures[i].id)UnloadTexture(wsTextures[i]);wsTextures[i]=(Texture2D){0};
   snprintf(wsTextureIDs[i],sizeof(wsTextureIDs[i]),"%s",id);wsTexturePaths[i][0]=0;
  }
  if(*path&&!wsTextures[i].id&&loaded<2){
   int bytes=0;unsigned char* data=LoadFileData(path,&bytes);if(data){Image image=LoadImageFromMemory(".png",data,bytes);UnloadFileData(data);if(image.data){if(image.width>1200)ImageResize(&image,1200,1200*(float)image.height/image.width);wsTextures[i]=LoadTextureFromImage(image);SetTextureFilter(wsTextures[i],TEXTURE_FILTER_BILINEAR);UnloadImage(image);}}snprintf(wsTexturePaths[i],sizeof(wsTexturePaths[i]),"%s",path);loaded++;
  }
 }
 if(wsAuthorEditing&&!inputBlocked){if(EditInput(&wsAuthorEdit,wsAuthor,sizeof(wsAuthor)))wsFiltersDirty=true;if(IsKeyPressed(KEY_ENTER))WorkshopSearch(1);}
}
static void WorkshopFilters(Rectangle area){
 DrawRectangleRec(area,surface);DrawLine(area.x+area.width-1,area.y,area.x+area.width-1,area.y+area.height,lineColor);Rectangle saved=inputClip;inputClip=area;
 if(Hot(area,true)&&choiceOpen<60)wsFilterScroll=fmaxf(0,wsFilterScroll-ScrollInput(area)*48);wsFilterDraw=SmoothScroll(wsFilterDraw,wsFilterScroll);
 BeginScissorMode(area.x,area.y,area.width,area.height);int x=area.x+16,y=area.y+16-wsFilterDraw,w=area.width-32;
 UiText(lab_tr("Filtri Workshop"),x,y,19,AppTheme.text);y+=32;
 wsLogicControl=(Rectangle){x,y,w,34};if(Button(wsLogicControl,lab_tr(wsAny?"Almeno un tag (OR)":"Tutti i tag (AND)"),true,true)){wsAny=!wsAny;wsFiltersDirty=true;}y+=42;
 y+=TextLines(lab_tr(wsAny?"Basta uno dei tag. Nome e autore restringono sempre la ricerca.":"Lo sfondo deve avere tutti i tag selezionati."),x,y,w,13,3,muted)+14;
 UiText(lab_tr("Autore"),x,y,14,muted);y+=24;wsAuthorControl=(Rectangle){x,y,w,34};bool hot=Hot(wsAuthorControl,true);ThemePanel(wsAuthorControl,.2f,8,wsAuthorEditing?controlHover:control);
 if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){wsAuthorEditing=true;isSearching=false;EditFocus(&wsAuthorEdit,wsAuthor,true);}if(hot)SetMouseCursor(MOUSE_CURSOR_IBEAM);
 if(wsAuthorEditing&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!hot)wsAuthorEditing=false;
 if(*wsAuthor||wsAuthorEditing)EditDraw(&wsAuthorEdit,wsAuthor,(Rectangle){x+8,y+2,w-16,30},14,wsAuthorEditing);else UiText(lab_tr("Nome noto, SteamID o link profilo"),x+8,y+10,12,muted);
 BeginScissorMode(area.x,area.y,area.width,area.height);y+=44;
 if(wsAuthorEditing&&*wsAuthor){int suggestions=0;for(int n=0;n<lab_ws_author_count()&&suggestions<5;n++){char name[256];snprintf(name,sizeof(name),"%s",lab_ws_author(n,0));if(!stristr(name,wsAuthor))continue;if(Button((Rectangle){x,y,w,30},name,false,true)){snprintf(wsAuthor,sizeof(wsAuthor),"%s",lab_ws_author(n,1));wsAuthorEditing=false;wsFiltersDirty=true;}y+=35;suggestions++;}}

 const char* sortNames[]={"Popolari questa settimana","Migliori valutazioni","Più scaricati","Più recenti","Ultimi aggiornamenti","Rilevanza"};const char* sortIcons[]={"↗","★","↓","◷","↻","≡"};static char sortText[6][192];const char* sorts[6];for(int n=0;n<6;n++){snprintf(sortText[n],192,"%s %s",sortIcons[n],lab_tr(sortNames[n]));sorts[n]=sortText[n];};int previous=wsSort;wsSort=Choice(60,x,&y,w,lab_tr("Ordina"),sorts,6,wsSort);if(wsSort!=previous)wsFiltersDirty=true;
 if(Button((Rectangle){x,y,w,34},lab_tr("Azzera filtri"),true,true)){memset(wsTagSelected,0,sizeof(wsTagSelected));wsAuthor[0]=0;searchBuffer[0]=0;searchBufferCount=0;memset(wsWholeGroups,0,sizeof(wsWholeGroups));wsSort=0;wsAny=false;wsFiltersDirty=true;}y+=48;
 const int starts[]={0,4,10,24,40,43},ids[]={61,62,63,55,56};const char* groupNames[]={lab_tr("Tipo di sfondo"),lab_tr("Caratteristiche"),lab_tr("Genere"),lab_tr("Risoluzioni"),lab_tr("Classificazione dei contenuti")};
 for(int group=0;group<5;group++){if(group==3)continue;bool selected[64],before[64];const char* labels[64];char resolutionLabels[64][128];int count=starts[group+1]-starts[group];for(int n=0;n<count;n++){int index=starts[group]+n;selected[n]=before[n]=wsTagSelected[index];labels[n]=wsTags[index];if(group==3){const char* digit=wsTags[index];while(*digit&&(*digit<'0'||*digit>'9'))digit++;snprintf(resolutionLabels[n],128,"%s",digit);labels[n]=resolutionLabels[n];}}MultiChoice(ids[group],x,&y,w,groupNames[group],labels,count,selected);for(int n=0;n<count;n++){if(selected[n]!=before[n])wsFiltersDirty=true;wsTagSelected[starts[group]+n]=selected[n];}}
 y+=12;UiText(lab_tr("Risoluzioni"),x,y,18,AppTheme.text);y+=30;
 bool selected[16],before[16];const char* labels[16];int groups[16];for(int n=0;n<16;n++){selected[n]=before[n]=wsTagSelected[24+n];const char* t=wsTags[24+n];groups[n]=!strncmp(t,"Dual",4)?1:!strncmp(t,"Ultrawide",9)?2:!strncmp(t,"Triple",6)?3:!strncmp(t,"Portrait",8)?4:0;while(*t&&(*t<'0'||*t>'9'))t++;labels[n]=t;}ResolutionPicker(x,&y,w,labels,groups,selected,16,wsWholeGroups);for(int n=0;n<16;n++){if(selected[n]!=before[n])wsFiltersDirty=true;wsTagSelected[n+24]=selected[n];}
 float total=y+wsFilterDraw-area.y+24;wsFilterScroll=Clamp(wsFilterScroll,0,fmaxf(0,total-area.height));EndScissorMode();inputClip=saved;
 if(total>area.height){float h=area.height*area.height/total;DrawRectangle(area.x+area.width-4,area.y+wsFilterDraw/(total-area.height)*(area.height-h),3,h,muted);}
}
static void WorkshopGrid(Rectangle area){
 Rectangle saved=inputClip;inputClip=area;bool busy=lab_ws_number(-1,"busy");
 const char* error=lab_ws_text(-1,"error");if(*error){TextLines(error,area.x+24,area.y+24,area.width-48,18,6,(Color){236,133,140,255});if(Button((Rectangle){area.x+24,area.y+184,170,36},lab_tr("Riprova"),false,!busy))WorkshopSearch(wsPage);inputClip=saved;return;}
 int count=lab_ws_count();if(!count){UiText(lab_tr(busy?"Ricerca su Steam...":"Nessun risultato. Prova altri filtri."),area.x+24,area.y+28,18,muted);inputClip=saved;return;}
 float footer=54;Rectangle viewport={area.x,area.y,area.width,area.height-footer};
 if(Hot(viewport,true))wsScroll=fmaxf(0,wsScroll-ScrollInput(viewport)*64);wsScrollDraw=SmoothScroll(wsScrollDraw,wsScroll);
 LibraryGridLayout layout=library_grid_layout(area.width,246);int columns=layout.columns;float tile=layout.tile,gap=12,rowHeight=tile*.5625f+84;float total=ceilf(count/(float)columns)*rowHeight+24;
 wsScroll=Clamp(wsScroll,0,fmaxf(0,total-viewport.height));BeginScissorMode(viewport.x,viewport.y,viewport.width,viewport.height);
 for(int i=0;i<count&&i<200;i++){
  int col=i%columns,row=i/columns;Rectangle card={area.x+16+col*(tile+gap),area.y+14+row*rowHeight-wsScrollDraw,tile,rowHeight-gap};wsResultCards[i]=card;
  if(card.y+card.height<viewport.y||card.y>viewport.y+viewport.height)continue;
  bool hot=Hot(card,!busy);ThemePanel(card,.12f,10,i==wsSelected?controlHover:surface);Rectangle image={card.x+6,card.y+6,card.width-12,tile*.5625f-6};
  if(wsTextures[i].id)Cover(wsTextures[i],image,false);else{ThemePanel(image,.1f,8,control);Icon(UI_LIBRARY,(Vector2){image.x+image.width/2,image.y+image.height/2},muted,false);}
  TextLines(lab_ws_text(i,"title"),card.x+12,image.y+image.height+10,card.width-24,16,2,AppTheme.text);
  char author[256];snprintf(author,sizeof(author),"%s",lab_ws_text(i,"author"));TextLines(author,card.x+12,card.y+card.height-22,card.width-75,12,1,muted);
  if(lab_ws_number(i,"installed"))Icon(UI_CHECK,(Vector2){card.x+card.width-20,card.y+card.height-16},AppTheme.border,true);
  Rectangle download={image.x+image.width-36,image.y+6,30,30};char state[32];snprintf(state,sizeof(state),"%s",lab_ws_download_text(i,"status"));bool downloading=!strcmp(state,"queued")||!strcmp(state,"downloading");
  bool downloaded=lab_ws_number(i,"installed");if(ActionGlyphButton(download,downloaded?UI_CHECK:UI_DOWNLOAD,!busy&&!downloading&&!downloaded,""))lab_ws_download(i);
  if(i==wsSelected||hot)DrawRectangleRoundedLinesEx(card,.12f,10,i==wsSelected?2:1,i==wsSelected?AppTheme.border:muted);
  if(hot&&!CheckCollisionPointRec(GetMousePosition(),download)&&CheckCollisionPointRec(GetMousePosition(),viewport)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){wsSelected=i;wsDetailScroll=wsDetailDraw=0;isSearching=wsAuthorEditing=false;}
 }
 EndScissorMode();
 int pages=fmaxf(1,ceil(lab_ws_number(-1,"total")/(lab_option("page_size")?lab_option("page_size"):100)));int move=PageFooter(area,wsPage,pages,busy);if(move)WorkshopSearch(move==2?1:wsPage+move);inputClip=saved;
}
static void WorkshopInspector(Rectangle area){
 if(wsSelected<0||wsSelected>=lab_ws_count()||area.width<120)return;int i=wsSelected;DrawRectangleRec(area,surface);DrawLine(area.x,area.y,area.x,area.y+area.height,lineColor);Rectangle saved=inputClip;Rectangle viewport={area.x+16,area.y+16,area.width-32,area.height-92};inputClip=viewport;
 if(Hot(viewport,true))wsDetailScroll=fmaxf(0,wsDetailScroll-ScrollInput(viewport)*48);wsDetailDraw=SmoothScroll(wsDetailDraw,wsDetailScroll);BeginScissorMode(viewport.x,viewport.y,viewport.width,viewport.height);
 int x=viewport.x,y=viewport.y-wsDetailDraw,w=viewport.width;Rectangle image={x,y,w,w*.5625f};if(wsTextures[i].id)Cover(wsTextures[i],image,false);else ThemePanel(image,.06f,8,control);y+=image.height+16;
 y+=TextLines(lab_ws_text(i,"title"),x,y,w,23,4,AppTheme.text)+8;
 y+=TextLines(lab_ws_text(i,"author"),x,y,w,16,2,muted)+14;
 if(Button((Rectangle){x,y,w,32},lab_tr("Altri sfondi di questo autore"),false,true)){snprintf(wsAuthor,sizeof(wsAuthor),"%s",lab_ws_text(i,"owner"));searchBuffer[0]=0;searchBufferCount=0;WorkshopSearch(1);}y+=44;
 const char* size=WsBytes(lab_ws_number(i,"size"));UiText(size,x,y,15,AppTheme.text);y+=26;
 UiText(TextFormat("%.0f  %s  ·  %.0f  %s",lab_ws_number(i,"votes_up"),lab_tr("voti positivi"),lab_ws_number(i,"subscriptions"),lab_tr("sottoscrizioni")),x,y,13,muted);y+=24;
 UiText(TextFormat("%s %s",lab_tr("Pubblicato:"),WsDate(lab_ws_number(i,"created"))),x,y,13,muted);y+=22;
 UiText(TextFormat("%s %s",lab_tr("Aggiornato:"),WsDate(lab_ws_number(i,"updated"))),x,y,13,muted);y+=30;
 y+=TextLines(lab_ws_text(i,"tags"),x,y,w,14,12,AppTheme.border)+18;
 if(strstr(lab_ws_text(i,"tags"),"Application"))y+=TextLines(lab_tr("Applicazioni Windows: il download non garantisce la compatibilità con il renderer Linux."),x,y,w,14,4,(Color){236,166,103,255})+12;
 if(lab_ws_number(i,"children")>0){y+=TextLines(TextFormat(lab_tr("Dipendenze richieste: %.0f · download automatico"),lab_ws_number(i,"children")),x,y,w,14,3,muted)+14;}
 UiText(lab_tr("Descrizione dell'autore"),x,y,17,AppTheme.text);y+=30;
 // Preserve the author's text and links without interpreting HTML/scripts.
 y+=TextLines(lab_ws_text(i,"plain_description"),x,y,w,15,320,AppTheme.text)+18;
 if(Button((Rectangle){x,y,w,34},lab_tr("Apri pagina Steam"),false,true))lab_ws_open(i,0);y+=42;
 if(Button((Rectangle){x,y,w,34},lab_tr("Profilo autore"),false,true))lab_ws_open(i,1);y+=48;
 float total=y+wsDetailDraw-viewport.y;wsDetailScroll=Clamp(wsDetailScroll,0,fmaxf(0,total-viewport.height));EndScissorMode();inputClip=saved;
 if(total>viewport.height){float h=fmaxf(20,viewport.height*viewport.height/total);DrawRectangle(area.x+area.width-4,viewport.y+wsDetailDraw/(total-viewport.height)*(viewport.height-h),2,h,muted);}
 char status[32];snprintf(status,sizeof(status),"%s",lab_ws_download_text(i,"status"));bool pending=!strcmp(status,"queued")||!strcmp(status,"downloading"),local=lab_ws_number(i,"installed");
 wsDownloadButton=(Rectangle){area.x+16,area.y+area.height-58,area.width-32,40};
 const char* label=local?lab_tr("Apri nella libreria"):!strcmp(lab_ws_download_text(i,"phase"),"dependencies")?lab_tr("Download dipendenze..."):!strcmp(status,"queued")?lab_tr("In coda su Steam..."):pending?TextFormat("%s  %.0f%%",lab_tr("Download"),100*lab_ws_download_progress(i)):!strcmp(status,"error")?lab_tr("Riprova download"):lab_tr("Download");
 if(Button(wsDownloadButton,label,true,!pending)){
  if(local){char id[80];snprintf(id,sizeof(id),"%s",lab_ws_text(i,"id"));for(int n=0;n<lab_count();n++){const char* path=lab_path(n);const char* base=strrchr(path,'/');if(base&&!strcmp(base+1,id)){ChangePage(0);SelectItem(n,0);break;}}}
  else lab_ws_download(i);
 }
 if(pending)DrawRectangle(area.x+16,area.y+area.height-12,(area.width-32)*lab_ws_download_progress(i),3,AppTheme.border);
 if(!strcmp(status,"error"))TextLines(lab_ws_download_text(i,"error"),area.x+16,area.y+area.height-86,area.width-32,12,2,(Color){236,133,140,255});
}
static void WorkshopCleanup(void){for(int i=0;i<200;i++)if(wsTextures[i].id)UnloadTexture(wsTextures[i]);}
