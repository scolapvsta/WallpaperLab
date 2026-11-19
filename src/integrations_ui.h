// Integration controls live here; activation remains in Settings.
static int integrationTab=0;
static float integrationScroll=0,integrationScrollDraw=0,integrationExtent=0;
static Rectangle integrationTabs[2];
static bool IntegrationsEnabled(void){return lab_option("caelestia_enabled");}
static bool ManagementPage(void){return page==2||page==4||page==5;}
static int CaelestiaBody(int x,int y,int w){
    int start=y;
    UiText("Caelestia",x,y,20,AppTheme.text);y+=34;
        if(lab_monitor_count()){const char* outputs[32];int count=lab_monitor_count()<32?lab_monitor_count():32;for(int i=0;i<count;i++)outputs[i]=lab_monitor(i);int screen=lab_caelestia_screen(),chosen=Choice(80,x,&y,w,lab_tr("Monitor per i colori"),outputs,count,screen);if(chosen!=screen)lab_caelestia_set_screen(chosen);}
        y+=TextLines(lab_tr("Caelestia usa una palette globale. Scegli il monitor da cui ricavare i colori."),x,y,w,13,4,muted)+12;
        if(lab_option("wallpaper_backend")==1){
            y+=TextLines(lab_tr("Le immagini awww usano lo schema e la selezione automatica di Caelestia CLI."),x,y,w,13,4,muted)+18;
        }else{
        const char* palettes[]={lab_tr("Vivace"),lab_tr("Toni morbidi"),lab_tr("Espressiva"),lab_tr("Fedele al wallpaper")};int palette=lab_option("caelestia_palette"),next=Choice(78,x,&y,w,lab_tr("Palette dinamica"),palettes,4,palette);if(next!=palette)lab_set_option("caelestia_palette",next);
        const char* modes[]={lab_tr("Automatica"),lab_tr("Scura"),lab_tr("Chiara")};int mode=lab_option("caelestia_mode");next=Choice(79,x,&y,w,lab_tr("Luminosità della palette"),modes,3,mode);if(next!=mode)lab_set_option("caelestia_mode",next);
        y+=TextLines(lab_tr("L’attivazione prepara solo i componenti colore opzionali. La shell Caelestia resta separata."),x,y,w,12,4,muted)+14;
        y+=TextLines(lab_tr("Aggiorna barre e widget di Caelestia dai colori dell’anteprima. Non cambia le altre applicazioni."),x,y,w,13,4,muted)+18;
        }
        y+=TextLines(lab_caelestia_available()?lab_caelestia_status():lab_tr("Caelestia non rilevato"),x,y,w,12,4,muted)+14;
    return y-start;
}
static void IntegrationsBody(Rectangle area){
    ThemePanel(area,.025f,12,surface);Rectangle saved=inputClip;inputClip=area;
    if(!choiceWheel&&Hot(area,true))integrationScroll=fmaxf(0,integrationScroll-ScrollInput(area)*48);choiceWheel=false;
    integrationScrollDraw=SmoothScroll(integrationScrollDraw,integrationScroll);BeginScissorMode(area.x,area.y,area.width,area.height);
    int used=CaelestiaBody(area.x+24,area.y+20-integrationScrollDraw,area.width-48);EndScissorMode();inputClip=saved;integrationScroll=Clamp(integrationScroll,0,fmaxf(0,used+40-area.height));
}
