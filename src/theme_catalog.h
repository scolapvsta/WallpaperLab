#pragma once
// Stable legacy palette IDs; UI design and light/dark mode are independent.
typedef struct LabTheme {const char* name;int light,effect;unsigned int bg,surface,text,muted,accent;float radius;int collection;} LabTheme;
static const LabTheme labThemes[]={
 {"Charcoal",0,0,0x101216,0x1a1d23,0xe7e9f1,0xa6afbf,0x99d8c3,1,0},
 {"Paper",1,0,0xf3f4f6,0xffffff,0x303442,0x616879,0x386d62,1.2f,0},
 {"Cyber Neon",0,1,0x070d18,0x101d2c,0xe5faff,0x96b9ca,0x41e5ff,.35f,1},
 {"Aurora Glass",0,2,0x111d30,0x233448,0xeaf4ff,0xa3bdce,0x9ccfd8,1.5f,3},
 {"Sakura Neon",0,3,0x1a0b22,0x2f1537,0xffe9f8,0xcaa0c0,0xff6bce,1.1f,1},
 {"Catppuccin Latte",1,4,0xeff1f5,0xe6e9ef,0x4c4f69,0x6c6f85,0x8839ef,1.8f,2},
 {"Midnight Violet",0,5,0x160c28,0x25153b,0xeee2ff,0xb19cc9,0xbc8cff,.75f,1},
 {"Matrix LED",0,6,0x07100b,0x101e15,0xd5f3dc,0x8baf95,0x71ee96,.2f,5},
 {"Prism RGB",0,7,0x10101c,0x1d1d30,0xeeeeff,0xaaaac8,0x9badff,.9f,5},
 {"Nord",0,8,0x2e3440,0x3b4252,0xeceff4,0xc0c8d6,0x88c0d0,.5f,0},
 {"Rosé Pine",0,9,0x191724,0x1f1d2e,0xe0def4,0xaaa4c4,0xebbcba,1.1f,2},
 {"Rosé Dawn",1,9,0xfaf4ed,0xfffaf3,0x575279,0x797593,0xb4637a,1.5f,2},
 {"Sakura Paper",1,3,0xfff1f7,0xfff9fc,0x57364b,0x836477,0xb83d81,1.8f,2},
 {"Frost",1,8,0xecf5fb,0xf8fcff,0x294350,0x5e7888,0x217b9d,.6f,0},
 {"Catppuccin Mocha",0,4,0x1e1e2e,0x313244,0xcdd6f4,0xa6adc8,0xcba6f7,1.8f,2},
 {"Comfy Mint",0,10,0x172124,0x233236,0xe4f3ec,0xa3bcb4,0x83d8b2,2,2},
 {"Lime Circuit",0,1,0x0c1110,0x18221d,0xe6faec,0x9dbbab,0xb6ef45,.35f,1},
 {"Arcade Violet",0,1,0x100c20,0x211733,0xf1eaff,0xb9a5d8,0xbb8cff,.35f,1},
 {"Ember Neon",0,1,0x190e12,0x2c1a20,0xfff0e6,0xc5a59e,0xff9b62,.35f,1},
 {"Lavender Mist",1,4,0xf0edf8,0xf9f6ff,0x48425c,0x746b88,0x7964aa,1.8f,2},
 {"Peach Cloud",1,4,0xffefe7,0xfff8f2,0x59433d,0x8c7168,0xb86a55,1.8f,2},
 {"Mint Sorbet",1,4,0xeaf5ef,0xf6fcf8,0x354e46,0x657f75,0x427c68,1.8f,2},
 {"Moonstone Glass",0,2,0x161c29,0x273146,0xe7edf9,0xb1bdd6,0x9bb7e8,1.5f,3},
 {"Sea Glass",1,2,0xe8f2f3,0xf5fbfb,0x304c54,0x647c84,0x357f8c,1.5f,3},
 {"Pixel Orchid",0,11,0x19152b,0x2a2340,0xf3eaff,0xb7a7ce,0xe1a8f4,0,4},
 {"Pixel Ocean",0,11,0x101f2a,0x1d3341,0xe0f7fb,0x99c2cc,0x69d6e0,0,4},
 {"Pocket Sage",1,11,0xe3e9d5,0xf0f4e5,0x374535,0x6c7961,0x627e45,0,4},
 {"Plum Noir",0,0,0x24171e,0x351e28,0xf6e8ec,0xc4a5b3,0xe6a0b5,1,0},
 {"Persimmon LED",0,6,0x180e0b,0x291d16,0xffefe5,0xc0a58f,0xff8450,.2f,5},
 {"Strawberry Milk",1,0,0xfbe4e9,0xfff7ef,0x683a48,0x8e5868,0xb64264,1.7f,2},
 {"Raspberry Ribbon",1,0,0xf5dce7,0xfff2f7,0x663347,0x8b536a,0xac315b,1.7f,2},
 {"Vanilla Shortcake",1,0,0xf9eddf,0xfffaf0,0x70483e,0x926358,0xb64759,1.7f,2}
};
#define LAB_THEME_COUNT ((int)(sizeof(labThemes)/sizeof(labThemes[0])))
#define LAB_COLLECTION_COUNT 6
static const char* labCollections[]={"Classici","Neon","Pastello","Vetro","Pixel","LED e RGB"};

#define LAB_LEGACY_PALETTE_COUNT 29
#define LAB_STRAWBERRY_PALETTE 29
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
