#pragma once
// Stable palette IDs. Default supports both light and dark palettes.
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
 {"Hazard Yellow",0,12,0x14160e,0x23271c,0xf5f8df,0xb6bca0,0xf4ef54,.15f,2},
 {"Crimson Protocol",0,12,0x16090f,0x28131c,0xffedf0,0xd2a6b1,0xff5267,.15f,2},
 {"Toxic Lime",0,12,0x09150f,0x14251e,0xeaffef,0x9cc8ad,0xb6ff4b,.15f,2},
 {"Amber Circuit",0,12,0x19100a,0x2c2016,0xfff1de,0xd5b28c,0xffb347,.15f,2},
 {"Ice Chrome",0,12,0x08161c,0x152a34,0xebfbff,0xa1c3d0,0xa3f4ff,.15f,2},
 {"Ultraviolet",0,12,0x110b22,0x241737,0xf5eaff,0xbfa1d5,0xe18bff,.15f,2},
 {"Copper Sunset",0,12,0x1c0d17,0x321b2a,0xffeff0,0xd2a5b8,0xff9676,.15f,2},
 {"Graphite",0,0,0x14171d,0x222731,0xe9edf5,0xa9b4c5,0x9ccbbd,1,0},
 {"Blue Slate",0,0,0x121a26,0x202e40,0xeaf0fb,0xa6b7d0,0x8ab4ef,1,0},
 {"Forest Ink",0,0,0x111d1a,0x20312b,0xe6f3ec,0xa7c0b4,0x83cda3,1,0},
 {"Arcade Berry",0,13,0x181226,0x2b2240,0xffedfb,0xbba8d1,0xff85c8,0,3},
 {"Dungeon Moss",0,13,0x141c1a,0x25382b,0xecffdd,0xa5bd99,0xa8ed72,0,3},
 {"Coin Rush",0,13,0x20172b,0x352640,0xfff4dd,0xc6b09d,0xffd66f,0,3},
 {"CRT Ocean",0,13,0x0b1b2a,0x173247,0xe4faff,0x95bfd0,0x6de5ee,0,3},
 {"Midnight Blue",0,0,0x0c1426,0x192742,0xe8efff,0xa4b5d1,0x739dff,1,0},
 {"Aubergine",0,0,0x1a1225,0x30223f,0xf2eaff,0xbfa9d1,0xc8a0ff,1,0},
 {"Rosewood",0,0,0x23131d,0x3c2333,0xffedf5,0xd2acbf,0xf293ba,1,0},
 {"Ember",0,0,0x231710,0x3b281d,0xffefdf,0xd0b49e,0xffae76,1,0},
 {"Golden Noir",0,0,0x201d12,0x353020,0xfff6db,0xc7bd9e,0xe7ca73,1,0},
 {"Ocean Depths",0,0,0x0d2025,0x19343c,0xe3f9ff,0x9ebfc9,0x69d4e3,1,0},
 {"Carbon",0,0,0x171719,0x29292e,0xf1f1f5,0xb5b5c1,0xc5c5d5,1,0},
 {"Porcelain",1,0,0xf5f7fa,0xffffff,0x252d3b,0x58667a,0x496b9c,1,0},
 {"Paper",1,0,0xf6f0e5,0xfffcf4,0x3a3329,0x716554,0x886143,1,0},
 {"Pearl Grey",1,0,0xe8ebef,0xf8fafc,0x29323f,0x5d6878,0x637187,1,0},
 {"Sky Mist",1,0,0xe4effc,0xf6faff,0x253b59,0x55708e,0x376db1,1,0},
 {"Mint Cream",1,0,0xe3f3ec,0xf6fffa,0x244437,0x537666,0x2b8263,1,0},
 {"Sage Linen",1,0,0xeaf0e1,0xfbfff4,0x35462d,0x68785e,0x5c7b42,1,0},
 {"Lavender Haze",1,0,0xeee7f8,0xfdf8ff,0x45345a,0x78658d,0x8557ad,1,0},
 {"Rose Quartz",1,0,0xf8e6ed,0xfff8fb,0x573346,0x8c6275,0xad4c76,1,0},
 {"Peach Sorbet",1,0,0xfbe9dc,0xfffaf4,0x583e2d,0x8a6b54,0xa95e32,1,0},
 {"Buttercream",1,0,0xf8f2d5,0xfffdef,0x504723,0x807448,0x8b741e,1,0}
};
#define LAB_THEME_COUNT ((int)(sizeof(labThemes)/sizeof(labThemes[0])))
#define LAB_STRAWBERRY_PALETTE 0
#define LAB_DESIGN_COUNT 4
static const char* labDesignNames[]={"Default","Strawberry core","2077","Arcade"};
static int LabDefaultPalette(int design){static const int ids[]={14,0,3,17};return ids[design>=0&&design<4?design:0];}
static const char* LabPaletteKey(int d){static const char* keys[]={"default_palette","strawberry_palette","cyber_palette","pixel_palette"};return keys[d>=0&&d<4?d:0];}
static const char* LabBackgroundKey(int d){static const char* keys[]={"default_background","strawberry_background","cyber_background","pixel_background"};return keys[d>=0&&d<4?d:0];}
static const char* LabFontKey(int d){static const char* keys[]={"theme_font_default","theme_font_strawberry","theme_font_cyber","theme_font_pixel"};return keys[d>=0&&d<4?d:0];}
static const char* LabDefaultFont(int d){return d==2?"Oxanium":d==3?"Silkscreen":"Sans";}
static int LabSuggestedFonts(int d,const char** names){
 names[0]=LabDefaultFont(d);names[1]=d==2?"Rajdhani":d==3?"Oxanium":"Adwaita Sans";names[2]=d==2?"Orbitron":d==3?"DejaVu Sans Mono":"Noto Sans";return 3;
}
static int LabPaletteForDesign(int id,int design){return id>=0&&id<LAB_THEME_COUNT&&labThemes[id].collection==design?id:LabDefaultPalette(design);}
// Preserve an old themed switcher's accent as closely as the plain catalog allows.
static int LabSwitcherPalette(int id){
 if(id<0||id>=LAB_THEME_COUNT)return LabDefaultPalette(0);
 if(labThemes[id].collection==0)return id;
 int best=LabDefaultPalette(0),distance=2147483647;
 for(int i=0;i<LAB_THEME_COUNT;i++)if(labThemes[i].collection==0){
  int d=labThemes[i].light==labThemes[id].light?0:1000000;
  for(int shift=0;shift<=16;shift+=8){int delta=(int)((labThemes[i].accent>>shift)&255)-(int)((labThemes[id].accent>>shift)&255);d+=delta*delta;}
  if(d<distance){distance=d;best=i;}
 }
 return best;
}
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
