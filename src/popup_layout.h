#pragma once
#include <math.h>

// Logical pixels. Large lists page in grid mode instead of growing off-screen.
typedef struct PopupLayout { int width, height, tile, columns, rows; } PopupLayout;
typedef struct PopupFrame { PopupLayout content; int padding, width, height; } PopupFrame;
static inline PopupLayout popup_layout(int mode, int count, int maxWidth, int maxHeight) {
    PopupLayout p = {0};
    if(maxWidth < 1) maxWidth = 1;
    if(maxHeight < 1) maxHeight = 1;
    p.tile = 160;
    if(p.tile > maxWidth) p.tile = maxWidth;
    if(p.tile > maxHeight) p.tile = maxHeight;
    if(count <= 0) {
        p.width = maxWidth < 320 ? maxWidth : 320;
        p.height = maxHeight < 80 ? maxHeight : 80;
        return p;
    }
    if(mode == 0) {
        p.columns = (int)ceil(sqrt((double)count));
        if(p.columns > 5) p.columns = 5;
        if(p.columns > maxWidth/p.tile) p.columns = maxWidth/p.tile;
        p.rows = (count+p.columns-1)/p.columns;
        if(p.rows > 4) p.rows = 4;
        if(p.rows > maxHeight/p.tile) p.rows = maxHeight/p.tile;
        p.width = p.columns*p.tile;
        p.height = p.rows*p.tile;
    } else {
        int length = (int)ceilf(p.tile+(count-1)*p.tile*(mode == 3 ? 0.8f : 1.f));
        if(mode == 2) {
            p.width = p.tile;
            p.height = length < maxHeight ? length : maxHeight;
        } else {
            p.width = length < maxWidth ? length : maxWidth;
            p.height = p.tile;
            if(mode == 3) {
                int peak=(int)ceilf(p.tile*1.28f);
                p.height=peak+48;
                if(p.width<peak)p.width=peak<maxWidth?peak:maxWidth;
            }
            if(p.height > maxHeight) p.height = maxHeight;
        }
    }
    return p;
}

// Transparent space belongs to the native surface, not to the visible panel.
// It lets a 1.38x card cross the panel edge without hitting the window boundary.
static inline PopupFrame popup_frame(int mode,int count,int maxWidth,int maxHeight) {
    PopupFrame f={0};
    int limit=maxWidth<maxHeight?maxWidth:maxHeight;
    f.padding=count>0?(limit/4<36?limit/4:36):0;
    if(f.padding<0)f.padding=0;
    f.content=popup_layout(mode,count,maxWidth-2*f.padding,maxHeight-2*f.padding);
    f.width=f.content.width+2*f.padding;f.height=f.content.height+2*f.padding;
    return f;
}

static inline PopupFrame popup_frame_scaled(int mode,int count,int maxWidth,int maxHeight,int percent,int zoom){
 float scale=fmaxf(.7f,fminf(1.4f,percent/100.f));int pad=(int)ceilf(160*scale*fmaxf(.05f,zoom/100.f-1)/2+12);int limit=(maxWidth<maxHeight?maxWidth:maxHeight)/4;if(pad>limit)pad=limit;
 PopupFrame f={0};f.padding=count>0?pad:0;f.content=popup_layout(mode,count,fmaxf(1,(maxWidth-2*f.padding)/scale),fmaxf(1,(maxHeight-2*f.padding)/scale));f.content.tile=roundf(f.content.tile*scale);f.content.width=roundf(f.content.width*scale);f.content.height=roundf(f.content.height*scale);if(mode==3){int waveHeight=(int)ceilf(f.content.tile*zoom/100.f+80);f.content.height=waveHeight<maxHeight-2*f.padding?waveHeight:maxHeight-2*f.padding;}f.width=f.content.width+2*f.padding;f.height=f.content.height+2*f.padding;return f;
}
