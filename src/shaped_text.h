#pragma once
#include <pango/pangocairo.h>
#include <stdint.h>
typedef struct {char* text;char family[96];int size,revision,width,height;Texture2D texture;unsigned used;} ShapedText;
static ShapedText shapedCache[1024];static unsigned shapedClock,shapedRasterizations;
typedef struct {char* text;uint64_t hash;int revision,size,width;} TextMeasure;
static TextMeasure textMeasures[8192];
static bool ComplexText(const char* s){return s&&*s;}
static uint64_t TextHash(const char* text,int size,const char* family){uint64_t h=1469598103934665603ull;for(const unsigned char* p=(const unsigned char*)text;*p;p++)h=(h^*p)*1099511628211ull;for(const unsigned char* p=(const unsigned char*)family;*p;p++)h=(h^*p)*1099511628211ull;return h^(uint64_t)size*7919u;}
static PangoFontDescription* TextDescription(int size,const char* family){PangoFontDescription* font=pango_font_description_new();pango_font_description_set_family(font,family);pango_font_description_set_absolute_size(font,size*.92*PANGO_SCALE);pango_font_description_set_weight(font,lab_option("font_weight")==2?PANGO_WEIGHT_BOLD:lab_option("font_weight")==0?PANGO_WEIGHT_MEDIUM:PANGO_WEIGHT_SEMIBOLD);return font;}
// Measurement never allocates a raylib image, texture or GL upload.
static int MeasureShaped(const char* text,int size){
 const char* family=lab_font_family();uint64_t hash=TextHash(text,size,family);TextMeasure* c=&textMeasures[hash&8191];int revision=lab_font_revision();
 if(c->text&&c->hash==hash&&c->revision==revision&&c->size==size&&!strcmp(c->text,text))return c->width;
 static PangoLayout* layout;static int seen=-1,seenSize=-1;if(!layout){PangoFontMap* map=pango_cairo_font_map_get_default();PangoContext* ctx=pango_font_map_create_context(map);layout=pango_layout_new(ctx);g_object_unref(ctx);}
 if(seen!=revision||seenSize!=size){PangoFontDescription* font=TextDescription(size,family);pango_layout_set_font_description(layout,font);pango_font_description_free(font);seen=revision;seenSize=size;}
 pango_layout_set_text(layout,text,-1);int width;pango_layout_get_pixel_size(layout,&width,NULL);
 free(c->text);*c=(TextMeasure){strdup(text),hash,revision,size,width};return width;
}
static ShapedText* ShapeFamily(const char* text,int size,const char* family){
 shapedClock++;unsigned bucket=(TextHash(text,size,family)&255)*4;int slot=bucket;unsigned oldest=~0u;
 for(int n=bucket;n<bucket+4;n++){ShapedText* c=&shapedCache[n];if(c->text&&c->size==size&&c->revision==lab_font_revision()&&!strcmp(c->family,family)&&!strcmp(c->text,text)){c->used=shapedClock;return c;}if(c->used<oldest){oldest=c->used;slot=n;}}
 ShapedText* c=&shapedCache[slot];if(c->texture.id){rlDrawRenderBatchActive();UnloadTexture(c->texture);}free(c->text);memset(c,0,sizeof(*c));c->text=strdup(text);snprintf(c->family,sizeof(c->family),"%s",family);c->size=size;c->revision=lab_font_revision();c->used=shapedClock;
 cairo_surface_t* surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,1,1);cairo_t* cr=cairo_create(surface);PangoLayout* layout=pango_cairo_create_layout(cr);PangoFontDescription* font=TextDescription(size,family);pango_layout_set_font_description(layout,font);pango_layout_set_text(layout,text,-1);
 PangoRectangle ink,logical;pango_layout_get_pixel_extents(layout,&ink,&logical);int left=ink.x<0?ink.x:0,top=ink.y<0?ink.y:0;
 c->width=Clamp(fmaxf(logical.width,ink.x+ink.width)-left,1,8192);c->height=Clamp(fmaxf(logical.height,ink.y+ink.height)-top,1,256);cairo_destroy(cr);cairo_surface_destroy(surface);
 surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,c->width,c->height);cr=cairo_create(surface);cairo_set_source_rgba(cr,1,1,1,1);cairo_move_to(cr,-left,-top);pango_cairo_update_layout(cr,layout);pango_cairo_show_layout(cr,layout);cairo_surface_flush(surface);
 unsigned char* data=cairo_image_surface_get_data(surface);int stride=cairo_image_surface_get_stride(surface);Image image=GenImageColor(c->width,c->height,BLANK);for(int y=0;y<c->height;y++)for(int x=0;x<c->width;x++){unsigned char* px=(unsigned char*)image.data+(y*c->width+x)*4;px[0]=px[1]=px[2]=255;px[3]=data[y*stride+x*4+3];}
 rlDrawRenderBatchActive();c->texture=LoadTextureFromImage(image);shapedRasterizations++;UnloadImage(image);g_object_unref(layout);pango_font_description_free(font);cairo_destroy(cr);cairo_surface_destroy(surface);return c;
}
static ShapedText* Shape(const char* text,int size){return ShapeFamily(text,size,lab_font_family());}
