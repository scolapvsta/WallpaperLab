// Deferred overlay menus: opening a choice never changes document geometry.
static int choiceReserve=0;static bool choiceEnabled=true;
static int choiceScroll=0,choicePending=-1,choiceValue=-1;
static Rectangle choiceControls[96],choiceRows[96][128];static int choiceCounts[96];
static bool choiceWheel=false,choiceMulti=false;static bool* choiceChecks=NULL;
static char choiceLabels[128][256];static int choiceSelected=0;static Rectangle choiceList,choiceThumb;
// Shared toggles: boolean switches and arbitrary pairs retain their real values.
static bool choiceToggle[96];static int choiceToggleValue[96];
static bool ToggleRow(Rectangle row,const char* label,const char* state,bool value,bool enabled){
 bool hot=Hot(row,enabled);Color ink=enabled?AppTheme.text:Fade(AppTheme.text,.4f);
 int labelLines=UiMeasure(label,13)>row.width-76?2:1;int h=TextLines(label,row.x,row.y+(state?0:(row.height-labelLines*20)/2),row.width-76,13,2,ink);
 if(state&&*state)TextLines(state,row.x,row.y+h+1,row.width-76,12,2,enabled?muted:Fade(muted,.4f));
 Rectangle track={row.x+row.width-54,row.y+(row.height-24)/2,48,24};
 if(StrawberryTheme())BerryCookie(track,enabled?(value?AppTheme.border:hot?controlHover:control):Fade(control,.45f));else ThemePanel(track,1,16,enabled?(value?AppTheme.border:hot?controlHover:control):Fade(control,.45f));
 DrawCircleV((Vector2){track.x+(value?36:12),track.y+12},9,value?(lab_option("light_theme")?WHITE:(Color){24,31,37,255}):ink);
 if(hot)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);return hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)?!value:value;
}
static bool SwitchToggle(Rectangle row,const char* label,bool value){return ToggleRow(row,label,NULL,value,!applying);}
static int ToggleHeight(const char* label,const char** options,int w){
 int lines=UiMeasure(label,13)>w-76?2:1,stateLines=options?(fmaxf(UiMeasure(options[0],12),UiMeasure(options[1],12))>w-76?2:1):0;return options?fmaxf(40,lines*20+stateLines*19+3):40;
}
static int ToggleChoice(int id,int x,int* y,int w,const char* label,const char** options,int selected){
 int height=ToggleHeight(label,options,w);Rectangle row={x,*y,w,height};
 choiceControls[id]=(Rectangle){x+w-54,*y+(height-24)/2,48,24};choiceCounts[id]=2;choiceToggle[id]=true;choiceToggleValue[id]=selected;memset(choiceRows[id],0,sizeof(choiceRows[id]));
 if(choiceOpen==id)choiceOpen=-1;
 const char* state=options?(selected>=0&&selected<2?options[selected]:"—"):NULL;
 bool value=selected==1,next=ToggleRow(row,label,state,value,!applying&&choiceEnabled);*y+=height+3;
 return next!=value?(selected<0?0:1-selected):selected;
}
static int Choice(int id,int x,int* py,int w,const char* label,const char** options,int count,int selected){
 if(count==2&&id!=38)return ToggleChoice(id,x,py,w,label,options,selected);
 choiceToggle[id]=false;
 int result=selected;if(choicePending==id){result=choiceValue;choicePending=-1;}
 int y=*py;int cw=*label?fminf(292,w*.60f):w;TextLines(label,x,y+5,w-cw-8,13,2,AppTheme.text);Rectangle box={x+w-cw,y,cw-choiceReserve,34};choiceControls[id]=box;choiceCounts[id]=count;*py+=43;
 choiceRegistering=choiceOpen<0||choiceOpen==id||!CheckCollisionPointRec(GetMousePosition(),choiceList);if(!choiceEnabled&&choiceOpen==id)choiceOpen=-1;buttonTrailing=24;bool clicked=Button(box,selected>=0&&selected<count?options[selected]:"—",choiceOpen==id,!applying&&choiceEnabled);buttonTrailing=0;choiceRegistering=false;
 if(clicked){choiceOpen=choiceOpen==id?-1:id;choiceScroll=Clamp(selected-2,0,fmaxf(0,count-7));}
 if(choiceOpen==id){choiceSelected=result;choiceMulti=false;choiceChecks=NULL;for(int n=0;n<count&&n<128;n++)snprintf(choiceLabels[n],256,"%s",options[n]);}
 DrawLineEx((Vector2){box.x+box.width-15,y+14},(Vector2){box.x+box.width-10,y+19},1.5f,muted);return result;
}
static void MultiChoice(int id,int x,int* y,int w,const char* label,const char** options,int count,bool* selected){
 // Multi-select pairs are independent flags, not a single exclusive switch.
 if(count==2){if(choiceOpen==id)choiceOpen=-1;choiceControls[id].width=0;UiText(label,x,*y,13,AppTheme.text);*y+=24;for(int n=0;n<2;n++){selected[n]=SwitchToggle((Rectangle){x,*y,w,40},options[n],selected[n]);*y+=43;}return;}
 if(choicePending==id){selected[choiceValue]=!selected[choiceValue];choicePending=-1;}
 int number=0;for(int n=0;n<count;n++)number+=selected[n];const char* summary=number?TextFormat(lab_tr("%d selezionate"),number):lab_tr("Tutti");
 // Reuse field geometry and overlay lifecycle without borrowing stack pointers.
 int ignored=Choice(id,x,y,w,label,options,count,-1);(void)ignored;
 Rectangle box=choiceControls[id];ThemePanel(box,.2f,8,control);UiText(summary,box.x+(box.width-UiMeasure(summary,14))/2,box.y+10,14,AppTheme.text);
 if(choiceOpen==id){choiceMulti=true;choiceChecks=NULL;for(int n=0;n<count;n++)snprintf(choiceLabels[n],256,"%s %s",selected[n]?"✓":"  ",options[n]);}
}
static void ChoiceOverlay(void){
 if(choiceOpen<0)return;int id=choiceOpen,count=choiceCounts[id],visible=fminf(7,count);Rectangle box=choiceControls[id];
 choiceList=(Rectangle){box.x,box.y+box.height+4,fmaxf(box.width,240),visible*34+8};choiceList.x=Clamp(choiceList.x,8,GetScreenWidth()-choiceList.width-8);if(choiceList.y+choiceList.height>GetScreenHeight()-12)choiceList.y=box.y-choiceList.height-4;
 Color oldSurface=surface,oldControl=control,oldHover=controlHover,oldLine=lineColor;surface.a=control.a=controlHover.a=lineColor.a=255;
 Rectangle old=inputClip;inputClip=(Rectangle){0,0,GetScreenWidth(),GetScreenHeight()};bool modal=modalDrawing;modalDrawing=true;choiceRendering=true;
 ThemePanel((Rectangle){choiceList.x-3,choiceList.y-3,choiceList.width+6,choiceList.height+6},.06f,8,lineColor);ThemePanel(choiceList,.06f,8,surface);
 if(CheckCollisionPointRec(GetMousePosition(),choiceList)){static float wheelRest=0;wheelRest+=ScrollInput(choiceList);int steps=truncf(wheelRest);wheelRest-=steps;choiceScroll=Clamp(choiceScroll-steps,0,fmaxf(0,count-visible));}
 memset(choiceRows[id],0,sizeof(choiceRows[id]));for(int n=0;n<visible;n++){int i=n+choiceScroll;Rectangle row={choiceList.x+4,choiceList.y+4+n*34,choiceList.width-20,32};choiceRows[id][i]=row;
  if(Button(row,choiceLabels[i],!choiceMulti&&i==choiceSelected,true)){choicePending=id;choiceValue=i;if(!choiceMulti)choiceOpen=-1;}


 }
 if(count>visible){static bool dragging=false;static float grab=0;float track=choiceList.height-8,h=fmaxf(22,track*visible/count);Rectangle thumb={choiceList.x+choiceList.width-10,choiceList.y+4+(track-h)*choiceScroll/(count-visible),6,h};choiceThumb=thumb;Rectangle hit={thumb.x-4,thumb.y,14,h};if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),hit)){dragging=true;grab=GetMouseY()-thumb.y;}if(dragging){if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))choiceScroll=Clamp(roundf((GetMouseY()-choiceList.y-4-grab)/(track-h)*(count-visible)),0,count-visible);else dragging=false;}ThemePanel(thumb,1,6,dragging?AppTheme.border:muted);}
 if(IsKeyPressed(KEY_ESCAPE)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),choiceList)&&!CheckCollisionPointRec(GetMousePosition(),box)))choiceOpen=-1;
 choiceRendering=false;modalDrawing=modal;inputClip=old;surface=oldSurface;control=oldControl;controlHover=oldHover;lineColor=oldLine;
}
static int BoolChoice(int id,int x,int* y,int w,const char* label,bool value){return ToggleChoice(id,x,y,w,label,NULL,value);}

