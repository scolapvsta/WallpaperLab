#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
// raylib's TTF loader does not open TTC collections. Rasterize only the
// codepoints used by this catalog, without bundling system fonts or assets.
static Font LoadFallbackFont(const char* path,int size,int* codepoints,int count){
    FT_Library library;FT_Face face;
    if(FT_Init_FreeType(&library))return GetFontDefault();
    long faceIndex=0;if(path&&path[0]=='@'){char* end=NULL;faceIndex=strtol(path+1,&end,10);if(end&&*end==':')path=end+1;}
    if(FT_New_Face(library,path,faceIndex,&face)){FT_Done_FreeType(library);return GetFontDefault();}
    if(face->units_per_EM<=0||face->ascender<=face->descender){FT_Done_Face(face);FT_Done_FreeType(library);return GetFontDefault();}
    int pixels=(int)roundf(size*face->units_per_EM/(float)(face->ascender-face->descender));
    if(FT_Set_Pixel_Sizes(face,0,pixels)){FT_Done_Face(face);FT_Done_FreeType(library);return GetFontDefault();}
    // Some families only ship a Light/Thin face. Fontconfig cannot conjure a
    // heavier face, so synthesize only the missing weight, after checking the
    // active variable-font axis (a named Semibold face is already heavy enough).
    float actualWeight=(face->style_flags&FT_STYLE_FLAG_BOLD)?700:400;
    TT_OS2* os2=FT_Get_Sfnt_Table(face,ft_sfnt_os2);if(os2&&os2->usWeightClass)actualWeight=os2->usWeightClass;
    FT_MM_Var* axes=NULL;if(!FT_Get_MM_Var(face,&axes)){FT_Fixed* coordinates=MemAlloc(axes->num_axis*sizeof(FT_Fixed));if(!FT_Get_Var_Design_Coordinates(face,axes->num_axis,coordinates))for(unsigned n=0;n<axes->num_axis;n++)if(axes->axis[n].tag==0x77676874)actualWeight=coordinates[n]/65536.f;MemFree(coordinates);FT_Done_MM_Var(library,axes);}
    float requested=lab_option("font_weight")==0?500:lab_option("font_weight")==2?700:600;
    FT_Pos strength=(FT_Pos)roundf(pixels*64*.0001f*fmaxf(0,requested-actualWeight));
    Font font={0};font.baseSize=size;font.glyphCount=count;font.glyphPadding=4;
    font.glyphs=MemAlloc(count*sizeof(GlyphInfo));memset(font.glyphs,0,count*sizeof(GlyphInfo));
    for(int i=0;i<count;i++){
        GlyphInfo* glyph=&font.glyphs[i];glyph->value=codepoints[i];
        if(FT_Get_Char_Index(face,codepoints[i])&&!FT_Load_Char(face,codepoints[i],FT_LOAD_DEFAULT)){
            FT_GlyphSlot slot=face->glyph;
            if(strength>0&&slot->format==FT_GLYPH_FORMAT_OUTLINE){FT_Outline_Embolden(&slot->outline,strength);slot->advance.x+=strength/2;}
            FT_Render_Glyph(slot,FT_RENDER_MODE_NORMAL);int w=slot->bitmap.width,h=slot->bitmap.rows;
            glyph->advanceX=slot->advance.x/64;glyph->offsetX=slot->bitmap_left;
            glyph->offsetY=(int)roundf(face->ascender*pixels/(float)face->units_per_EM)-slot->bitmap_top;
            glyph->image=(Image){MemAlloc((w>0?w:1)*(h>0?h:1)),w>0?w:1,h>0?h:1,1,PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
            memset(glyph->image.data,0,glyph->image.width*glyph->image.height);
            for(int y=0;y<h;y++)for(int x=0;x<w;x++)((unsigned char*)glyph->image.data)[y*w+x]=slot->bitmap.buffer[y*slot->bitmap.pitch+x];
        }else {glyph->value=0;glyph->image=(Image){MemAlloc(1),1,1,1,PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};((unsigned char*)glyph->image.data)[0]=0;}
    }
    Image atlas=GenImageFontAtlas(font.glyphs,&font.recs,count,size,font.glyphPadding,0);
    font.texture=LoadTextureFromImage(atlas);UnloadImage(atlas);SetTextureFilter(font.texture,TEXTURE_FILTER_BILINEAR);
    FT_Done_Face(face);FT_Done_FreeType(library);return font;
}
