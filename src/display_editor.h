// Wallpaper-only display calibration. Coordinates are mm or logical pixels;
// native surface sizes, cursor paths and the compositor layout never change.
static float displayContentHeight=680;
static Rectangle displaySaveButton,displayCards[32];
static int DisplayEditor(int x,int y,int w){
    int start=y;if(displayDragging<0&&lab_mode()==2)displaySelected=lab_screen();
    UiText(lab_tr("Disposizione dei monitor"),x,y,26,AppTheme.text);y+=36;
    const char* modes[]={lab_tr("Panoramico"),lab_tr("Duplicato"),lab_tr("Un monitor")};int at=lab_mode();int next=Choice(1,x,&y,w,lab_tr("Modalita"),modes,3,at);if(next!=at){lab_set_mode(next);if(next==2)lab_set_screen(displaySelected);}
    Rectangle chart={x,y,w,Clamp(inputClip.height-350,130,200)};ThemePanel(chart,0.03f,12,control);
    UiText(lab_display_physical()?lab_tr("Dimensioni reali · trascina i monitor"):lab_tr("Pixel logici · trascina i monitor"),chart.x+12,chart.y+8,12,muted);
    Rectangle parent=inputClip;
    float clipTop=fmaxf(chart.y,parent.y),clipBottom=fminf(chart.y+chart.height,parent.y+parent.height);
    BeginScissorMode(chart.x,clipTop,chart.width,fmaxf(0,clipBottom-clipTop));
    // Stable transform during a drag keeps the dragged panel under the pointer.
    float minX=1e9,minY=1e9,maxX=-1e9,maxY=-1e9;
    for(int i=0;i<lab_monitor_count();i++){float px,py,pw,ph;if(lab_display_rect(i,&px,&py,&pw,&ph)){minX=fminf(minX,px);minY=fminf(minY,py);maxX=fmaxf(maxX,px+pw);maxY=fmaxf(maxY,py+ph);}}
    if(displaySelected>=lab_monitor_count())displaySelected=0;
    if(displayDragging<0&&lab_monitor_count()){
        displayMapScale=fminf((chart.width-80)/fmaxf(1,maxX-minX),(chart.height-60)/fmaxf(1,maxY-minY));
        displayMapOrigin=(Vector2){chart.x+(chart.width-(maxX-minX)*displayMapScale)/2-minX*displayMapScale,chart.y+(chart.height-(maxY-minY)*displayMapScale)/2-minY*displayMapScale};
    }
    // Light guides make top/center/bottom relationships visible.
    for(int n=1;n<4;n++)DrawLine(chart.x+12,chart.y+n*chart.height/4,chart.x+chart.width-12,chart.y+n*chart.height/4,Fade(muted,0.12f));
    if(displayDragging>=0){
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&!applying){
            Vector2 mouse=GetMousePosition();
            float snap=(IsKeyDown(KEY_LEFT_ALT)||IsKeyDown(KEY_RIGHT_ALT))?0:8/fmaxf(0.001f,displayMapScale);
            lab_display_move(displayDragging,displayDragPosition.x+(mouse.x-displayDragMouse.x)/displayMapScale,displayDragPosition.y+(mouse.y-displayDragMouse.y)/displayMapScale,snap);
            SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
        }else displayDragging=-1;
    }
    for(int i=0;i<lab_monitor_count();i++){
        float px,py,pw,ph;if(!lab_display_rect(i,&px,&py,&pw,&ph))continue;
        Rectangle r={displayMapOrigin.x+px*displayMapScale,displayMapOrigin.y+py*displayMapScale,pw*displayMapScale,ph*displayMapScale};
        if(i<32)displayCards[i]=r;
        bool hot=Hot(r,!applying)&&CheckCollisionPointRec(GetMousePosition(),chart);
        bool selected=displaySelected==i;
        ThemePanel(r,0.06f,8,surface);
        DrawRectangleRoundedLinesEx(r,0.06f,8,selected?2.5f:1.4f,selected?AppTheme.border:muted);
        if(r.width>100&&r.height>70){
            const char* title=TextFormat("%d · %s",i+1,lab_monitor_name(i));
            UiText(title,r.x+(r.width-UiMeasure(title,17))/2,r.y+r.height/2-26,17,AppTheme.text);
            TextLines(lab_display_info(i),r.x+12,r.y+r.height/2+1,r.width-24,13,2,muted);
        }
        if(hot){
            SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                displaySelected=displayDragging=i;displayDragMouse=GetMousePosition();displayDragPosition=(Vector2){px,py};
                if(lab_mode()==2)lab_set_screen(i);
            }
        }
    }
    EndScissorMode();BeginScissorMode(parent.x,parent.y,parent.width,parent.height);
    if(!lab_monitor_count())UiText(lab_tr("Nessun monitor disponibile"),x+24,y+90,18,muted);
    y+=chart.height+12;
    if(lab_monitor_count()){
        float px,py,pw,ph;lab_display_rect(displaySelected,&px,&py,&pw,&ph);
        UiText(TextFormat(lab_display_physical()?lab_tr("Posizione: %.1f, %.1f mm · dimensioni rilevate del pannello"):lab_tr("Posizione: %.0f, %.0f px logici"),px,py),x,y,13,muted);y+=20;
    }
    y+=TextLines(lab_tr("Trascina i monitor come sulla scrivania. Cambia solo il panorama dello sfondo."),x,y,w,13,2,muted)+8;
    const char* basis[]={lab_tr("Pixel logici"),lab_tr("Dimensioni reali")};at=lab_display_physical();next=Choice(2,x,&y,w,lab_tr("Misurazione"),basis,2,at);if(next!=at){lab_display_basis(next);displayDragging=-1;}
    const char* align[]={lab_tr("Trascina liberamente"),lab_tr("Allinea in alto"),lab_tr("Al centro"),lab_tr("In basso"),lab_tr("Posizioni rilevate")};next=Choice(3,x,&y,w,lab_tr("Allineamento"),align,5,0);if(next>0){if(next==4)lab_display_basis(lab_display_physical());else lab_display_align(next-1);displayDragging=-1;}
    const char* error=lab_display_error();
    TextLines(*error?error:lab_display_dirty()?lab_tr("Modifiche da salvare. Alt disattiva l'aggancio."):lab_display_active()?lab_tr("Disposizione salvata. Trascina per modificare."):lab_tr("Salva per usare le dimensioni reali nel panorama."),x,y,w,13,2,*error?(Color){236,133,140,255}:muted);y+=28;
    displaySaveButton=(Rectangle){x,y,212,36};
    if(Button(displaySaveButton,lab_current()>=0?lab_tr("Salva e applica"):lab_tr("Salva disposizione"),true,!applying&&!*error)){
        if(lab_display_save()){snprintf(notice,sizeof(notice),lab_tr("Disposizione dei monitor salvata."));if(lab_current()>=0)applying=lab_start(lab_current(),0);}
    }
    if(Button((Rectangle){x+224,y,116,36},lab_tr("Annulla"),false,!applying&&lab_display_dirty())){lab_display_revert();displayDragging=-1;}
    displayContentHeight=y-start+48;return displayContentHeight;
}
