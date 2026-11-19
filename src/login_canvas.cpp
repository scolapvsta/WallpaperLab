#include "login_canvas.hpp"
#include "i18n.hpp"
#include <pango/pangocairo.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>
namespace lab::login {
std::array<double,4> widgetRect(const json& d,int i){auto& w=d.at("widgets").at(widgets.at(i));return {w.at("x"),w.at("y"),w.at("width"),w.at("height")};}
int hit(const json& d,double x,double y){
    auto r=widgetRect(d,4);double a=(x-r[0])/r[2],b=(y-r[1])/r[3];
    if(a>=0&&a<=1&&b>=0&&b<=1){if(b<.28)return 0;if(b<.50)return 1;if(b<.67)return 2;if(b<.85)return 3;return 4;}
    r=widgetRect(d,2);if(d["widgets"]["media"]["enabled"].get<bool>()&&x>=r[0]&&x<=r[0]+r[2]&&y>=r[1]&&y<=r[1]+r[3])return 5;
    return -1;
}
Pixels draw(const json& d,int width,int height,const CanvasState& state,std::time_t when){
    if(width<1||height<1||int64_t(width)*height>40000000)throw std::invalid_argument("Invalid login canvas size");
    auto surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);auto cr=cairo_create(surface);
    const double scale=std::min(width/1920.,height/1080.);bool light=d.value("light",false);auto family=d.value("font",std::string("Sans"));
    unsigned accent=std::stoul(d.value("accent",std::string("#87cfdf")).substr(1),nullptr,16);
    auto color=[&](bool ink,double alpha=1.){double v=ink?(light?.10:.96):(light?.96:.035);cairo_set_source_rgba(cr,v,v+(ink?0:.008),v+(ink?0:.018),alpha);};
    auto round=[&](double x,double y,double w,double h,double radius){double r=std::min({radius,w/2,h/2});cairo_new_sub_path(cr);cairo_arc(cr,x+w-r,y+r,r,-M_PI/2,0);cairo_arc(cr,x+w-r,y+h-r,r,0,M_PI/2);cairo_arc(cr,x+r,y+h-r,r,M_PI/2,M_PI);cairo_arc(cr,x+r,y+r,r,M_PI,3*M_PI/2);cairo_close_path(cr);};
    auto panel=[&](double x,double y,double w,double h,double opacity){round(x,y,w,h,d.value("radius",20.)*scale);color(false,opacity);cairo_fill(cr);};
    bool shadow=false;
    auto text=[&](const std::string& value,double x,double y,double w,double h,double size,bool centered=false,bool bold=false){
        auto layout=pango_cairo_create_layout(cr);auto desc=pango_font_description_new();pango_font_description_set_family(desc,family.c_str());pango_font_description_set_absolute_size(desc,std::max(1.,size)*PANGO_SCALE);pango_font_description_set_weight(desc,bold?PANGO_WEIGHT_BOLD:PANGO_WEIGHT_NORMAL);pango_layout_set_font_description(layout,desc);pango_layout_set_text(layout,value.c_str(),value.size());pango_layout_set_width(layout,std::max(1,int(w*PANGO_SCALE)));pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_END);pango_layout_set_single_paragraph_mode(layout,true);pango_layout_set_alignment(layout,centered?PANGO_ALIGN_CENTER:PANGO_ALIGN_LEFT);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);
        cairo_save(cr);cairo_rectangle(cr,x,y,w,h);cairo_clip(cr);if(shadow){cairo_save(cr);cairo_set_source_rgba(cr,0,0,0,.8);cairo_move_to(cr,x+scale,y+(h-th)/2+2*scale);pango_cairo_show_layout(cr,layout);cairo_restore(cr);}cairo_move_to(cr,x,y+(h-th)/2);pango_cairo_show_layout(cr,layout);cairo_restore(cr);pango_font_description_free(desc);g_object_unref(layout);
    };
    std::tm now{};localtime_r(&when,&now);char clock[32],date[128];std::strftime(clock,sizeof(clock),"%H:%M",&now);std::strftime(date,sizeof(date),"%A, %d %B",&now);
    for(int i=0;i<int(widgets.size());i++){
        auto& v=d.at("widgets").at(widgets[i]);if(!v.at("enabled").get<bool>())continue;auto r=widgetRect(d,i);double x=r[0]*width,y=r[1]*height,w=r[2]*width,h=r[3]*height,size=v.at("size").get<double>()*scale;
        shadow=i==0||i==1||i==3;cairo_save(cr);cairo_rectangle(cr,x,y,w,h);cairo_clip(cr);color(true);
        if(i==0)text(clock,x,y,w,h,size,true,true);
        if(i==1)text(date,x,y,w,h,size,true);
        if(i==3)text(d.value("message",std::string()),x,y,w,h,size);
        if(i==2){panel(x,y,w,h,d.value("panel_opacity",.65));color(true);text(state.title.empty()?translate("Sfondo selezionato"):state.title,x+16*scale,y,w-32*scale,h*.55,size,false,true);text(state.paused?translate("Riproduci"):translate("Pausa"),x+16*scale,y+h*.5,w-32*scale,h*.45,size*.8);}
        if(i==4){
            // Native interpretation of Astronaut's icon-led fields and focus treatment.
            panel(x,y,w,h,d.value("panel_opacity",.65));
            round(x+.5*scale,y+.5*scale,w-scale,h-scale,d.value("radius",20.)*scale);color(true,.12);cairo_set_line_width(cr,scale);cairo_stroke(cr);
            double pad=24*scale,ww=w-2*pad;auto accentColor=[&](double alpha){cairo_set_source_rgba(cr,((accent>>16)&255)/255.,((accent>>8)&255)/255.,(accent&255)/255.,alpha);};
            auto field=[&](double top,double height,const std::string& label,int focus){
                double yy=y+h*top,hh=h*height,xx=x+pad;bool focused=state.focus==focus&&!state.preview;
                round(xx,yy,ww,hh,d.value("field_radius",10.)*scale);color(true,d.value("field_opacity",.10)+(focused?.05:0));cairo_fill(cr);
                if(focused){round(xx,yy,ww,hh,d.value("field_radius",10.)*scale);accentColor(.85);cairo_set_line_width(cr,std::max(1.,d.value("border_width",1.))*scale);cairo_stroke(cr);}
                double ix=xx+22*scale,iy=yy+hh/2;cairo_set_line_width(cr,1.7*scale);color(true,focused?1:.65);
                if(focus==0){cairo_arc(cr,ix,iy-4*scale,4*scale,0,2*M_PI);cairo_stroke(cr);cairo_arc(cr,ix,iy+9*scale,8*scale,M_PI,2*M_PI);cairo_stroke(cr);}
                else if(focus==1){round(ix-6*scale,iy-2*scale,12*scale,10*scale,2*scale);cairo_stroke(cr);cairo_arc(cr,ix,iy-2*scale,4*scale,M_PI,2*M_PI);cairo_stroke(cr);}
                else {round(ix-7*scale,iy-5*scale,14*scale,10*scale,2*scale);cairo_stroke(cr);cairo_move_to(cr,ix,iy+5*scale);cairo_line_to(cr,ix,iy+9*scale);cairo_stroke(cr);}
                double tx=xx+44*scale,available=ww-58*scale,caretX=tx;bool secret=focus==1&&state.secret&&!state.input.empty();
                cairo_save(cr);cairo_rectangle(cr,tx,yy,available,hh);cairo_clip(cr);color(true,focused?1:.85);
                if(secret){
                    int count=g_utf8_strlen(state.input.c_str(),-1),cursor=g_utf8_strlen(state.input.c_str(),std::clamp(state.cursor,0,int(state.input.size())));double step=12*scale;int visible=std::max(1,int((available-4*scale)/step)),start=std::max(0,cursor-visible);
                    for(int n=start;n<count&&n<start+visible;n++){double progress=n==count-1?std::clamp(double(state.inputProgress),0.,1.):1.;double radius=(2.+1.1*(1-std::pow(1-progress,3)))*scale;cairo_arc(cr,tx+4*scale+(n-start)*step,yy+hh/2,radius,0,2*M_PI);cairo_fill(cr);}caretX=tx+(cursor-start)*step+2*scale;
                }else{
                    auto layout=pango_cairo_create_layout(cr);auto desc=pango_font_description_new();pango_font_description_set_family(desc,family.c_str());pango_font_description_set_absolute_size(desc,size*.85*PANGO_SCALE);pango_layout_set_font_description(layout,desc);pango_layout_set_text(layout,label.c_str(),label.size());
                    PangoRectangle cursor{};int offset=std::clamp(state.cursor,0,int(label.size()));pango_layout_get_cursor_pos(layout,offset,&cursor,nullptr);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);double scroll=focused?std::max(0.,cursor.x/double(PANGO_SCALE)-available+3*scale):0;
                    cairo_move_to(cr,tx-scroll,yy+(hh-th)/2);pango_cairo_show_layout(cr,layout);caretX=tx+cursor.x/double(PANGO_SCALE)-scroll;pango_font_description_free(desc);g_object_unref(layout);
                }
                if(focused&&focus<2&&state.caret&&!state.busy){accentColor(1);cairo_set_line_width(cr,1.5*scale);cairo_move_to(cr,caretX,yy+hh*.29);cairo_line_to(cr,caretX,yy+hh*.71);cairo_stroke(cr);}cairo_restore(cr);
            };
            if(d.value("show_welcome",true)){color(true,.85);text(translate("Benvenuto"),x+pad,y+h*.02,ww,h*.09,size*.82,false,true);}
            field(.13,.13,state.username.empty()?translate("Utente"):state.username,0);
            std::string prompt=state.prompt=="Password"?translate("Password"):state.prompt;
            if(d.value("show_labels",true)){color(true,.65);text(prompt,x+pad,y+h*.27,ww,h*.06,size*.55);}
            field(.34,.13,state.input.empty()?prompt:state.input,1);field(.54,.10,state.session+"  ▾",2);
            round(x+pad,y+h*.70,ww,h*.12,d.value("field_radius",10.)*scale);accentColor(state.busy?.55:.95);cairo_fill(cr);cairo_set_source_rgba(cr,.045,.06,.075,1);text(state.busy?translate("Attendi la fine del caricamento"):state.preview?translate("Anteprima"):translate("Accedi"),x+pad,y+h*.70,ww,h*.12,size*.85,true,true);
            color(true,.8);text(state.status.empty()?(std::string("Esc · ")+(state.preview?translate("Chiudi"):translate("Annulla"))):state.status,x+pad,y+h*.86,ww,h*.10,size*.58,true);

        }
        cairo_restore(cr);
    }
    if(state.preview){color(true,.65);text(std::string("Wallpaper Lab · ")+translate("Anteprima")+" · Esc",16*scale,height-32*scale,width-32*scale,24*scale,14*scale);}
    cairo_surface_flush(surface);auto data=cairo_image_surface_get_data(surface);int stride=cairo_image_surface_get_stride(surface);Pixels pixels{width,height,std::vector<unsigned char>(size_t(width)*height*4)};
    for(int y=0;y<height;y++)for(int x=0;x<width;x++){auto a=data+y*stride+x*4,b=pixels.rgba.data()+(size_t(y)*width+x)*4;b[0]=a[2];b[1]=a[1];b[2]=a[0];b[3]=a[3];}
    cairo_destroy(cr);cairo_surface_destroy(surface);return pixels;
}
}
