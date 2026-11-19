// Color suggestions operate on an edited frame of the monitor carrying the UI.
// They intentionally never change alpha, text effects, geometry or typography.
static const std::array<const char*,9> colorKeys={"color","accent","background","border_color","effect_color","field_color","button_color","button_text_color","button_effect_color"};
json colors(const json& d){
    json result={{"accent",d.at("accent")}};
    for(auto id:widgets)for(auto key:colorKeys)if(d.at("widgets").at(id).contains(key))result["widgets"][id][key]=d["widgets"][id][key];
    return result;
}
json restoreColors(json d,const json& saved){d.merge_patch(saved);return validate(d);}
json applyPalette(json d,const unsigned char* rgb,int width,int height){
    if(!rgb||width<1||height<1||width*height>4096)return d;
    using Color=std::array<double,3>;
    auto luminance=[](Color c){for(auto& v:c)v=v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);return .2126*c[0]+.7152*c[1]+.0722*c[2];};
    auto mix=[](Color a,Color b,double t){for(int i=0;i<3;i++)a[i]=a[i]*(1-t)+b[i]*t;return a;};
    auto hex=[](Color c){char s[8];snprintf(s,sizeof(s),"#%02x%02x%02x",int(std::round(c[0]*255)),int(std::round(c[1]*255)),int(std::round(c[2]*255)));return std::string(s);};
    struct Bin{double weight=0;Color total{};};std::array<Bin,512> bins{};
    std::vector<Color> pixels;pixels.reserve(width*height);
    for(int n=0;n<width*height;n++){
        Color c={rgb[n*3]/255.,rgb[n*3+1]/255.,rgb[n*3+2]/255.};pixels.push_back(c);
        double chroma=*std::max_element(c.begin(),c.end())-*std::min_element(c.begin(),c.end());
        // Suppress clipped highlights and black pixels, without favoring tiny neon details.
        double weight=(.2+chroma)*(.15+4*luminance(c)*(1-luminance(c)));
        auto& bin=bins[(int(c[0]*7.99)<<6)|(int(c[1]*7.99)<<3)|int(c[2]*7.99)];bin.weight+=weight;
        for(int i=0;i<3;i++)bin.total[i]+=c[i]*weight;
    }
    auto dominant=*std::max_element(bins.begin(),bins.end(),[](auto& a,auto& b){return a.weight<b.weight;});
    Color hue=dominant.total;for(auto& c:hue)c/=dominant.weight;
    double low=*std::min_element(hue.begin(),hue.end()),high=*std::max_element(hue.begin(),hue.end());
    if(high-low<.04)hue={.48,.55,.62};else for(auto& c:hue)c=.2+.6*(c-low)/(high-low);
    Color accent=mix(hue,{.82,.84,.87},.42);d["accent"]=hex(accent);
    for(auto id:widgets){
        auto& w=d["widgets"][id];std::vector<Color> region;
        int x0=std::clamp(int(w["x"].get<double>()*width),0,width-1),y0=std::clamp(int(w["y"].get<double>()*height),0,height-1);
        int x1=std::clamp(int((w["x"].get<double>()+w["width"].get<double>())*width),x0+1,width),y1=std::clamp(int((w["y"].get<double>()+w["height"].get<double>())*height),y0+1,height);
        double average=0;for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){region.push_back(pixels[y*width+x]);average+=luminance(region.back());}average/=region.size();
        bool pale=average>.48;Color surface=pale?mix(hue,{.98,.98,.98},.94):mix(hue,{.025,.03,.04},.93);
        auto score=[&](Color text){std::vector<double> contrasts;for(auto p:region){auto bg=mix(p,surface,w["background_opacity"]);auto fg=mix(bg,text,w["opacity"]);double a=luminance(bg),b=luminance(fg);contrasts.push_back((std::max(a,b)+.05)/(std::min(a,b)+.05));}std::sort(contrasts.begin(),contrasts.end());return contrasts[contrasts.size()/10];};
        Color white={.98,.985,1},black={.025,.03,.045};bool bright=score(white)>=score(black);
        w["color"]=hex(bright?white:black);w["background"]=hex(surface);w["effect_color"]=bright?"#000000":"#ffffff";
        w["accent"]=hex(accent);w["border_color"]=hex(pale?mix(hue,black,.3):accent);
        if(std::string(id)=="login"){
            // Fields share the panel's tonal family, keeping labels legible even at full field alpha.
            w["field_color"]=hex(mix(surface,bright?white:black,.12));w["button_color"]=hex(accent);
            w["button_text_color"]=luminance(accent)>.179?"#000000":"#ffffff";w["button_effect_color"]=luminance(accent)>.179?"#ffffff":"#000000";
        }
    }
    return validate(d);
}
