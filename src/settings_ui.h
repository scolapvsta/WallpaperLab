// Deferred overlay menus: opening a choice never changes document geometry.
static int choiceReserve=0;static bool choiceEnabled=true;
static int choiceScroll=0,choicePending=-1,choiceValue=-1;
static Rectangle choiceControls[96],choiceRows[96][96];static int choiceCounts[96];
static bool choiceWheel=false,choiceMulti=false;static bool* choiceChecks=NULL;
static char choiceLabels[96][256];static int choiceSelected=0;static Rectangle choiceList,choiceThumb;
static int Choice(int id,int x,int* py,int w,const char* label,const char** options,int count,int selected){
 int result=selected;if(choicePending==id){result=choiceValue;choicePending=-1;}
 int y=*py;int cw=fminf(292,w*.60f);TextLines(label,x,y+5,w-cw-8,13,2,AppTheme.text);Rectangle box={x+w-cw,y,cw-choiceReserve,34};choiceControls[id]=box;choiceCounts[id]=count;*py+=43;
 choiceRegistering=choiceOpen<0||choiceOpen==id||!CheckCollisionPointRec(GetMousePosition(),choiceList);if(!choiceEnabled&&choiceOpen==id)choiceOpen=-1;bool clicked=Button(box,selected>=0&&selected<count?options[selected]:"—",choiceOpen==id,!applying&&choiceEnabled);choiceRegistering=false;
 if(clicked){choiceOpen=choiceOpen==id?-1:id;choiceScroll=Clamp(selected-2,0,fmaxf(0,count-7));}
 if(choiceOpen==id){choiceSelected=result;choiceMulti=false;choiceChecks=NULL;for(int n=0;n<count&&n<96;n++)snprintf(choiceLabels[n],256,"%s",options[n]);}
 DrawLineEx((Vector2){box.x+box.width-15,y+14},(Vector2){box.x+box.width-10,y+19},1.5f,muted);return result;
}
static void MultiChoice(int id,int x,int* y,int w,const char* label,const char** options,int count,bool* selected){
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
 Rectangle old=inputClip;inputClip=(Rectangle){0,0,GetScreenWidth(),GetScreenHeight()};bool modal=modalDrawing;modalDrawing=true;choiceRendering=true;
 ThemePanel((Rectangle){choiceList.x-3,choiceList.y-3,choiceList.width+6,choiceList.height+6},.06f,8,lineColor);ThemePanel(choiceList,.06f,8,surface);
 if(CheckCollisionPointRec(GetMousePosition(),choiceList)){static float wheelRest=0;wheelRest+=ScrollInput(choiceList);int steps=truncf(wheelRest);wheelRest-=steps;choiceScroll=Clamp(choiceScroll-steps,0,fmaxf(0,count-visible));}
 memset(choiceRows[id],0,sizeof(choiceRows[id]));for(int n=0;n<visible;n++){int i=n+choiceScroll;Rectangle row={choiceList.x+4,choiceList.y+4+n*34,choiceList.width-20,32};choiceRows[id][i]=row;
  if(Button(row,choiceLabels[i],!choiceMulti&&i==choiceSelected,true)){choicePending=id;choiceValue=i;if(!choiceMulti)choiceOpen=-1;}
  if(id==33){if(!fontPreview[i].texture.id){rlDrawRenderBatchActive();int glyphs[224];for(int n=0;n<224;n++)glyphs[n]=n+32;fontPreview[i]=LoadSystemFont(lab_font_preview_path(i),32,glyphs,224);}ThemePanel(row,.15f,8,control);DrawTextEx(fontPreview[i],choiceLabels[i],(Vector2){row.x+10,row.y+7},17,0,AppTheme.text);}

 }
 if(count>visible){static bool dragging=false;static float grab=0;float track=choiceList.height-8,h=fmaxf(22,track*visible/count);Rectangle thumb={choiceList.x+choiceList.width-10,choiceList.y+4+(track-h)*choiceScroll/(count-visible),6,h};choiceThumb=thumb;Rectangle hit={thumb.x-4,thumb.y,14,h};if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),hit)){dragging=true;grab=GetMouseY()-thumb.y;}if(dragging){if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))choiceScroll=Clamp(roundf((GetMouseY()-choiceList.y-4-grab)/(track-h)*(count-visible)),0,count-visible);else dragging=false;}ThemePanel(thumb,1,6,dragging?AppTheme.border:muted);}
 if(IsKeyPressed(KEY_ESCAPE)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),choiceList)&&!CheckCollisionPointRec(GetMousePosition(),box)))choiceOpen=-1;
 choiceRendering=false;modalDrawing=modal;inputClip=old;
}
static int BoolChoice(int id,int x,int* y,int w,const char* label,bool value){const char* names[]={lab_tr("Disattivo"),lab_tr("Attivo")};return Choice(id,x,y,w,label,names,2,value);}

// A shared compact slider. Drafts belong to the caller until mouse release.
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
