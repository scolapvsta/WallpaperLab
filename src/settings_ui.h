// Deferred overlay menus: opening a choice never changes document geometry.
static int choiceScroll=0,choicePending=-1,choiceValue=-1;
static Rectangle choiceControls[64],choiceRows[64][96];static int choiceCounts[64];
static bool choiceWheel=false,choiceMulti=false;static bool* choiceChecks=NULL;
static char choiceLabels[96][256];static int choiceSelected=0;static Rectangle choiceList;
static int Choice(int id,int x,int* py,int w,const char* label,const char** options,int count,int selected){
 int result=selected;if(choicePending==id){result=choiceValue;choicePending=-1;}
 int y=*py;int cw=fminf(292,w*.60f);UiText(label,x,y+9,14,AppTheme.text);Rectangle box={x+w-cw,y,cw,34};choiceControls[id]=box;choiceCounts[id]=count;*py+=43;
 choiceRegistering=choiceOpen<0||choiceOpen==id||!CheckCollisionPointRec(GetMousePosition(),choiceList);bool clicked=Button(box,selected>=0&&selected<count?options[selected]:"—",choiceOpen==id,!applying);choiceRegistering=false;
 if(clicked){choiceOpen=choiceOpen==id?-1:id;choiceScroll=Clamp(selected-2,0,fmaxf(0,count-7));}
 if(choiceOpen==id){choiceSelected=result;choiceMulti=false;choiceChecks=NULL;for(int n=0;n<count&&n<96;n++)snprintf(choiceLabels[n],256,"%s",options[n]);}
 DrawLineEx((Vector2){box.x+box.width-15,y+14},(Vector2){box.x+box.width-10,y+19},1.5f,muted);return result;
}
static void MultiChoice(int id,int x,int* y,int w,const char* label,const char** options,int count,bool* selected){
 if(choicePending==id){selected[choiceValue]=!selected[choiceValue];choicePending=-1;}
 int number=0;for(int n=0;n<count;n++)number+=selected[n];const char* summary=number?TextFormat("%d selected",number):lab_tr("Tutti");
 // Reuse field geometry and overlay lifecycle without borrowing stack pointers.
 int ignored=Choice(id,x,y,w,label,options,count,-1);(void)ignored;
 Rectangle box=choiceControls[id];ThemePanel(box,.2f,8,control);ButtonText(summary,box.x+10,box.y+10,AppTheme.text);
 if(choiceOpen==id){choiceMulti=true;choiceChecks=NULL;for(int n=0;n<count;n++)snprintf(choiceLabels[n],256,"%s %s",selected[n]?"✓":"  ",options[n]);}
}
static void ChoiceOverlay(void){
 if(choiceOpen<0)return;int id=choiceOpen,count=choiceCounts[id],visible=fminf(7,count);Rectangle box=choiceControls[id];
 choiceList=(Rectangle){box.x,box.y+box.height+4,fmaxf(box.width,240),visible*34+8};choiceList.x=Clamp(choiceList.x,8,GetScreenWidth()-choiceList.width-8);if(choiceList.y+choiceList.height>GetScreenHeight()-12)choiceList.y=box.y-choiceList.height-4;
 Rectangle old=inputClip;inputClip=(Rectangle){0,0,GetScreenWidth(),GetScreenHeight()};bool modal=modalDrawing;modalDrawing=true;choiceRendering=true;
 ThemePanel((Rectangle){choiceList.x-3,choiceList.y-3,choiceList.width+6,choiceList.height+6},.06f,8,lineColor);ThemePanel(choiceList,.06f,8,surface);
 if(CheckCollisionPointRec(GetMousePosition(),choiceList))choiceScroll=Clamp(choiceScroll-GetMouseWheelMove(),0,fmaxf(0,count-visible));
 memset(choiceRows[id],0,sizeof(choiceRows[id]));for(int n=0;n<visible;n++){int i=n+choiceScroll;Rectangle row={choiceList.x+4,choiceList.y+4+n*34,choiceList.width-8,32};choiceRows[id][i]=row;
  if(Button(row,choiceLabels[i],!choiceMulti&&i==choiceSelected,true)){choicePending=id;choiceValue=i;if(!choiceMulti)choiceOpen=-1;}
  if(id==33){if(!fontPreview[i].texture.id){int glyphs[224];for(int n=0;n<224;n++)glyphs[n]=n+32;fontPreview[i]=LoadSystemFont(lab_font_preview_path(i),32,glyphs,224);}ThemePanel(row,.15f,8,control);DrawTextEx(fontPreview[i],choiceLabels[i],(Vector2){row.x+10,row.y+7},17,0,AppTheme.text);}

 }
 if(IsKeyPressed(KEY_ESCAPE)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),choiceList)&&!CheckCollisionPointRec(GetMousePosition(),box)))choiceOpen=-1;
 choiceRendering=false;modalDrawing=modal;inputClip=old;
}
static int BoolChoice(int id,int x,int* y,int w,const char* label,bool value){const char* names[]={lab_tr("Disattivo"),lab_tr("Attivo")};return Choice(id,x,y,w,label,names,2,value);}
