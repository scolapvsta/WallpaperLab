#include "gif.h"
#include <stdio.h>
#include <stdlib.h>
static void check(int ok){if(!ok){fprintf(stderr,"GIF timing failed\n");exit(1);}}
int main(){
    LabGif g={0};g.frames=3;g.width=g.height=1;g.pixels=malloc(12);g.delays=malloc(3*sizeof(int));
    g.delays[0]=40;g.delays[1]=200;g.delays[2]=80;
    check(!lab_gif_step(&g,0.02f));check(g.frame==0);
    check(lab_gif_step(&g,0.03f));check(g.frame==1);
    check(!lab_gif_step(&g,0.10f));check(g.frame==1);
    check(lab_gif_step(&g,0.11f));check(g.frame==2);
    check(lab_gif_step(&g,0.07f));check(g.frame==0);
    lab_gif_close(&g);check(!g.pixels&&!g.delays);
    puts("GIF frame timing passed");return 0;
}
