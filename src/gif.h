#pragma once
typedef struct { unsigned char* pixels; int* delays; int width,height,frames,frame; float clock; } LabGif;
int lab_gif_open(LabGif*,const char*);
int lab_gif_step(LabGif*,float);
void lab_gif_close(LabGif*);
