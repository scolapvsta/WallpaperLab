// Standalone native artwork captures; no catalog or user configuration is loaded.
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include "theme_catalog.h"
static struct {Color bg,text,border;} AppTheme;
static Color surface;
static int preset,design,motion;
static int ActiveThemeDesign(void){return design;}
static int ActiveBackgroundPreset(void){return preset;}
static int lab_option(const char* key){return motion;}
static Color ThemeHex(unsigned int c){return (Color){c>>16,(c>>8)&255,c&255,255};}
#include "strawberry_ui.h"
#include "pixel_ui.h"
static void save(RenderTexture2D target,const char* directory,const char* name){
 Image im=LoadImageFromTexture(target.texture);ImageFlipVertical(&im);
 ExportImage(im,TextFormat("%s/%s.png",directory,name));UnloadImage(im);
}
int main(int argc,char** argv){
 if(argc!=2)return 2;
 SetConfigFlags(FLAG_WINDOW_HIDDEN);InitWindow(1280,720,"Theme artwork verification");
 RenderTexture2D target=LoadRenderTexture(1280,720);
 for(design=1;design<=3;design+=2){
  LabTheme theme=labThemes[LabDefaultPalette(design)];AppTheme.bg=ThemeHex(theme.bg);surface=ThemeHex(theme.surface);AppTheme.text=ThemeHex(theme.text);AppTheme.border=ThemeHex(theme.accent);
  for(preset=0;preset<4;preset++){
   BeginTextureMode(target);ClearBackground(BLANK);if(design==1)BerryBackdrop(1280,720);else PixelBackdrop(1280,720);EndTextureMode();save(target,argv[1],TextFormat("art-%d-%d",design,preset));
  }
 }
 BeginTextureMode(target);ClearBackground(AppTheme.bg);
 float stages[]={0,3.8f,4.5f,5.5f,6.5f,7.5f,8.6f,11};
 for(int i=0;i<8;i++)ArcadeChaseFrame((Rectangle){20,15+i*86,1240,70},stages[i]);
 EndTextureMode();save(target,argv[1],"arcade-chase-stages");UnloadRenderTexture(target);CloseWindow();return 0;
}
