// Creator-defined controls. Changes persist per project and reapply on commit.
static int creatorItem=-1,creatorDrag=-1,creatorCombo=-1,creatorEdit=-1;
static float creatorDraft=0;
static double creatorTimeMs=0;
static int creatorColor=-1,creatorColorDrag=-1;static Color creatorColorDraft;static Vector3 creatorColorHsv;
static Rectangle creatorColorSwatch,creatorColorBox,creatorHue,creatorRgb[3],creatorReset;
static char creatorText[4097];static TextEdit creatorEditor;
static void CreatorCommit(int i,int ok){
    if(ok&&!uiSmoke)applying=lab_start(i,0);
    if(!ok)snprintf(notice,sizeof(notice),"%s",lab_error());
}
static int CreatorControls(int i,int x,int y,int w){
    if(creatorItem!=i){creatorItem=i;creatorDrag=creatorCombo=creatorEdit=creatorColor=creatorColorDrag=-1;}
    double began=GetTime();
    int start=y,count=lab_property_count(i);if(!count)return 0;
    y+=TextLines(lab_tr("Controlli definiti dal creator. Le modifiche salvano e ricaricano la scena."),x,y,w,12,2,muted)+10;
    for(int p=0;p<count;p++){
        if(!lab_property_visible(i,p))continue;
        char type[32];snprintf(type,sizeof(type),"%s",lab_property_type(i,p));
        const char* label=lab_property_label(i,p);
        if(!strcmp(type,"text")||!strcmp(type,"group")){y+=TextLines(label,x,y,w,14,2,muted)+10;continue;}
        if(!strcmp(type,"bool")){
            bool on=lab_property_number(i,p)!=0;TextLines(label,x,y+5,w-106,14,2,AppTheme.text);
            if(Button((Rectangle){x+w-96,y,96,28},on?lab_tr("Attivo"):lab_tr("Disattivo"),on,!applying))CreatorCommit(i,lab_property_number_set(i,p,!on));y+=36;continue;
        }
        y+=TextLines(label,x,y,w,14,2,AppTheme.text)+4;
        if(lab_property_scheme(i,p))y+=TextLines(lab_tr("Colore definito dal creator: modifica la scena solo se il progetto lo utilizza. Non cambia il tema dell’app."),x,y,w,12,4,muted)+8;
        if(!strcmp(type,"bool")){
            bool on=lab_property_number(i,p)!=0;
            if(Button((Rectangle){x,y,w,32},on?lab_tr("Attivo"):lab_tr("Disattivo"),on,!applying))CreatorCommit(i,lab_property_number_set(i,p,!on));y+=36;
        }else if(!strcmp(type,"slider")){
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
            Color shown=creatorColor==p?creatorColorDraft:current;
            Rectangle swatch={x+8,y+5,28,22};creatorColorSwatch=(Rectangle){x,y,w,32};
            if(Button(creatorColorSwatch,TextFormat("#%02X%02X%02X",shown.r,shown.g,shown.b),false,!applying)){
                if(creatorColor==p)creatorColor=-1;else{creatorColor=p;creatorColorDraft=current;creatorColorHsv=ColorToHSV(current);detailScroll+=fmaxf(0,y+210-(inputClip.y+inputClip.height)+12);}
            }DrawRectangleRounded(swatch,.2f,8,shown);DrawRectangleRoundedLinesEx(swatch,.2f,8,1,muted);y+=38;
            if(creatorColor==p){
                Vector3 hsv=creatorColorHsv;Rectangle square={x+4,y,w-38,86},hue={x+w-23,y,15,86};creatorColorBox=square;creatorHue=hue;
                // Two gradients form the saturation/value plane: hue under white and black.
                DrawRectangleRec(square,ColorFromHSV(hsv.x,1,1));DrawRectangleGradientEx(square,WHITE,WHITE,Fade(WHITE,0),Fade(WHITE,0));
                DrawRectangleGradientV(square.x,square.y,square.width,square.height,Fade(BLACK,0),BLACK);
                for(int n=0;n<6;n++)DrawRectangleGradientV(hue.x,hue.y+n*hue.height/6,hue.width,hue.height/6+1,ColorFromHSV(n*60,1,1),ColorFromHSV((n+1)*60,1,1));
                bool change=false;
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){if(Hot(square,!applying))creatorColorDrag=0;else if(Hot(hue,!applying))creatorColorDrag=1;}
                if(creatorColorDrag==0){hsv.y=Clamp((GetMouseX()-square.x)/square.width,0,1);hsv.z=1-Clamp((GetMouseY()-square.y)/square.height,0,1);change=true;}
                if(creatorColorDrag==1){hsv.x=Clamp((GetMouseY()-hue.y)/hue.height,0,1)*359.9f;change=true;}
                if(change){creatorColorDraft=ColorFromHSV(hsv.x,hsv.y,hsv.z);creatorColorHsv=hsv;}
                DrawCircleLines(square.x+hsv.y*square.width,square.y+(1-hsv.z)*square.height,4,WHITE);DrawLineEx((Vector2){hue.x-2,hue.y+hsv.x/360*hue.height},(Vector2){hue.x+hue.width+2,hue.y+hsv.x/360*hue.height},2,AppTheme.text);y+=94;
                const char* channels[]={"R","G","B"};
                for(int c=0;c<3;c++){
                    unsigned char* values[]={&creatorColorDraft.r,&creatorColorDraft.g,&creatorColorDraft.b};Rectangle track={x+58,y+8,w-68,5};creatorRgb[c]=track;
                    UiText(TextFormat("%s %3d",channels[c],*values[c]),x,y,12,AppTheme.text);ThemePanel(track,1,6,controlHover);
                    if(Hot((Rectangle){track.x-4,y,track.width+8,20},!applying)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))creatorColorDrag=2+c;
                    if(creatorColorDrag==2+c)*values[c]=(unsigned char)roundf(Clamp((GetMouseX()-track.x)/track.width,0,1)*255);
                    DrawCircle(track.x+*values[c]/255.f*track.width,track.y+2,5,AppTheme.text);y+=22;
                }
                if(creatorColorDrag>=2){Vector3 rgbHsv=ColorToHSV(creatorColorDraft);if(rgbHsv.y<.001f)rgbHsv.x=creatorColorHsv.x;creatorColorHsv=rgbHsv;}
                if(creatorColorDrag>=0&&!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
                    creatorColorDrag=-1;snprintf(creatorText,sizeof(creatorText),"%.6f %.6f %.6f",creatorColorDraft.r/255.f,creatorColorDraft.g/255.f,creatorColorDraft.b/255.f);
                    if(current.r!=creatorColorDraft.r||current.g!=creatorColorDraft.g||current.b!=creatorColorDraft.b)CreatorCommit(i,lab_property_text_set(i,p,creatorText));
                }y+=4;
            }
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
    if(DangerButton(creatorReset,lab_tr("Ripristina proprietà del creator"),!applying)){creatorColor=creatorColorDrag=-1;CreatorCommit(i,lab_properties_reset(i));}
    AppTheme.border=previous;return 54;
}
