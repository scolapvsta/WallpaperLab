// Shared visual HSV/RGB picker for project properties and native login widgets.
static int colorPickerOpen=-1,colorPickerDrag=-1,colorPickerNumeric=-1,colorPickerPicking=-1,colorPickerPicked=-1;
static Color colorPickerDraft,colorPickerSample;static Vector3 colorPickerHsv;static char colorPickerNumbers[3][8];static TextEdit colorPickerEdits[3];
static Rectangle colorPickerSwatch,colorPickerBox,colorPickerHue,colorPickerRgb[3],colorPickerInputs[3],colorPickerEye;
static void ColorPickerSync(void){snprintf(colorPickerNumbers[0],8,"%d",colorPickerDraft.r);snprintf(colorPickerNumbers[1],8,"%d",colorPickerDraft.g);snprintf(colorPickerNumbers[2],8,"%d",colorPickerDraft.b);}
static void ColorPickerPoll(void){if(colorPickerPicking<0)return;unsigned value=0;int result=lab_color_pick_poll(&value);if(!result)return;if(result>0&&colorPickerOpen==colorPickerPicking){colorPickerPicked=colorPickerPicking;colorPickerSample=(Color){value>>16,(value>>8)&255,value&255,255};}colorPickerPicking=-1;}
static bool ColorPickerEscape(void){if(colorPickerNumeric>=0){colorPickerNumeric=-1;ColorPickerSync();return true;}if(colorPickerOpen>=0){colorPickerOpen=-1;colorPickerDrag=-1;return true;}return false;}
static bool VisualColorPicker(int id,int x,int* y,int w,Color current,Color* result,float* scroll){
 bool changed=false,opened=false;Color shown=colorPickerOpen==id?colorPickerDraft:current;
 colorPickerSwatch=(Rectangle){x,*y,w,36};
 if(Button(colorPickerSwatch,lab_tr("Scegli colore"),colorPickerOpen==id,!applying)){if(colorPickerOpen==id)colorPickerOpen=-1;else{colorPickerOpen=id;opened=true;colorPickerDraft=current;colorPickerHsv=ColorToHSV(current);ColorPickerSync();}colorPickerNumeric=-1;colorPickerDrag=-1;}
 Rectangle swatch={x+8,*y+6,38,24};DrawRectangleRounded(swatch,.2,8,shown);DrawRectangleRoundedLinesEx(swatch,.2,8,1,muted);*y+=44;
 if(colorPickerOpen!=id)return false;
 if(colorPickerPicked==id){colorPickerDraft=colorPickerSample;colorPickerHsv=ColorToHSV(colorPickerDraft);ColorPickerSync();colorPickerPicked=-1;changed=true;}
 Vector3 hsv=colorPickerHsv;Rectangle square={x+4,*y,w-38,110},hue={x+w-23,*y,15,110};colorPickerBox=square;colorPickerHue=hue;
 DrawRectangleRec(square,ColorFromHSV(hsv.x,1,1));DrawRectangleGradientEx(square,WHITE,WHITE,Fade(WHITE,0),Fade(WHITE,0));DrawRectangleGradientV(square.x,square.y,square.width,square.height,Fade(BLACK,0),BLACK);
 for(int n=0;n<6;n++)DrawRectangleGradientV(hue.x,hue.y+n*hue.height/6,hue.width,hue.height/6+1,ColorFromHSV(n*60,1,1),ColorFromHSV((n+1)*60,1,1));
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){if(Hot(square,!applying))colorPickerDrag=0;else if(Hot(hue,!applying))colorPickerDrag=1;}
 if(colorPickerDrag==0){hsv.y=Clamp((GetMouseX()-square.x)/square.width,0,1);hsv.z=1-Clamp((GetMouseY()-square.y)/square.height,0,1);}
 if(colorPickerDrag==1)hsv.x=Clamp((GetMouseY()-hue.y)/hue.height,0,1)*359.9;
 if(colorPickerDrag==0||colorPickerDrag==1){colorPickerDraft=ColorFromHSV(hsv.x,hsv.y,hsv.z);colorPickerHsv=hsv;}
 DrawCircleLines(square.x+hsv.y*square.width,square.y+(1-hsv.z)*square.height,5,WHITE);DrawLineEx((Vector2){hue.x-2,hue.y+hsv.x/360*hue.height},(Vector2){hue.x+hue.width+2,hue.y+hsv.x/360*hue.height},2,AppTheme.text);*y+=120;
 if(colorPickerNumeric>=0&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),colorPickerInputs[colorPickerNumeric])){
  char* end;long value=strtol(colorPickerNumbers[colorPickerNumeric],&end,10);if(*colorPickerNumbers[colorPickerNumeric]&&!*end&&value>=0&&value<=255){unsigned char* values[]={&colorPickerDraft.r,&colorPickerDraft.g,&colorPickerDraft.b};*values[colorPickerNumeric]=value;changed=true;colorPickerHsv=ColorToHSV(colorPickerDraft);}colorPickerNumeric=-1;ColorPickerSync();
 }
 const char* channels[]={"R","G","B"};for(int c=0;c<3;c++){
  unsigned char* values[]={&colorPickerDraft.r,&colorPickerDraft.g,&colorPickerDraft.b};Rectangle box={x+18,*y,52,28},track={x+84,*y+12,w-94,5};colorPickerRgb[c]=track;colorPickerInputs[c]=box;
  UiText(channels[c],x,*y+8,12,AppTheme.text);bool focused=colorPickerNumeric==c;
  if(Hot(box,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ColorPickerSync();colorPickerNumeric=c;EditFocus(&colorPickerEdits[c],colorPickerNumbers[c],true);focused=true;}
  if(!focused)snprintf(colorPickerNumbers[c],8,"%d",*values[c]);
  char* end;long number=strtol(colorPickerNumbers[c],&end,10);bool valid=*colorPickerNumbers[c]&&!*end&&number>=0&&number<=255;
  ThemePanel(box,.08,6,focused?controlHover:control);DrawRectangleLinesEx(box,focused?2:1,valid?(focused?AppTheme.border:lineColor):RED);
  if(focused){EditInput(&colorPickerEdits[c],colorPickerNumbers[c],8);number=strtol(colorPickerNumbers[c],&end,10);valid=*colorPickerNumbers[c]&&!*end&&number>=0&&number<=255;
   if(IsKeyPressed(KEY_ENTER)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),box))){if(valid){*values[c]=number;changed=true;colorPickerHsv=ColorToHSV(colorPickerDraft);}colorPickerNumeric=-1;ColorPickerSync();}
  }
  EditDraw(&colorPickerEdits[c],colorPickerNumbers[c],(Rectangle){box.x+6,box.y+2,box.width-12,24},13,focused);BeginScissorMode(inputClip.x,inputClip.y,inputClip.width,inputClip.height);
  ThemePanel(track,1,6,controlHover);if(Hot((Rectangle){track.x-4,*y,track.width+8,28},true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))colorPickerDrag=2+c;
  if(colorPickerDrag==2+c)*values[c]=roundf(Clamp((GetMouseX()-track.x)/track.width,0,1)*255);DrawCircle(track.x+*values[c]/255.f*track.width,track.y+2,5,AppTheme.text);*y+=34;
 }
 if(colorPickerDrag>=0){colorPickerNumeric=-1;ColorPickerSync();if(colorPickerDrag>=2){Vector3 rgb=ColorToHSV(colorPickerDraft);if(rgb.y<.001)rgb.x=colorPickerHsv.x;colorPickerHsv=rgb;}}
 if(colorPickerDrag>=0&&!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){colorPickerDrag=-1;changed=true;}
 colorPickerEye=(Rectangle){x,*y,w,32};if(Button(colorPickerEye,lab_tr("Contagocce sullo schermo"),colorPickerPicking==id,lab_color_pick_available()&&colorPickerPicking<0)){colorPickerNumeric=-1;if(lab_color_pick_start())colorPickerPicking=id;}*y+=38;
 if(!lab_color_pick_available()){*y+=TextLines(lab_tr("Contagocce: richiede Hyprpicker su Wayland."),x,*y,w,12,2,muted)+6;}
 if(changed)*result=colorPickerDraft;
 *y+=8;if(opened&&scroll)*scroll+=fmaxf(0,*y-(inputClip.y+inputClip.height)+12);return changed;
}
