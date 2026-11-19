// Shared visual HSV/RGB picker for project properties and native login widgets.
static int colorPickerOpen=-1,colorPickerDrag=-1;static Color colorPickerDraft;static Vector3 colorPickerHsv;
static Rectangle colorPickerSwatch,colorPickerBox,colorPickerHue,colorPickerRgb[3];
static bool VisualColorPicker(int id,int x,int* y,int w,Color current,Color* result,float* scroll){
 bool changed=false,opened=false;Color shown=colorPickerOpen==id?colorPickerDraft:current;
 colorPickerSwatch=(Rectangle){x,*y,w,36};
 if(Button(colorPickerSwatch,lab_tr("Scegli colore"),colorPickerOpen==id,!applying)){if(colorPickerOpen==id)colorPickerOpen=-1;else{colorPickerOpen=id;opened=true;colorPickerDraft=current;colorPickerHsv=ColorToHSV(current);}colorPickerDrag=-1;}
 Rectangle swatch={x+8,*y+6,38,24};DrawRectangleRounded(swatch,.2,8,shown);DrawRectangleRoundedLinesEx(swatch,.2,8,1,muted);*y+=44;
 if(colorPickerOpen!=id)return false;
 Vector3 hsv=colorPickerHsv;Rectangle square={x+4,*y,w-38,110},hue={x+w-23,*y,15,110};colorPickerBox=square;colorPickerHue=hue;
 DrawRectangleRec(square,ColorFromHSV(hsv.x,1,1));DrawRectangleGradientEx(square,WHITE,WHITE,Fade(WHITE,0),Fade(WHITE,0));DrawRectangleGradientV(square.x,square.y,square.width,square.height,Fade(BLACK,0),BLACK);
 for(int n=0;n<6;n++)DrawRectangleGradientV(hue.x,hue.y+n*hue.height/6,hue.width,hue.height/6+1,ColorFromHSV(n*60,1,1),ColorFromHSV((n+1)*60,1,1));
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){if(Hot(square,!applying))colorPickerDrag=0;else if(Hot(hue,!applying))colorPickerDrag=1;}
 if(colorPickerDrag==0){hsv.y=Clamp((GetMouseX()-square.x)/square.width,0,1);hsv.z=1-Clamp((GetMouseY()-square.y)/square.height,0,1);}
 if(colorPickerDrag==1)hsv.x=Clamp((GetMouseY()-hue.y)/hue.height,0,1)*359.9;
 if(colorPickerDrag==0||colorPickerDrag==1){colorPickerDraft=ColorFromHSV(hsv.x,hsv.y,hsv.z);colorPickerHsv=hsv;}
 DrawCircleLines(square.x+hsv.y*square.width,square.y+(1-hsv.z)*square.height,5,WHITE);DrawLineEx((Vector2){hue.x-2,hue.y+hsv.x/360*hue.height},(Vector2){hue.x+hue.width+2,hue.y+hsv.x/360*hue.height},2,AppTheme.text);*y+=120;
 const char* channels[]={"R","G","B"};for(int c=0;c<3;c++){unsigned char* values[]={&colorPickerDraft.r,&colorPickerDraft.g,&colorPickerDraft.b};Rectangle track={x+58,*y+10,w-68,5};colorPickerRgb[c]=track;UiText(TextFormat("%s %3d",channels[c],*values[c]),x,*y+2,12,AppTheme.text);ThemePanel(track,1,6,controlHover);if(Hot((Rectangle){track.x-4,*y,track.width+8,24},true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))colorPickerDrag=2+c;if(colorPickerDrag==2+c)*values[c]=roundf(Clamp((GetMouseX()-track.x)/track.width,0,1)*255);DrawCircle(track.x+*values[c]/255.f*track.width,track.y+2,5,AppTheme.text);*y+=28;}
 if(colorPickerDrag>=2){Vector3 rgb=ColorToHSV(colorPickerDraft);if(rgb.y<.001)rgb.x=colorPickerHsv.x;colorPickerHsv=rgb;}
 if(colorPickerDrag>=0&&!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){colorPickerDrag=-1;changed=current.r!=colorPickerDraft.r||current.g!=colorPickerDraft.g||current.b!=colorPickerDraft.b;*result=colorPickerDraft;}
 *y+=8;if(opened&&scroll)*scroll+=fmaxf(0,*y-(inputClip.y+inputClip.height)+12);return changed;
}
