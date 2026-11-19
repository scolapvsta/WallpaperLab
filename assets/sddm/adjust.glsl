// Shared by the launcher preview and the Qt greeter. Colour controls use the
// same ranges and colour operations as Wallpaper Lab's desktop renderer.
vec4 labImage(sampler2D frame, vec2 uv, vec2 imageSize, vec2 outputSize,
              vec3 basic, vec4 more, vec3 geometry, vec3 placement, float crop, bool premultiplied) {
    vec2 size = max(imageSize, vec2(1.0));
    vec2 view = max(outputSize, vec2(1.0));
    vec2 ratio = view / size;
    float scale = crop > 0.5 ? max(ratio.x, ratio.y) : min(ratio.x, ratio.y);
    vec2 display = size * scale * placement.z;
    float c = cos(geometry.x), s = sin(geometry.x);
    float fit = max(abs(c) + abs(s)*view.y/view.x, abs(c) + abs(s)*view.x/view.y);
    vec2 q = (uv - 0.5) * view;
    q = mat2(c, -s, s, c) * q / max(1.0, fit);
    q = q * geometry.yz / display + 0.5;
    q += placement.xy * max(vec2(0.0), 0.5 - view / (2.0 * display));
    if (any(lessThan(q, vec2(0.0))) || any(greaterThan(q, vec2(1.0)))) return vec4(0,0,0,1);
    vec4 pixel = texture(frame, q);
    vec3 rgb = pixel.rgb;
    // Qt's offscreen texture is premultiplied; raylib's loaded PNG texture is not.
    if (premultiplied) rgb = pixel.a > 0.0 ? rgb / pixel.a : vec3(0.0);
    float luma = dot(rgb, vec3(0.2126, 0.7152, 0.0722));
    rgb = mix(vec3(luma), rgb, basic.z);
    rgb = (rgb - 0.5) * basic.y + 0.5 + basic.x;
    vec3 axis = normalize(vec3(1.0));
    rgb = rgb*cos(more.x) + cross(axis,rgb)*sin(more.x) + axis*dot(axis,rgb)*(1.0-cos(more.x));
    rgb += vec3(more.y*0.15, 0.0, -more.y*0.15);
    rgb = pow(clamp(rgb, 0.0, 1.0), vec3(1.0/max(0.25,more.z)));
    rgb *= 1.0 - more.w*smoothstep(0.25,0.72,length(uv-0.5));
    return vec4(clamp(rgb,0.0,1.0)*pixel.a,1.0);
}
