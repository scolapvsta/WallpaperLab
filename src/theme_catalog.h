#pragma once
// Stable palette IDs. Each UI determines its own light/dark mode.
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
 {"CRT Ocean",0,13,0x0b1b2a,0x173247,0xe4faff,0x95bfd0,0x6de5ee,0,3}
};
#define LAB_THEME_COUNT ((int)(sizeof(labThemes)/sizeof(labThemes[0])))
#define LAB_STRAWBERRY_PALETTE 0
#define LAB_DESIGN_COUNT 4
static const char* labDesignNames[]={"Default","Strawberry core","2077","Pixel Theme"};
static int LabDefaultPalette(int design){static const int ids[]={14,0,3,17};return ids[design>=0&&design<4?design:0];}
static const char* LabPaletteKey(int d){static const char* keys[]={"default_palette","strawberry_palette","cyber_palette","pixel_palette"};return keys[d>=0&&d<4?d:0];}
static const char* LabBackgroundKey(int d){static const char* keys[]={"default_background","strawberry_background","cyber_background","pixel_background"};return keys[d>=0&&d<4?d:0];}
static const char* LabFontKey(int d){static const char* keys[]={"theme_font_default","theme_font_strawberry","theme_font_cyber","theme_font_pixel"};return keys[d>=0&&d<4?d:0];}
static const char* LabDefaultFont(int d){return d==2?"Oxanium":d==3?"Silkscreen":"Sans";}
static int LabSuggestedFonts(int d,const char** names){
 names[0]=LabDefaultFont(d);names[1]=d==2?"Rajdhani":d==3?"Oxanium":"Adwaita Sans";names[2]=d==2?"Orbitron":d==3?"DejaVu Sans Mono":"Noto Sans";return 3;
}
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