// A shared compact slider. Drafts belong to the caller until mouse release.
static Rectangle numberTracks[160];
static int uiSliderDrag=-1;static Rectangle adjustmentTracks[3],opacityTrack;
static bool ValueSlider(int id,int x,int* y,int w,const char* label,float* value,float lo,float hi,bool signedValue){
 int top=*y;UiText(label,x,top,14,AppTheme.text);const char* number=signedValue?TextFormat("%+.0f%%",*value*100):TextFormat("%.0f%%",*value*100);UiText(number,x+w-UiMeasure(number,13),top,13,muted);
 Rectangle track={x+5,top+28,w-10,5};if(id==0)opacityTrack=track;else if(id<=3)adjustmentTracks[id-1]=track;
 if(Hot((Rectangle){x,top+17,w,26},!applying)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))uiSliderDrag=id;
 bool committed=false;if(uiSliderDrag==id){*value=lo+Clamp((GetMouseX()-track.x)/track.width,0,1)*(hi-lo);if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){uiSliderDrag=-1;committed=true;}}
 float t=(*value-lo)/(hi-lo);ThemePanel(track,1,6,controlHover);DrawRectangleRec((Rectangle){track.x,track.y,track.width*t,5},AppTheme.border);DrawCircle(track.x+track.width*t,track.y+2,6,AppTheme.text);if(Hot((Rectangle){x,top+17,w,26},!applying)||uiSliderDrag==id)SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);*y+=48;return committed;
}

