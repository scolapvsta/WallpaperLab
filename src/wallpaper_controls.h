// Local preferences are independent of the global playback switches.
static int adjustmentItem=-1;static float adjustmentDraft[8];
static int ImageControls(int i,int x,int y,int w){
 int start=y;const char* keys[]={"brightness","contrast","saturation","temperature","hue","gamma","vignette","rotation"};const char* names[]={"Luminosità","Contrasto","Saturazione","Temperatura","Tonalità","Gamma","Vignettatura","Rotazione"};float lo[]={-1,0,0,-1,-180,.25,0,-180},hi[]={1,2,2,1,180,3,1,180};
 for(int n=0;n<8;n++){int id=n<3?n+1:n+117;if(adjustmentItem!=i||uiSliderDrag!=id)adjustmentDraft[n]=lab_adjust(i,keys[n]);bool commit;
 if(n<3)commit=ValueSlider(id,x,&y,w,lab_tr(names[n]),&adjustmentDraft[n],lo[n],hi[n],n==0);else{float display=adjustmentDraft[n]*(n==3||n==6?100:1);commit=NumberSlider(id,x,&y,w,lab_tr(names[n]),&display,lo[n]*(n==3||n==6?100:1),hi[n]*(n==3||n==6?100:1),n==4||n==7?"°":n==3||n==6?"%":"×");adjustmentDraft[n]=display/(n==3||n==6?100:1);}
 static double lastSent[8];if(commit||(uiSliderDrag==id&&GetTime()-lastSent[n]>.033)){lab_adjust_set(i,keys[n],adjustmentDraft[n],commit);lastSent[n]=GetTime();}
 }
 adjustmentItem=i;const char* flipKeys[]={"flip_x","flip_y"},*flipNames[]={"Specchia orizzontalmente","Specchia verticalmente"};for(int n=0;n<2;n++){int at=lab_adjust(i,flipKeys[n])>.5f,v=BoolChoice(81+n,x,&y,w,lab_tr(flipNames[n]),at);if(v!=at)lab_adjust_set(i,flipKeys[n],v,1);}
 if(Button((Rectangle){x,y,w,30},lab_tr("Ripristina immagine"),false,true)){uiSliderDrag=-1;for(int n=0;n<8;n++)lab_adjust_set(i,keys[n],n==1||n==2||n==5?1:0,1);for(int n=0;n<2;n++)lab_adjust_set(i,flipKeys[n],0,1);}y+=44;
 return y-start;
}
static int WallpaperPlayback(int i,int x,int y,int w){int start=y;
 const char* keys[]={"audio","reactive","mouse","parallax","particles"},*global[]={"muted","disable_audio_processing","disable_mouse","disable_parallax","disable_particles"};
 const char* labels[]={"Audio dello sfondo","Reazione alla musica","Interazione con il mouse","Parallasse","Particelle"};
 for(int n=0;n<5;n++){int at=lab_playback(i,keys[n]),value=BoolChoice(64+n,x,&y,w,lab_tr(labels[n]),at);if(at!=value&&lab_playback_set(i,keys[n],value)&&!uiSmoke&&lab_current()==i)applying=lab_start(i,0);if(lab_option(global[n])){y+=TextLines(lab_tr("Disattivato dalle impostazioni generali"),x,y-5,w,12,2,muted)+7;}}
 y+=TextLines(lab_tr("Le opzioni generali hanno precedenza. Queste preferenze restano salvate per questo sfondo."),x,y,w,12,3,muted)+16;return y-start;
}
