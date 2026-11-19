#include "login_canvas.hpp"
#include "i18n.hpp"
#include <pango/pangocairo.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace lab::login {
std::array<double,4> widgetRect(const json& d,int i){auto& w=d.at("widgets").at(widgets.at(i));return {w.at("x"),w.at("y"),w.at("width"),w.at("height")};}
std::array<double,4> sessionMenuRect(const json& d,int count){
    auto r=widgetRect(d,2);double h=std::min(5,std::max(1,count))*r[3]*.085,y=r[1]+r[3]*.66;
    if(y+h>.98)y=std::max(.01,r[1]+r[3]*.53-h);
    return {r[0]+r[2]*.04,y,r[2]*.92,h};
}
int hit(const json& d,double x,double y,const CanvasState* state){
    if(state&&state->menuOpen){auto r=sessionMenuRect(d,state->sessions.size());if(x>=r[0]&&x<=r[0]+r[2]&&y>=r[1]&&y<r[1]+r[3])return 100+state->menuScroll+int((y-r[1])/(r[3]/std::min(5,std::max(1,int(state->sessions.size())))));return 6;}
    auto r=widgetRect(d,2);double a=(x-r[0])/r[2],b=(y-r[1])/r[3];
    if(a>=0&&a<=1&&b>=0&&b<=1){if(b<.28)return 0;if(b<.50)return 1;if(b<.67)return 2;if(b<.85)return 3;return 4;}return -1;
}
std::string greeting(const json& d,const CanvasState& state,const std::tm& now){
    auto& w=d["widgets"]["login"];if(w.value("dynamic_welcome",false)){std::string greeting=translate(now.tm_hour<12?"Buongiorno":now.tm_hour<18?"Buon pomeriggio":"Buonasera");return state.username.empty()?greeting:greeting+", "+state.username;}
    return w.value("welcome_text",std::string()).empty()?translate("Benvenuto"):w["welcome_text"].get<std::string>();
}
Pixels draw(const json& d,int width,int height,const CanvasState& state,std::time_t when){
    if(width<1||height<1||int64_t(width)*height>40000000)throw std::invalid_argument("Invalid login canvas size");
    auto surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);auto cr=cairo_create(surface);
    const double scale=std::min(width/1920.,height/1080.);const json* style=&d["widgets"]["login"];
    auto ink=[&](const std::string& value,double alpha=1.){unsigned c=std::stoul(value.substr(1),nullptr,16);cairo_set_source_rgba(cr,((c>>16)&255)/255.,((c>>8)&255)/255.,(c&255)/255.,alpha);};
    auto round=[&](double x,double y,double w,double h,double radius){double r=std::max(0.,std::min({radius,w/2,h/2}));cairo_new_sub_path(cr);cairo_arc(cr,x+w-r,y+r,r,-M_PI/2,0);cairo_arc(cr,x+w-r,y+h-r,r,0,M_PI/2);cairo_arc(cr,x+r,y+h-r,r,M_PI/2,M_PI);cairo_arc(cr,x+r,y+r,r,M_PI,3*M_PI/2);cairo_close_path(cr);};
    auto layoutFor=[&](const std::string& value,double size,double w=-1.){
        auto layout=pango_cairo_create_layout(cr);auto desc=pango_font_description_new();auto font=style->value("font",std::string());if(font.empty())font=d.value("font",std::string("Sans"));
        pango_font_description_set_family(desc,font.c_str());pango_font_description_set_absolute_size(desc,std::max(1.,size)*PANGO_SCALE);pango_font_description_set_weight(desc,style->value("bold",false)?PANGO_WEIGHT_BOLD:PANGO_WEIGHT_NORMAL);pango_font_description_set_style(desc,style->value("italic",false)?PANGO_STYLE_ITALIC:PANGO_STYLE_NORMAL);pango_layout_set_font_description(layout,desc);pango_font_description_free(desc);
        pango_layout_set_text(layout,value.c_str(),value.size());pango_layout_set_single_paragraph_mode(layout,true);
        if(w>=0){pango_layout_set_width(layout,std::max(1,int(w*PANGO_SCALE)));pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_END);auto align=style->value("align",std::string("center"));pango_layout_set_alignment(layout,align=="left"?PANGO_ALIGN_LEFT:align=="right"?PANGO_ALIGN_RIGHT:PANGO_ALIGN_CENTER);}
        auto attrs=pango_attr_list_new();if(style->value("underline",false))pango_attr_list_insert(attrs,pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));pango_attr_list_insert(attrs,pango_attr_letter_spacing_new(style->value("tracking",0.)*scale*PANGO_SCALE));pango_layout_set_attributes(layout,attrs);pango_attr_list_unref(attrs);return layout;
    };
    auto paintText=[&](PangoLayout* layout,double x,double y){
        cairo_save(cr);auto effect=style->value("effect",std::string("none"));double depth=style->value("effect_depth",2.)*scale;
        if(effect!="none"){ink(style->value("effect_color",std::string("#000000")),.8);int steps=effect=="extrude"?std::max(1,int(std::ceil(depth))):1;for(int n=steps;n>0;n--){double offset=depth*n/steps;cairo_move_to(cr,x+offset,y+offset);pango_cairo_show_layout(cr,layout);}}
        cairo_restore(cr);cairo_move_to(cr,x,y);pango_cairo_show_layout(cr,layout);
    };
    auto text=[&](const std::string& value,double x,double y,double w,double h,double size,bool align=true){
        auto layout=layoutFor(value,size,align?w:-1);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);cairo_save(cr);cairo_rectangle(cr,x,y,w,h);cairo_clip(cr);paintText(layout,x,y+(h-th)/2);cairo_restore(cr);g_object_unref(layout);
    };
    auto panel=[&](double x,double y,double w,double h){round(x,y,w,h,style->value("radius",20.)*scale);ink(style->value("background",std::string("#101522")),style->value("background_opacity",0.));cairo_fill(cr);double line=style->value("border_width",0.)*scale;if(line>0){round(x+line/2,y+line/2,w-line,h-line,style->value("radius",20.)*scale);ink(style->value("border_color",std::string("#87cfdf")));cairo_set_line_width(cr,line);cairo_stroke(cr);}};
    std::tm now{};localtime_r(&when,&now);
    for(int i=0;i<int(widgets.size());i++){
        auto& v=d.at("widgets").at(widgets[i]);if(!v.at("enabled").get<bool>())continue;style=&v;auto r=widgetRect(d,i);double x=r[0]*width,y=r[1]*height,w=r[2]*width,h=r[3]*height,size=v.at("size").get<double>()*scale;
        cairo_save(cr);cairo_rectangle(cr,x,y,w,h);cairo_clip(cr);cairo_push_group(cr);panel(x,y,w,h);ink(v["color"]);
        if(i==0){
            auto type=v.value("style",std::string("digital"));bool seconds=v.value("seconds",false);
            if(type=="digital"){char buffer[64];std::strftime(buffer,sizeof(buffer),v.value("format",std::string("24"))=="12"?(seconds?"%I:%M:%S %p":"%I:%M %p"):(seconds?"%H:%M:%S":"%H:%M"),&now);text(buffer,x,y,w,h,size);}
            else{
                double cx=x+w/2,cy=y+h/2,radius=std::min(w,h)*.43;auto accent=v["accent"].get<std::string>();
                auto ring=[&](double rad,double fraction,double line,const std::string& colour){ink(colour,.18);cairo_set_line_width(cr,line);cairo_arc(cr,cx,cy,rad,0,2*M_PI);cairo_stroke(cr);ink(colour);cairo_arc(cr,cx,cy,rad,-M_PI/2,-M_PI/2+fraction*2*M_PI);cairo_stroke(cr);};
                if(type=="rings"){ring(radius,((now.tm_hour%12)+now.tm_min/60.)/12.,radius*.08,v["color"]);ring(radius*.78,now.tm_min/60.,radius*.07,accent);if(seconds)ring(radius*.56,now.tm_sec/60.,radius*.05,accent);}
                else{
                    ink(v["color"],.65);cairo_set_line_width(cr,std::max(scale,radius*.015));cairo_arc(cr,cx,cy,radius,0,2*M_PI);cairo_stroke(cr);
                    for(int n=0;n<60;n++){if(type=="minimal"&&n%5)continue;double a=n*M_PI/30-M_PI/2,length=radius*(n%5?.035:.09);cairo_move_to(cr,cx+std::cos(a)*(radius-length),cy+std::sin(a)*(radius-length));cairo_line_to(cr,cx+std::cos(a)*radius,cy+std::sin(a)*radius);cairo_stroke(cr);}
                    if(type=="classic")for(int n=1;n<=12;n++){double a=n*M_PI/6-M_PI/2;auto label=std::to_string(n);text(label,cx+std::cos(a)*radius*.73-radius*.11,cy+std::sin(a)*radius*.73-radius*.1,radius*.22,radius*.2,radius*.14);}
                    auto hand=[&](double a,double length,double thick,const std::string& color){ink(color);cairo_set_line_width(cr,thick);cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);cairo_move_to(cr,cx,cy);cairo_line_to(cr,cx+std::cos(a)*length,cy+std::sin(a)*length);cairo_stroke(cr);};
                    hand(((now.tm_hour%12)+now.tm_min/60.)*M_PI/6-M_PI/2,radius*.49,radius*.055,v["color"]);hand((now.tm_min+(seconds?now.tm_sec/60.:0))*M_PI/30-M_PI/2,radius*.78,radius*.035,v["color"]);if(seconds)hand(now.tm_sec*M_PI/30-M_PI/2,radius*.84,radius*.015,accent);ink(accent);cairo_arc(cr,cx,cy,radius*.055,0,2*M_PI);cairo_fill(cr);
                }
            }
        }else if(i==1){
            auto format=v.value("format",std::string("long"));char buffer[160];std::strftime(buffer,sizeof(buffer),format=="iso"?"%Y-%m-%d":format=="short"?"%d/%m/%Y":format=="weekday"?"%A":"%A, %d %B",&now);std::string label=buffer;if(v.value("uppercase",false)){auto upper=g_utf8_strup(label.c_str(),-1);label=upper;g_free(upper);}text(label,x,y,w,h,size);
        }else{
            // Native fields share their style and hit geometry between preview and greeter.
            double pad=v.value("padding",24.)*scale,ww=w-2*pad;auto accent=v.value("button_color",std::string("#87cfdf"));
            auto field=[&](double top,double fh,const std::string& label,int focus){
                double yy=y+h*top,hh=h*fh,xx=x+pad;bool focused=state.focus==focus;
                round(xx,yy,ww,hh,v.value("field_radius",10.)*scale);ink(v["field_color"],v.value("field_opacity",.1)+(focused?.05:0));cairo_fill(cr);
                if(focused){round(xx,yy,ww,hh,v.value("field_radius",10.)*scale);ink(accent,.9);cairo_set_line_width(cr,v.value("focus_width",1.)*scale);if(v.value("focus_width",1.)>0)cairo_stroke(cr);else cairo_new_path(cr);}
                double ix=xx+22*scale,iy=yy+hh/2;cairo_set_line_width(cr,1.7*scale);ink(v["color"],focused?1:.65);
                if(focus==0){cairo_arc(cr,ix,iy-4*scale,4*scale,0,2*M_PI);cairo_stroke(cr);cairo_arc(cr,ix,iy+9*scale,8*scale,M_PI,2*M_PI);cairo_stroke(cr);}
                else if(focus==1){round(ix-6*scale,iy-2*scale,12*scale,10*scale,2*scale);cairo_stroke(cr);cairo_arc(cr,ix,iy-2*scale,4*scale,M_PI,2*M_PI);cairo_stroke(cr);}
                else{round(ix-7*scale,iy-5*scale,14*scale,10*scale,2*scale);cairo_stroke(cr);cairo_move_to(cr,ix,iy+5*scale);cairo_line_to(cr,ix,iy+9*scale);cairo_stroke(cr);}
                double tx=xx+44*scale,available=ww-58*scale,caretX=tx;bool secret=focus==1&&state.secret&&!state.input.empty();
                cairo_save(cr);cairo_rectangle(cr,tx,yy,available,hh);cairo_clip(cr);ink(v["color"],focused?1:.85);
                if(secret){int count=g_utf8_strlen(state.input.c_str(),-1),cursor=g_utf8_strlen(state.input.c_str(),std::clamp(state.cursor,0,int(state.input.size())));double radius=v.value("dot_size",3.1)*scale,step=std::max(10*scale,3*radius);int visible=std::max(1,int((available-4*scale)/step)),start=std::max(0,cursor-visible);
                    for(int n=start;n<count&&n<start+visible;n++){double progress=n==cursor-1?std::clamp(double(state.inputProgress),0.,1.):1.;cairo_arc(cr,tx+radius+(n-start)*step,yy+hh/2,radius*(.6+.4*(1-std::pow(1-progress,3))),0,2*M_PI);cairo_fill(cr);}caretX=tx+(cursor-start)*step+2*scale;
                }else{auto layout=layoutFor(label,size*.85);PangoRectangle cursor{};int offset=std::clamp(state.cursor,0,int(label.size()));pango_layout_get_cursor_pos(layout,offset,&cursor,nullptr);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);double scroll=focused?std::max(0.,cursor.x/double(PANGO_SCALE)-available+3*scale):0;paintText(layout,tx-scroll,yy+(hh-th)/2);caretX=tx+cursor.x/double(PANGO_SCALE)-scroll;g_object_unref(layout);}
                if(focused&&focus<2&&state.caret&&!state.busy){ink(accent);cairo_set_line_width(cr,1.5*scale);cairo_move_to(cr,caretX,yy+hh*.29);cairo_line_to(cr,caretX,yy+hh*.71);cairo_stroke(cr);}cairo_restore(cr);
            };
            auto label=[&](const char* key,const char* fallback){auto value=v.value(key,std::string());return value.empty()?translate(fallback):value;};
            if(v.value("welcome_enabled",true)){ink(v["color"]);text(greeting(d,state,now),x+pad,y+h*.015,ww,h*.09,size*.9,false);}
            double fh=v.value("field_height",.13);field(.13,fh,state.username.empty()?label("user_label","Utente"):state.username,0);
            auto prompt=state.prompt=="Password"?label("password_label","Password"):state.prompt;
            if(v.value("labels",true)){ink(v["color"],.7);text(prompt,x+pad,y+h*.295,ww,h*.04,size*v.value("label_size",.65),false);text(label("session_label","Sessione"),x+pad,y+h*.495,ww,h*.04,size*v.value("label_size",.65),false);}
            field(.34,fh,state.input.empty()?prompt:state.input,1);field(.54,.10,state.session+"  ▾",2);
            round(x+pad,y+h*.70,ww,h*v.value("button_height",.12),v.value("field_radius",10.)*scale);ink(accent,state.busy?.55:.95);cairo_fill(cr);ink(v["button_text_color"]);text(state.busy?translate("Attendi la fine del caricamento"):label("button_label","Accedi"),x+pad,y+h*.70,ww,h*v.value("button_height",.12),size*.85);
            ink(v["color"],.8);text(state.status.empty()?(std::string("Esc · ")+(state.preview?translate("Chiudi"):translate("Annulla"))):state.status,x+pad,y+h*.88,ww,h*.09,size*.58);
        }
        cairo_pop_group_to_source(cr);cairo_paint_with_alpha(cr,v.value("opacity",1.));cairo_restore(cr);
    }
    if(state.menuOpen&&!state.sessions.empty()){
        style=&d["widgets"]["login"];auto r=sessionMenuRect(d,state.sessions.size());double x=r[0]*width,y=r[1]*height,w=r[2]*width,h=r[3]*height,row=h/std::min(5,int(state.sessions.size()));round(x,y,w,h,8*scale);ink((*style)["background"]);cairo_fill(cr);
        for(int n=0;n<5&&n+state.menuScroll<int(state.sessions.size());n++){int index=n+state.menuScroll;if(index==state.menuIndex){round(x+3*scale,y+n*row+2*scale,w-6*scale,row-4*scale,4*scale);ink((*style)["button_color"],.35);cairo_fill(cr);}ink((*style)["color"]);text(state.sessions[index],x+14*scale,y+n*row,w-28*scale,row,style->at("size").get<double>()*scale*.85,false);}
    }
    if(state.preview){style=&d["widgets"]["login"];ink("#ffffff",.8);text(translate("Anteprima interattiva · testo di prova · Esc per chiudere"),16*scale,height-34*scale,width-32*scale,28*scale,14*scale,false);}
    cairo_surface_flush(surface);auto data=cairo_image_surface_get_data(surface);int stride=cairo_image_surface_get_stride(surface);Pixels pixels{width,height,std::vector<unsigned char>(size_t(width)*height*4)};
    for(int y=0;y<height;y++)for(int x=0;x<width;x++){auto a=data+y*stride+x*4,b=pixels.rgba.data()+(size_t(y)*width+x)*4;b[0]=a[2];b[1]=a[1];b[2]=a[0];b[3]=a[3];}
    cairo_destroy(cr);cairo_surface_destroy(surface);return pixels;
}
}