static bool EntrySlider(int x,int* y,int w,const char* label,float* value){
 int top=*y,cw=fminf(292,w*.6f);TextLines(label,x,top+5,w-cw-8,13,2,AppTheme.text);Rectangle box={x+w-cw,top,cw,34};ThemePanel(box,.2f,8,control);opacityTrack=(Rectangle){box.x+10,top+16,cw-66,4};
 if(Hot(box,!applying)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))uiSliderDrag=0;bool commit=false;
 if(uiSliderDrag==0){*value=Clamp((GetMouseX()-opacityTrack.x)/opacityTrack.width,0,1)*.3f;if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){uiSliderDrag=-1;commit=true;}}
 themeTransparencyPreview=uiSliderDrag==0?*value*100:-1;
 ThemePanel(opacityTrack,1,6,controlHover);DrawRectangle(opacityTrack.x,opacityTrack.y,opacityTrack.width*(*value/.3f),4,AppTheme.border);DrawCircle(opacityTrack.x+opacityTrack.width*(*value/.3f),opacityTrack.y+2,5,AppTheme.text);UiText(TextFormat("%.0f%%",*value*100),box.x+cw-42,top+10,12,muted);*y+=43;return commit;
}

// Compact setting slider with explicit units; values persist on release.
static bool NumberSlider(int id,int x,int* y,int w,const char* label,float* value,float lo,float hi,const char* suffix){
 int top=*y;TextLines(label,x,top,w-90,14,2,AppTheme.text);const char* number=TextFormat(!strcmp(suffix,"×")?"%.2f%s":"%.0f%s",*value,suffix);UiText(number,x+w-UiMeasure(number,13),top,13,muted);Rectangle track={x+5,top+28,w-10,4};numberTracks[id]=track;
 if(Hot((Rectangle){x,top+18,w,25},!applying)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))uiSliderDrag=id;
 bool commit=false;if(uiSliderDrag==id){*value=lo+Clamp((GetMouseX()-track.x)/track.width,0,1)*(hi-lo);if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){uiSliderDrag=-1;commit=true;}}
 ThemePanel(track,1,8,controlHover);DrawRectangle(track.x,track.y,track.width*(*value-lo)/(hi-lo),4,AppTheme.border);DrawCircleV((Vector2){track.x+track.width*(*value-lo)/(hi-lo),track.y+2},5,AppTheme.text);*y+=48;return commit;
}
static void SettingSlider(int id,int x,int* y,int w,const char* label,const char* key,int lo,int hi,const char* suffix){static float drafts[32];int at=id-100;if(uiSliderDrag!=id)drafts[at]=lab_option(key);if(NumberSlider(id,x,y,w,label,&drafts[at],lo,hi,suffix))lab_set_option(key,lroundf(drafts[at]));}
