// Per-project placement: preview while dragging, persist on release.
static int placementItem=-1,placementAxis=-1;
static float placementX=0,placementY=0,placementZoom=1;
static Rectangle placementTracks[2],placementReset,zoomTrack;
static void PlacementSync(int index){
    if(placementItem!=index||placementAxis<0){placementItem=index;placementX=lab_position_x(index);placementY=lab_position_y(index);placementZoom=lab_zoom(index);}
}
static bool PositionSlider(Rectangle r,const char* label,float* value,int axis){
    UiText(label,r.x,r.y,15,AppTheme.text);UiText(TextFormat("%+.0f%%",*value*100),r.x+r.width-48,r.y,14,muted);
    Rectangle track={r.x+4,r.y+28,r.width-8,5};placementTracks[axis]=track;
    bool hot=Hot((Rectangle){r.x,r.y+20,r.width,30},true);
    if(hot&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))placementAxis=axis;
    if(placementAxis==axis){
        *value=Clamp((GetMouseX()-track.x)/track.width*2-1,-1,1);
        if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){placementAxis=-1;return true;}
    }
    ThemePanel(track,1,8,controlHover);
    float center=track.x+track.width/2,thumb=track.x+(*value+1)*track.width/2;
    DrawRectangleRec((Rectangle){fminf(center,thumb),track.y,fabsf(center-thumb),5},AppTheme.border);
    DrawLine(center,track.y-4,center,track.y+9,muted);DrawCircle(thumb,track.y+2,7,AppTheme.text);
    if(hot||placementAxis==axis)SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
    return false;
}
static int PlacementControls(int index,int x,int y,int width){
    int start=y;
    UiText(lab_tr("Zoom"),x,y,15,AppTheme.text);UiText(TextFormat("%.0f%%",placementZoom*100),x+width-48,y,14,muted);
    zoomTrack=(Rectangle){x+4,y+27,width-8,5};
    if(Hot((Rectangle){x,y+16,width,26},!applying)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))placementAxis=2;
    if(placementAxis==2){placementZoom=1+Clamp((GetMouseX()-zoomTrack.x)/zoomTrack.width,0,1)*2;if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){placementAxis=-1;lab_zoom_set(index,placementZoom);}}
    ThemePanel(zoomTrack,1,8,controlHover);DrawRectangleRec((Rectangle){zoomTrack.x,zoomTrack.y,(placementZoom-1)/2*zoomTrack.width,5},AppTheme.border);DrawCircle(zoomTrack.x+(placementZoom-1)/2*zoomTrack.width,zoomTrack.y+2,7,AppTheme.text);
    y+=46;UiText(lab_tr("Posizione dello sfondo"),x,y,16,AppTheme.text);y+=26;
    bool changed=PositionSlider((Rectangle){x,y,width,48},lab_tr("Sinistra / destra"),&placementX,0);y+=48;
    changed=PositionSlider((Rectangle){x,y,width,48},lab_tr("Alto / basso"),&placementY,1)||changed;y+=48;
    placementReset=(Rectangle){x,y,width,34};
    if(Button(placementReset,lab_tr("Centra lo sfondo"),false,true)){placementAxis=-1;placementX=placementY=0;changed=true;}y+=46;
    static double sent=0;
    if(placementAxis>=0&&GetTime()-sent>0.033){lab_placement_live(index,placementX,placementY,placementZoom);sent=GetTime();}
    if(changed){
        if(lab_set_position(index,placementX,placementY))snprintf(notice,sizeof(notice),lab_tr("Posizione salvata e aggiornata sul desktop."));
    }
    TextLines(lab_tr("Aumenta lo zoom per spostare il ritaglio."),x,y,width,12,2,muted);y+=38;
    return y-start;
}
