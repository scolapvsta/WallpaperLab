// Creator-defined controls. Changes persist per project and reapply on commit.
static int creatorItem=-1,creatorDrag=-1,creatorCombo=-1,creatorEdit=-1;
static float creatorDraft=0;
static double creatorTimeMs=0;
static Rectangle creatorColorSwatch,creatorColorBox,creatorHue,creatorRgb[3],creatorReset;
static Rectangle creatorBinaryRows[256];static int creatorBinaryProps[256],creatorBinaryValues[256],creatorBinaryCount;
static char creatorText[4097];static TextEdit creatorEditor;
static void CreatorCommit(int i,int ok){
    if(ok&&!uiSmoke)applying=lab_start(i,0);
    if(!ok)snprintf(notice,sizeof(notice),"%s",lab_error());
}
static int CreatorControls(int i,int x,int y,int w){
    if(creatorItem!=i){creatorItem=i;creatorDrag=creatorCombo=creatorEdit=-1;colorPickerOpen=colorPickerDrag=-1;}
    creatorBinaryCount=0;double began=GetTime();
    int start=y,count=lab_property_count(i);if(!count)return 0;
    y+=TextLines(lab_tr("Controlli definiti dal creator. Le modifiche salvano e ricaricano la scena."),x,y,w,12,2,muted)+10;
    for(int p=0;p<count;p++){
        if(!lab_property_visible(i,p))continue;
        char type[32];snprintf(type,sizeof(type),"%s",lab_property_type(i,p));
        const char* label=lab_property_label(i,p);
        if(!strcmp(type,"text")||!strcmp(type,"group")){y+=TextLines(label,x,y,w,14,2,muted)+10;continue;}
        if(!strcmp(type,"bool")){
            bool on=lab_property_number(i,p)!=0;Rectangle row={x,y,w,40};if(creatorBinaryCount<256){creatorBinaryRows[creatorBinaryCount]=row;creatorBinaryProps[creatorBinaryCount]=p;creatorBinaryValues[creatorBinaryCount++]=on;}
            bool next=SwitchToggle(row,label,on);if(next!=on)CreatorCommit(i,lab_property_number_set(i,p,next));y+=43;continue;
        }
        if(!strcmp(type,"combo")&&lab_property_options(i,p)==2){
            int at=lab_property_choice(i,p);char optionText[2][512];for(int n=0;n<2;n++)snprintf(optionText[n],512,"%s",lab_property_option(i,p,n));const char* options[]={optionText[0],optionText[1]};int height=ToggleHeight(label,options,w);Rectangle row={x,y,w,height};
            if(creatorBinaryCount<256){creatorBinaryRows[creatorBinaryCount]=row;creatorBinaryProps[creatorBinaryCount]=p;creatorBinaryValues[creatorBinaryCount++]=at;}
            bool next=ToggleRow(row,label,at>=0?options[at]:lab_property_text(i,p),at==1,!applying);if(next!=(at==1)){CreatorCommit(i,lab_property_choice_set(i,p,at<0?0:1-at));creatorCombo=-1;}y+=height+3;continue;
        }
        y+=TextLines(label,x,y,w,14,2,AppTheme.text)+4;
        if(lab_property_scheme(i,p))y+=TextLines(lab_tr("Colore definito dal creator: modifica la scena solo se il progetto lo utilizza. Non cambia il tema dell’app."),x,y,w,12,4,muted)+8;
        if(!strcmp(type,"slider")){
            double lo=lab_property_bound(i,p,0),hi=lab_property_bound(i,p,1);
            float value=creatorDrag==p?creatorDraft:lab_property_number(i,p);
            Rectangle track={x+6,y+22,w-12,5};bool hot=Hot((Rectangle){x,y,w,40},!applying);
            if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))creatorDrag=p;
            if(creatorDrag==p){creatorDraft=value=lo+Clamp((GetMouseX()-track.x)/track.width,0,1)*(hi-lo);if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){creatorDrag=-1;CreatorCommit(i,lab_property_number_set(i,p,value));}}
            UiText(lo==0&&hi==1?TextFormat("%.0f%%",value*100):TextFormat("%.3g",value),x,y,13,muted);ThemePanel(track,1,8,controlHover);
            DrawCircle(track.x+(hi>lo?(value-lo)/(hi-lo):0)*track.width,track.y+2,6,AppTheme.text);y+=40;
        }else if(!strcmp(type,"combo")){
            int choice=lab_property_choice(i,p);const char* value=choice<0?lab_property_text(i,p):lab_property_option(i,p,choice);
            if(Button((Rectangle){x,y,w,34},value,creatorCombo==p,!applying))creatorCombo=creatorCombo==p?-1:p;y+=40;
            if(creatorCombo==p)for(int o=0;o<lab_property_options(i,p);o++){if(Button((Rectangle){x+8,y,w-8,32},lab_property_option(i,p,o),choice==o,!applying)){CreatorCommit(i,lab_property_choice_set(i,p,o));creatorCombo=-1;}y+=38;}
        }else if(!strcmp(type,"color")){
            float red=0,green=0,blue=0;sscanf(lab_property_text(i,p),"%f %f %f",&red,&green,&blue);
            Color current={(unsigned char)roundf(Clamp(red,0,1)*255),(unsigned char)roundf(Clamp(green,0,1)*255),(unsigned char)roundf(Clamp(blue,0,1)*255),255};
            Color chosen=current;if(VisualColorPicker(p,x,&y,w,current,&chosen,&detailScroll)){snprintf(creatorText,sizeof(creatorText),"%.6f %.6f %.6f",chosen.r/255.f,chosen.g/255.f,chosen.b/255.f);CreatorCommit(i,lab_property_text_set(i,p,creatorText));}
            creatorColorSwatch=colorPickerSwatch;creatorColorBox=colorPickerBox;creatorHue=colorPickerHue;memcpy(creatorRgb,colorPickerRgb,sizeof(creatorRgb));
        }else if(!strcmp(type,"textinput")){
            if(creatorEdit!=p&&Button((Rectangle){x,y,w,30},lab_property_text(i,p),false,!applying)){creatorEdit=p;snprintf(creatorText,sizeof(creatorText),"%s",lab_property_text(i,p));EditFocus(&creatorEditor,creatorText,true);}
            if(creatorEdit==p){
                ThemePanel((Rectangle){x,y,w,30},.04f,6,controlHover);EditInput(&creatorEditor,creatorText,sizeof(creatorText));EditDraw(&creatorEditor,creatorText,(Rectangle){x+8,y+2,w-16,26},14,true);BeginScissorMode(inputClip.x,inputClip.y,inputClip.width,inputClip.height);
                if(IsKeyPressed(KEY_ENTER)){CreatorCommit(i,lab_property_text_set(i,p,creatorText));creatorEdit=-1;}
                if(IsKeyPressed(KEY_ESCAPE))creatorEdit=-1;
            }y+=36;
        }else{y+=TextLines(lab_tr("Selezione file/materiali non ancora disponibile."),x,y,w,12,2,muted)+12;}
    }

    creatorTimeMs=(GetTime()-began)*1000;return y-start;
}

static int CreatorReset(int i,int x,int y,int w){
    if(!lab_property_count(i))return 0;Color previous=AppTheme.border;AppTheme.border=(Color){209,64,77,255};
    creatorReset=(Rectangle){x,y+10,w,32};
    if(DangerButton(creatorReset,lab_tr("Ripristina proprietà del creator"),!applying)){colorPickerOpen=colorPickerDrag=-1;CreatorCommit(i,lab_properties_reset(i));}
    AppTheme.border=previous;return 54;
}
