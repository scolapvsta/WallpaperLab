#pragma once
#include <math.h>
typedef struct LibraryGridLayout {int columns;float tile;} LibraryGridLayout;
static inline LibraryGridLayout library_grid_layout(float width,float preferred){
    const float padding=20,gap=22,minimum=210;
    int columns=(int)fmaxf(1,ceilf((width-2*padding+gap)/(preferred+gap)));
    while(columns>1&&(width-2*padding-gap*(columns-1))/columns<minimum)columns--;
    float tile=fmaxf(1,fminf(300,(width-2*padding-gap*(columns-1))/columns));
    return (LibraryGridLayout){columns,tile};
}
