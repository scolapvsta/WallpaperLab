// Integration controls live here; activation remains in Settings.
static int integrationTab=0;
static float integrationScroll=0,integrationScrollDraw=0,integrationExtent=0;
static Rectangle integrationTabs[2];
static bool IntegrationsEnabled(void){return lab_option("caelestia_enabled")||lab_sddm_enabled();}
static bool ManagementPage(void){return page==2||page==4;}
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
    bool enabled[]={lab_option("caelestia_enabled"),lab_sddm_enabled()};
    if(!enabled[integrationTab]){integrationTab=enabled[0]?0:1;choiceOpen=-1;integrationScroll=integrationScrollDraw=0;}
    ThemePanel((Rectangle){area.x+148,area.y,area.width-156,area.height},.025f,12,surface);
    int x=area.x+16,y=area.y+12,w=area.width-32,slot=0;
    const char* names[]={"Caelestia","SDDM"};memset(integrationTabs,0,sizeof(integrationTabs));
    for(int i=0;i<2;i++)if(enabled[i]){
        integrationTabs[i]=(Rectangle){x,y+slot++*43,122,36};
        if(Button(integrationTabs[i],names[i],integrationTab==i,!applying)){integrationTab=i;choiceOpen=-1;integrationScroll=integrationScrollDraw=0;}
    }
    if(integrationTab==1&&enabled[1]&&lab_sddm_ready()){SddmLibrary((Rectangle){area.x+148,area.y,area.width-156,area.height});return;}
    x+=140;w-=140;Rectangle body={x,y-3,w+3,area.height-21},saved=inputClip;inputClip=body;
    if(!choiceWheel&&Hot(body,true))integrationScroll=Clamp(integrationScroll-ScrollInput(body)*48,0,fmaxf(0,integrationExtent-body.height));choiceWheel=false;
    integrationScrollDraw=SmoothScroll(integrationScrollDraw,integrationScroll);BeginScissorMode(body.x,body.y,body.width,body.height);y=body.y+6-integrationScrollDraw;
    if(integrationTab==0&&enabled[0])y+=CaelestiaBody(x,y,w);
    else if(enabled[1]){
        y+=TextLines(lab_tr("Da verificare"),x,y,w,16,3,AppTheme.text)+20;
        if(Button((Rectangle){x,y,w,34},lab_tr("Verifica SDDM"),false,!lab_sddm_busy()&&!applying))lab_sddm_action("check","");y+=46;
        if(!lab_sddm_ok()&&!lab_sddm_busy())y+=TextLines(lab_tr("Operazione non riuscita. Dettagli nel registro."),x,y,w,13,3,muted)+12;
    }
    integrationExtent=fmaxf(0,y+integrationScrollDraw-body.y+20);EndScissorMode();inputClip=saved;
    integrationScroll=Clamp(integrationScroll,0,fmaxf(0,integrationExtent-body.height));
    if(integrationExtent>body.height){float height=fmaxf(24,body.height*body.height/integrationExtent);DrawRectangleRounded((Rectangle){body.x+body.width-3,body.y+integrationScrollDraw/(integrationExtent-body.height)*(body.height-height),3,height},1,8,muted);}
}
