#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_GIF
#include "../vendor/raylib/src/external/stb_image.h"
#include "gif.h"
#include <stdio.h>
#include <string.h>
// Bound decoded memory before stb allocates every GIF frame.
static int bounded_gif(const unsigned char* b,size_t n){
    if(n<13||memcmp(b,"GIF",3))return 0;
    size_t width=b[6]|(b[7]<<8),height=b[8]|(b[9]<<8),frames=0;
    if(!width||!height||width>2048||height>2048)return 0;
    size_t p=13+((b[10]&128)?3u*(1u<<((b[10]&7)+1)):0);
    while(p<n){
        unsigned char tag=b[p++];
        if(tag==0x3b)return frames>0;
        if(tag==0x21){if(p>=n)return 0;p++;}
        else if(tag==0x2c){
            if(p+9>n)return 0;
            unsigned char flags=b[p+8];p+=9;
            if(flags&128)p+=3u*(1u<<((flags&7)+1));
            if(p>=n)return 0;p++;
            if(++frames>512||width*height*frames*4>128u*1024*1024)return 0;
        }else return 0;
        for(;;){if(p>=n)return 0;size_t count=b[p++];if(!count)break;if(p+count>n)return 0;p+=count;}
    }
    return 0;
}
void lab_gif_close(LabGif* g){free(g->pixels);free(g->delays);memset(g,0,sizeof(*g));}
int lab_gif_open(LabGif* g,const char* path){
    lab_gif_close(g);
    const char* ext=strrchr(path,'.');if(!ext || strcasecmp(ext,".gif"))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;
    fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<=0 || size>16*1024*1024){fclose(f);return 0;}
    unsigned char* buf=malloc(size);if(!buf){fclose(f);return 0;}
    if(fread(buf,1,size,f)!=(size_t)size){free(buf);fclose(f);return 0;}fclose(f);
    if(!bounded_gif(buf,size)){free(buf);return 0;}
    int w,h,c;
    if(!stbi_info_from_memory(buf,size,&w,&h,&c)||w>2048||h>2048){free(buf);return 0;}
    g->pixels=stbi_load_gif_from_memory(buf,size,&g->delays,&g->width,&g->height,&g->frames,&c,4);
    free(buf);return g->pixels!=NULL;
}
int lab_gif_step(LabGif*g,float dt){
    if(!g->pixels||g->frames<2)return 0;
    int old=g->frame;g->clock+=dt;
    for(int i=0;i<g->frames;++i){float delay=(g->delays[g->frame]>0?g->delays[g->frame]:100)/1000.f;if(g->clock<delay)break;g->clock-=delay;g->frame=(g->frame+1)%g->frames;}
    return old!=g->frame;
}
