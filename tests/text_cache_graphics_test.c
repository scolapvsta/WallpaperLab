// Opt-in graphical regression: LRU texture eviction must preserve queued text draws.
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
static const char* lab_font_family(void){return "Sans";}
static int lab_font_revision(void){return 1;}
static int lab_font_selected(void){return 0;}
static const char* lab_font_name(int i){(void)i;return "Sans";}
static int lab_option(const char* key){(void)key;return 1;}
#include "shaped_text.h"
int main(int argc,char** argv){
 SetTraceLogLevel(LOG_ERROR);InitWindow(1280,720,"Wallpaper Lab text cache regression");SetTargetFPS(60);
 unsigned before=shapedRasterizations;
 for(int n=0;n<3000;n++){char label[80];snprintf(label,sizeof(label),"Workshop title measurement %d",n);MeasureShaped(label,14);}
 if(shapedRasterizations!=before)return 2;
 Image reference={0},batched={0};RenderTexture2D canvas=LoadRenderTexture(1280,720);
 for(int pass=0;pass<2;pass++){
  BeginDrawing();BeginTextureMode(canvas);ClearBackground((Color){22,26,32,255});
  for(int n=0;n<1100;n++){char text[64];snprintf(text,sizeof(text),"字%d",n);ShapedText* entry=Shape(text,14);DrawTexture(entry->texture,8+n%44*29,8+n/44*27,WHITE);if(!pass)rlDrawRenderBatchActive();}
  EndTextureMode();Image shot=LoadImageFromTexture(canvas.texture);EndDrawing();if(!pass)reference=shot;else batched=shot;
 }
 Color* a=LoadImageColors(reference),*b=LoadImageColors(batched);int different=0,ink=0;
 for(int n=0;n<1280*720;n++){if(memcmp(a+n,b+n,sizeof(Color)))different++;if(a[n].r>100)ink++;}
 if(argc>1){ExportImage(reference,TextFormat("%s/reference.png",argv[1]));ExportImage(batched,TextFormat("%s/batched.png",argv[1]));}
 printf("{\"different_pixels\":%d,\"text_pixels\":%d,\"cache_capacity\":1024,\"labels_per_frame\":1100}\n",different,ink);
 UnloadImageColors(a);UnloadImageColors(b);UnloadImage(reference);UnloadImage(batched);UnloadRenderTexture(canvas);CloseWindow();return different||ink<1000;
}
