#!/usr/bin/env python3
"""Pango/Cairo audit of every app message in every locale, with private PNG samples."""
import argparse,ctypes as c,ctypes.util,json,pathlib
p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);catalog=json.loads((pathlib.Path(__file__).resolve().parents[1]/'src/translations.json').read_text());codes=['it','en','de','es','fr','pt','nl','pl','ro','sv','zh-CN','zh-TW','ja','ko','hi','ru','ar','bn','id','tr','vi','ur'];pang=c.CDLL(ctypes.util.find_library('pango-1.0'));pc=c.CDLL(ctypes.util.find_library('pangocairo-1.0'));ca=c.CDLL(ctypes.util.find_library('cairo'));go=c.CDLL(ctypes.util.find_library('gobject-2.0'))
def fn(lib,name,ret,*args):
 f=getattr(lib,name);f.restype=ret;f.argtypes=args;return f
ptr=c.c_void_p;integer=c.c_int;double=c.c_double;string=c.c_char_p
surface_new=fn(ca,'cairo_image_surface_create',ptr,integer,integer,integer);context_new=fn(ca,'cairo_create',ptr,ptr);layout_new=fn(pc,'pango_cairo_create_layout',ptr,ptr);font_new=fn(pang,'pango_font_description_from_string',ptr,string);setfont=fn(pang,'pango_layout_set_font_description',None,ptr,ptr);settext=fn(pang,'pango_layout_set_text',None,ptr,string,integer);unknown=fn(pang,'pango_layout_get_unknown_glyphs_count',integer,ptr);size=fn(pang,'pango_layout_get_pixel_size',None,ptr,c.POINTER(integer),c.POINTER(integer));width=fn(pang,'pango_layout_set_width',None,ptr,integer);wrap=fn(pang,'pango_layout_set_wrap',None,ptr,integer);rgb=fn(ca,'cairo_set_source_rgb',None,ptr,double,double,double);paint=fn(ca,'cairo_paint',None,ptr);move=fn(ca,'cairo_move_to',None,ptr,double,double);draw=fn(pc,'pango_cairo_show_layout',None,ptr,ptr);png=fn(ca,'cairo_surface_write_to_png',integer,ptr,string);unref=fn(go,'g_object_unref',None,ptr);fontfree=fn(pang,'pango_font_description_free',None,ptr);destroy=fn(ca,'cairo_destroy',None,ptr);surfacedestroy=fn(ca,'cairo_surface_destroy',None,ptr)
examples=['Libreria','Impostazioni','Anteprima dello switcher','Personalizza transizione','Dissolvenza in entrata','Durata del movimento','Palette dinamica','Temperatura','Specchia orizzontalmente','Specchia verticalmente','Ripristina effetti','L’attivazione prepara solo i componenti colore opzionali. La shell Caelestia resta separata.'];results={};missing=[]
for index,code in enumerate(codes):
 surface=surface_new(0,620,680);cr=context_new(surface);layout=layout_new(cr);font=font_new(b'Sans 14');setfont(layout,font);rgb(cr,.07,.09,.12);paint(cr);width(layout,-1)
 for key,values in catalog.items():
  text=key if index==0 else values[index-1];settext(layout,text.encode(),-1);count=unknown(layout)
  if count:missing.append({'locale':code,'key':key,'unknown':count})
 rgb(cr,.92,.94,.97);width(layout,580*1024);wrap(layout,2);y=12
 for key in examples:
  text=key if index==0 else catalog[key][index-1];settext(layout,text.encode(),-1);w,h=integer(),integer();size(layout,c.byref(w),c.byref(h));move(cr,16,y);draw(cr,layout);y+=h.value+14
 assert y<680,(code,y);assert png(surface,str(out/(code+'.png')).encode())==0;results[code]={'messages':len(catalog),'sample_height':y};fontfree(font);unref(layout);destroy(cr);surfacedestroy(surface)
(out/'results.json').write_text(json.dumps({'locales':results,'missing_glyphs':missing},indent=2,ensure_ascii=False));print(json.dumps({'locales':len(results),'messages':len(catalog),'missing_glyphs':len(missing)}));assert not missing,missing[:8]
