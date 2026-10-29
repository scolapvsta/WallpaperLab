// Local preferences are independent of the global playback switches.
static int adjustmentItem=-1;static float adjustmentDraft[3]={0,1,1};
static int WallpaperControls(int i,int x,int y,int w){
 int start=y;const char* imageKeys[]={"brightness","contrast","saturation"};const char* imageNames[]={"Luminosità","Contrasto","Saturazione"};
 UiText(lab_tr("Immagine"),x,y,17,AppTheme.text);y+=29;
 for(int n=0;n<3;n++){
  if(adjustmentItem!=i||uiSliderDrag!=n+1)adjustmentDraft[n]=lab_adjust(i,imageKeys[n]);
  bool commit=ValueSlider(n+1,x,&y,w,lab_tr(imageNames[n]),&adjustmentDraft[n],n==0?-1:0,n==0?1:2,n==0);
  static double lastSent[3];if(commit||(uiSliderDrag==n+1&&GetTime()-lastSent[n]>.033)){lab_adjust_set(i,imageKeys[n],adjustmentDraft[n],commit);lastSent[n]=GetTime();}
 }
 adjustmentItem=i;
 if(Button((Rectangle){x,y,w,30},lab_tr("Ripristina immagine"),false,true)){uiSliderDrag=-1;for(int n=0;n<3;n++)lab_adjust_set(i,imageKeys[n],n==0?0:1,1);}y+=44;
 UiText(lab_tr("Riproduzione per questo sfondo"),x,y,17,AppTheme.text);y+=30;
 const char* keys[]={"audio","reactive","mouse","parallax","particles"},*global[]={"muted","disable_audio_processing","disable_mouse","disable_parallax","disable_particles"};
 const char* labels[]={"Audio dello sfondo","Reazione alla musica","Interazione con il mouse","Parallasse","Particelle"};
 for(int n=0;n<5;n++){int at=lab_playback(i,keys[n]),value=BoolChoice(64+n,x,&y,w,lab_tr(labels[n]),at);if(at!=value&&lab_playback_set(i,keys[n],value)&&!uiSmoke&&lab_current()==i)applying=lab_select(i);if(lab_option(global[n])){y+=TextLines(lab_tr("Disattivato dalle impostazioni generali"),x,y-5,w,12,2,muted)+7;}}
 y+=TextLines(lab_tr("Le opzioni generali hanno precedenza. Queste preferenze restano salvate per questo sfondo."),x,y,w,12,3,muted)+16;return y-start;
}
