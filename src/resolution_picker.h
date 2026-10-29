static Rectangle groupChecks[6];
static void ResolutionPicker(int x,int* py,int w,const char** labels,const int* groups,bool* selected,int count,bool* whole){
 const char* names[]={"Monitor singolo","Dual monitor","Ultrawide / panoramico","Triplo monitor","Verticale","Altre informazioni"};
 for(int g=0;g<6;g++){int number=0,checked=0;for(int n=0;n<count;n++)if(groups[n]==g){number++;checked+=selected[n];}if(!number)continue;
  int y=*py;Rectangle header={x,y,w,32};groupChecks[g]=header;ThemePanel(header,.15f,8,control);
  Icon(g==0?UI_DISPLAY:UI_CAROUSEL,(Vector2){x+14,y+16},muted,false);UiText(lab_tr(names[g]),x+32,y+10,13,AppTheme.text);
  Rectangle check={x+w-24,y+9,14,14};DrawRectangleRoundedLinesEx(check,.2f,4,1.2f,checked==number?AppTheme.border:muted);if(checked==number)DrawRectangle(check.x+3,check.y+3,8,8,AppTheme.border);else if(checked)DrawRectangle(check.x+3,check.y+6,8,2,muted);
  if(Hot(header,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){bool on=checked!=number;for(int n=0;n<count;n++)if(groups[n]==g)selected[n]=on;whole[g]=on;}
  *py+=38;
  for(int n=0;n<count;n++)if(groups[n]==g){Rectangle row={x+8,*py,w-8,28};if(Hot(row,true)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){selected[n]=!selected[n];whole[g]=false;}DrawRectangleRoundedLinesEx((Rectangle){x+14,*py+7,14,14},.1f,4,1.2f,selected[n]?AppTheme.border:muted);if(selected[n])DrawRectangle(x+17,*py+10,8,8,AppTheme.border);UiText(labels[n],x+40,*py+7,13,AppTheme.text);*py+=31;}
  *py+=10;
 }
}
