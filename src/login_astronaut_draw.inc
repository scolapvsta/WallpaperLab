// Native adaptation of Keyitdev's Astronaut fields and controls (GPL-3.0-or-later).
// See assets/login/astronaut/NOTICE.md for source and attribution.
auto& a=v["astronaut"];auto controls=astronautControls(d);
auto label=[&](const char* key,const char* fallback){auto value=v.value(key,std::string());return value.empty()?std::string(fallback):value;};
if(v.value("welcome_enabled",false)){
    auto header=v;style=&header;auto value=greeting(d,state,now);double size=a["header_size"].get<double>()*scale;
    auto layout=layoutFor(value,size,w);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);g_object_unref(layout);
    double extra=header.value("effect",std::string("none"))=="none"?0:header.value("effect_depth",2.)*scale;
    auto slot=welcomeRect(d,(th+extra)/height,2*scale/height);ink(a["header_color"]);text(value,slot[0]*width,slot[1]*height,slot[2]*width,slot[3]*height,size);style=&v;
}
if(!state.status.empty()){auto r=controls[0].rect;ink(a["warning_color"]);text(state.status,r[0]*width,r[1]*height-size*2,r[2]*width,size*1.5,size*.8);}
for(auto& control:controls){
    auto r=control.rect;double xx=r[0]*width,yy=r[1]*height,ww=r[2]*width,hh=r[3]*height;int id=control.action;
    if(id<2){
        double focusedMix=state.focusMix[id]<0?double(state.focus==id):state.focusMix[id];bool focused=state.focus==id,placeholder=id==0?state.username.empty():state.input.empty();
        auto value=id==0?(placeholder?label("user_label","User name"):state.username):(placeholder?label("password_label","Password"):state.input);
        round(xx,yy,ww,hh,v.value("field_radius",20.)*scale);ink(id?a["password_background"].get<std::string>():v["field_color"].get<std::string>(),v.value("field_opacity",.2));cairo_fill(cr);
        if(focusedMix>0){round(xx,yy,ww,hh,v.value("field_radius",20.)*scale);ink(a["highlight"],a["highlight_opacity"].get<double>()*focusedMix);cairo_set_line_width(cr,std::max(2.,v.value("focus_width",0.))*scale);cairo_stroke(cr);}
        double row=hh*1.5,iconSize=row*.25;
        ink(id?a["password_icon_color"]:a["user_icon_color"]);icon(id?(state.reveal?"Password":"Password2"):"User",xx+row/2-iconSize/2,yy+hh/2-iconSize/2,iconSize);
        double left=xx+row,right=xx+ww-row,available=std::max(1.,right-left),caretX=xx+ww/2;
        cairo_save(cr);cairo_rectangle(cr,left,yy,available,hh);cairo_clip(cr);
        ink(placeholder?a["placeholder_color"].get<std::string>():id?a["password_color"].get<std::string>():v["color"].get<std::string>());
        if(id==1&&state.secret&&!state.reveal&&!placeholder){
            double radius=v.value("dot_size",2.)*scale;caretX=dots(left,yy,available,hh,radius,std::max(size*.4,3*radius),true);
        }else{
            auto layout=layoutFor(value,size);int tw,th;pango_layout_get_pixel_size(layout,&tw,&th);PangoRectangle cursor{};pango_layout_get_cursor_pos(layout,placeholder?0:std::clamp(state.cursor,0,int(value.size())),&cursor,nullptr);
            double tx=xx+(ww-tw)/2;if(tw>available)tx=left-(focused?std::max(0.,cursor.x/double(PANGO_SCALE)-available+3*scale):0);paintText(layout,tx,yy+(hh-th)/2);caretX=placeholder?xx+ww/2:tx+cursor.x/double(PANGO_SCALE);g_object_unref(layout);
        }
        if(focused&&state.caret&&!state.busy){ink(id?a["password_color"]:v["color"],state.caretAlpha);cairo_set_line_width(cr,scale);cairo_move_to(cr,caretX,yy+hh*.2);cairo_line_to(cr,caretX,yy+hh*.8);cairo_stroke(cr);}cairo_restore(cr);
    }else if(id==3){
        bool enabled=!state.username.empty()&&!state.input.empty()&&!state.busy;double blend=state.buttonMix<0?double(enabled):state.buttonMix;
        round(xx,yy,ww,hh,v.value("field_radius",20.)*scale);ink(v["button_color"],.2+.8*blend);cairo_fill(cr);ink(v["button_text_color"],.5+.5*blend);
        auto button=v;button["effect"]=v["button_effect"];button["effect_color"]=v["button_effect_color"];button["effect_depth"]=v["button_effect_depth"];button["bold"]=v["button_bold"];style=&button;
        text(state.busy?"…":label("button_label","Login"),xx,yy,ww,hh,size*v["button_text_size"].get<double>());style=&v;
    }else{
        auto plain=v;plain["bold"]=false;style=&plain;
        if(id==2){ink(a["session_color"]);text(label("session_label","Session")+" ("+state.session+")",xx,yy,ww,hh,size*.8);}
        else if(id==14){ink(a["keyboard_color"]);text(std::string("Virtual Keyboard (")+(state.keyboardOpen?"on":"off")+")",xx,yy,ww,hh,size*.8);}
        else if(id>=10&&id<=13){const char* icons[]={"Shutdown","Reboot","Suspend","Hibernate"};double extent=2*std::round(v["size"].get<double>()*.75*1.5)*scale;ink(a["system_color"]);icon(icons[id-10],xx+(ww-extent)/2,yy,extent);text(icons[id-10],xx-10*scale,yy+extent+6*scale,ww+20*scale,size*1.2,size*.8);}
        style=&v;
    }
}
if(state.keyboardOpen||state.keyboardAlpha>0){
    cairo_push_group(cr);
    auto r=keyboardRect(d);round(r[0]*width,r[1]*height,r[2]*width,r[3]*height,8*scale);ink(a["dropdown_background"],.98);cairo_fill(cr);
    std::string letters;for(auto& row:keyRows)letters+=row;
    for(auto& key:keyboardControls(d)){auto b=key.rect;double xx=b[0]*width+2*scale,yy=b[1]*height+2*scale,ww=b[2]*width-4*scale,hh=b[3]*height-4*scale;round(xx,yy,ww,hh,4*scale);ink(a["dropdown_selected"],.6);cairo_fill(cr);ink(a["dropdown_color"]);std::string value;
        if(key.action<250){char c=letters[key.action-200];if(state.keyboardShift){const std::string from="1234567890-=[];',./",to="!@#$%^&*()_+{}\":<>?";auto index=from.find(c);c=index==std::string::npos?std::toupper(c):to[index];}value=c;}
        else{const char* names[]={"Shift","Space","⌫","Enter","Hide"};value=names[key.action-250];}text(value,xx,yy,ww,hh,size);
    }
    cairo_pop_group_to_source(cr);cairo_paint_with_alpha(cr,state.keyboardAlpha<0?1:state.keyboardAlpha);
}
