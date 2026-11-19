// SDDM mutations run in a worker; only verified configurations expose this page.
static int SddmBody(int x,int y,int w){
    int start=y;bool busy=lab_sddm_busy();choiceEnabled=!busy;
    UiText(lab_tr("Sfondo selezionato"),x,y,20,AppTheme.text);y+=34;
    static int mediaPage=0;int total=lab_sddm_count(0),pages=(total+99)/100;if(pages<1)pages=1;
    mediaPage=Clamp(mediaPage,0,pages-1);int first=mediaPage*100,n=fminf(100,total-first);const char* labels[128];
    for(int i=0;i<n;i++)labels[i]=lab_sddm_label(0,first+i);
    int current=lab_sddm_selected(0),selected=Choice(89,x,&y,w,lab_tr("Sfondo selezionato"),labels,n,current-first);
    if(selected>=0&&selected<n&&selected+first!=current){if(lab_sddm_supported(selected+first))lab_sddm_select(0,selected+first);else snprintf(notice,sizeof(notice),"%s",lab_tr("SDDM supporta immagini, GIF e video. Scene e progetti web richiedono un renderer dedicato."));}
    if(pages>1){if(Button((Rectangle){x,y,100,30},lab_tr("Indietro"),false,!busy&&mediaPage>0))mediaPage--;UiText(TextFormat("%d / %d",mediaPage+1,pages),x+115,y+8,13,muted);if(Button((Rectangle){x+w-100,y,100,30},lab_tr("Avanti"),false,!busy&&mediaPage+1<pages))mediaPage++;y+=40;}
    y+=TextLines(lab_tr("SDDM supporta immagini, GIF e video. Scene e progetti web richiedono un renderer dedicato."),x,y,w,13,4,muted)+18;
    current=lab_sddm_option("crop");selected=BoolChoice(92,x,&y,w,lab_tr("Riempi lo schermo"),current);if(selected!=current)lab_sddm_action("crop",selected?"true":"false");
    UiText(lab_tr("UI di login"),x,y,20,AppTheme.text);y+=34;
    n=lab_sddm_count(1);for(int i=0;i<n&&i<128;i++)labels[i]=lab_sddm_label(1,i);
    current=lab_sddm_selected(1);selected=Choice(90,x,&y,w,lab_tr("UI di login"),labels,n,current);if(selected!=current)lab_sddm_select(1,selected);
    const char* positions[]={lab_tr("Predefinito"),lab_tr("Sinistra"),lab_tr("Al centro"),lab_tr("Destra")};const char* values[]={"preset","left","center","right"};
    current=lab_sddm_option("position");selected=Choice(91,x,&y,w,lab_tr("Posizione"),positions,4,current);if(selected!=current)lab_sddm_action("position",values[selected]);
    current=lab_sddm_option("blur");selected=BoolChoice(93,x,&y,w,lab_tr("Sfocatura del login"),current);if(selected!=current)lab_sddm_action("blur",selected?"true":"false");
    y+=14;if(Button((Rectangle){x,y,w,36},lab_tr("Anteprima SDDM · 60 secondi"),false,!busy))lab_sddm_action("preview","");y+=48;
    y+=TextLines(lab_tr("Configurazione di prova. L’installazione nel login di sistema sarà un passaggio separato."),x,y,w,13,4,muted)+14;
    y+=TextLines(lab_sddm_bundle(),x,y,w,12,3,muted)+10;
    if(busy)y+=TextLines(lab_tr("Attendi la fine del caricamento"),x,y,w,13,3,muted)+10;
    else if(!lab_sddm_ok())y+=TextLines(lab_tr("Operazione non riuscita. Dettagli nel registro."),x,y,w,13,3,AppTheme.text)+10;
    choiceEnabled=true;return y-start;
}
