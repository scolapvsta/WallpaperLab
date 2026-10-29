#pragma once
// Stable legacy palette IDs; UI design and light/dark mode are independent.
typedef struct LabTheme {const char* name;int light,effect;unsigned int bg,surface,text,muted,accent;float radius;int collection;} LabTheme;
// Each active palette belongs to exactly one UI. Historical palettes live in Git/backup.
static const LabTheme labThemes[]={
 {"Strawberry Milk",1,0,0xfbe4e9,0xfff7ef,0x683a48,0x8e5868,0xb64264,1.7f,1},
 {"Raspberry Ribbon",1,0,0xf5dce7,0xfff2f7,0x663347,0x8b536a,0xac315b,1.7f,1},
 {"Vanilla Shortcake",1,0,0xf9eddf,0xfffaf0,0x70483e,0x926358,0xb64759,1.7f,1},
 {"Violet Afterglow",0,12,0x080b1b,0x14172c,0xe9edff,0xa5afd0,0xba90ff,.15f,2},
 {"Pink Voltage",0,12,0x160a1b,0x281329,0xffe9fa,0xd1a5c9,0xff69c6,.15f,2},
 {"Electric Blue",0,12,0x06131f,0x102332,0xe5faff,0x9dbfce,0x49dfff,.15f,2},
 {"Night Drive",0,12,0x0a1021,0x172039,0xedf0ff,0xadb8d9,0xf989cf,.15f,2},
 {"Hazard Yellow",0,12,0x14160e,0x23271c,0xf5f8df,0xb6bca0,0xf4ef54,.15f,2}
};
#define LAB_THEME_COUNT ((int)(sizeof(labThemes)/sizeof(labThemes[0])))
#define LAB_STRAWBERRY_PALETTE 0
static int LabDefaultPalette(int design){return design==2?3:0;}
static int LabPaletteForDesign(int id,int design){return id>=0&&id<LAB_THEME_COUNT&&labThemes[id].collection==design?id:LabDefaultPalette(design);}
static unsigned int LabMixHex(unsigned int a,unsigned int b,float t){
 unsigned int out=0;for(int shift=0;shift<=16;shift+=8)out|=(unsigned int)(((a>>shift)&255)*(1-t)+((b>>shift)&255)*t)<<shift;return out;
}
static LabTheme LabResolvedPalette(int id,int light){
 if(id<0||id>=LAB_THEME_COUNT)id=0;LabTheme t=labThemes[id];
 if(t.light!=light){
  unsigned int accent=t.accent;
  t.bg=LabMixHex(light?0xfff8f4:0x1b1219,accent,light?.07f:.09f);
  t.surface=LabMixHex(light?0xfffdfa:0x2c2028,accent,light?.025f:.06f);
  t.text=LabMixHex(light?0x30212b:0xffeff5,accent,.12f);
  t.muted=LabMixHex(t.text,t.surface,.28f);
  t.accent=LabMixHex(light?0x60243e:0xffc5d8,accent,light?.42f:.25f);
 }
 t.light=light;return t;
}
