// Wallpaper Lab selector, derived from danihek/hellpaper and Belial02/hellpaper (MIT).
// Modified 2026-10-26: integrated Steam catalog, renderer control, UI and safe preview lifecycle.
#include "lab.h"
#include "gif.h"
#include "popup_layout.h"
#include "popup_animation.h"
// this was vibe coded xd,
// don't take this seriosly,
// but feel free to use this.
// It turned out to be actually pretty nice,
// but someone may mind this.
//
// Peace :) - danihek

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/sysinfo.h>
#include <ctype.h>
#include <stdarg.h>

const char *postProcessingFs =
    "#version 330 core\n"
    "in vec2 fragTexCoord;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0; uniform sampler2D bloomTexture; uniform sampler2D bloomTextureHiRes;\n"
    "uniform float time; uniform vec2 resolution;\n"
    "uniform float glitchIntensity; uniform float blurIntensity;\n"
    "uniform float pixelSize; uniform int mirrorMode;\n"
    "uniform float shakeIntensity; uniform float collapseIntensity;\n"
    "uniform float scanlineIntensity;\n"
    "const float bloomFactor = 1.0; const float vignetteStrength = 0.3;\n"
    "const float grainStrength = 0.04;\n"
    "float random(vec2 st) { return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123); }\n"
    "vec2 barrelDistortion(vec2 uv, float strength) { vec2 p=uv*2.0-1.0; float r2=dot(p,p); p*=(1.0+strength*r2); return p*0.5+0.5; }\n"
    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    if (shakeIntensity > 0.0) { uv += vec2(random(vec2(time*10.0))-0.5, random(vec2(time*10.0+0.5))-0.5) * shakeIntensity * 0.1; }\n"
    "    if (pixelSize > 1.0) { uv = floor(uv * pixelSize) / pixelSize; }\n"
    "    if (mirrorMode == 1) uv.x = 1.0 - uv.x;\n"
    "    if (collapseIntensity > 0.0) { uv = barrelDistortion(uv, collapseIntensity*2.0); if(length(uv*2.0-1.0) > 1.0-collapseIntensity) {discard;} }\n"
    "    vec3 baseColor = vec3(0.0);\n"
    "    if (blurIntensity > 0.0) { \n"
    "        vec2 center = vec2(0.5); \n"
    "        baseColor += texture(texture0, uv).rgb; \n"
    "        for (int i = 1; i < 10; i++) { \n"
    "           vec2 offset = normalize(uv - center) * blurIntensity * float(i)/100.0; \n"
    "           baseColor += texture(texture0, uv + offset).rgb; \n"
    "           baseColor += texture(texture0, uv - offset).rgb; \n"
    "        } baseColor /= 21.0; \n"
    "    } else { baseColor = texture(texture0, uv).rgb; }\n"
    "    if (glitchIntensity > 0.0) { \n"
    "       float lineJitter = random(vec2(uv.y * 10.0, time)) * 0.03 * glitchIntensity; \n"
    "       float blocky = random(vec2(floor(uv.y*10.0)/10.0, time)) > 0.95 ? 0.1 : 0.0; \n"
    "       baseColor.r = texture(texture0, uv + vec2(lineJitter + blocky, 0.0)).r; \n"
    "       baseColor.g = texture(texture0, uv).g; \n"
    "       baseColor.b = texture(texture0, uv - vec2(lineJitter, 0.0)).b; \n"
    "    }\n"
    "    vec4 bloomColor = texture(bloomTexture, uv);\n"
    "    vec4 bloomColorHiRes = texture(bloomTextureHiRes, uv);\n"
    "    vec3 combinedColor = baseColor + (bloomColor.rgb + bloomColorHiRes.rgb) * bloomFactor;\n"
    "    float vignette = 1.0 - vignetteStrength * pow(length(fragTexCoord * 2.0 - 1.0), 2.0);\n"
    "    combinedColor *= vignette - collapseIntensity;\n"
    "    if (scanlineIntensity > 0.0) { combinedColor *= (1.0 - (sin(fragTexCoord.y * 400.0) * 0.5 + 0.5) * scanlineIntensity * 0.1); }\n"
    "    float grain = (random(fragTexCoord + time) - 0.5) * grainStrength;\n"
    "    combinedColor += grain;\n"
    "    finalColor = vec4(combinedColor, 1.0);\n"
    "}";
const char *blurFs =
    "#version 330 core\n"
    "in vec2 fragTexCoord;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 renderSize;\n"
    "uniform bool horizontal;\n"
    "const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n"
    "void main() {\n"
    "    vec2 texel = 1.0/renderSize;\n"
    "    vec3 result = texture(texture0, fragTexCoord).rgb * weight[0];\n"
    "    if(horizontal) {\n"
    "        for(int i=1; i<5; i++) {\n"
    "            result += texture(texture0, fragTexCoord + vec2(texel.x*i, 0.0)).rgb * weight[i];\n"
    "            result += texture(texture0, fragTexCoord - vec2(texel.x*i, 0.0)).rgb * weight[i];\n"
    "        }\n"
    "    } else {\n"
    "        for(int i=1; i<5; i++) {\n"
    "            result += texture(texture0, fragTexCoord + vec2(0.0, texel.y*i)).rgb * weight[i];\n"
    "            result += texture(texture0, fragTexCoord - vec2(0.0, texel.y*i)).rgb * weight[i];\n"
    "        }\n"
    "    }\n"
    "    finalColor = vec4(result, 1.0);\n"
    "}";

typedef struct
{
    Color bg;
    Color idle;
    Color hover;
    Color border;
    Color ripple;
    Color overlay;
    Color text;
} Theme;

Theme AppTheme;

#define MAX_POSSIBLE_WALLPAPERS 2048
#define MAX_POSSIBLE_PARTICLES 256
#define MAX_TEXTURES_TO_LOAD_PER_FRAME 4

int g_base_thumb_size;
int g_max_wallpapers;
int g_particle_count;
int g_base_padding;
int g_max_threads;
int g_max_fps;
float g_anim_speed;
float g_ken_burns_duration;
float g_border_thickness_bloom;

typedef enum {
    MODE_GRID = 0,
    MODE_RIVER_H,
    MODE_RIVER_V,
    MODE_WAVE
} LayoutMode;
#define NUM_MODES 4

typedef struct {
    float hoverAnim;
    atomic_bool loaded;
    char *path, *filename;

    Texture2D texture;
    Vector2 animPos, animSize;
} Wallpaper;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
} Particle;

typedef enum
{
    EFFECT_NONE,
    EFFECT_GLITCH,
    EFFECT_BLUR,
    EFFECT_PIXELATE,
    EFFECT_REVEAL,
    EFFECT_SHAKE,
    EFFECT_COLLAPSE
} EffectType;

static Wallpaper wallpapers[MAX_POSSIBLE_WALLPAPERS];
static Particle particles[MAX_POSSIBLE_PARTICLES];

static int wallpaper_count = 0;
static bool applying = false;
static bool popupView = false;
static int popupAnchor=-1;
static int sceneTop=154, sceneBottom=166;
static int smokeStage = -1;
static double smokeStart;
static bool smokeApplyStarted=false, smokeApplySucceeded=false;
static int smokeFramesAfterApply=0;


static atomic_int next_load_index = 0;
static atomic_bool loader_running = true;

static Image pendingImages[MAX_POSSIBLE_WALLPAPERS];
static atomic_bool imagePending[MAX_POSSIBLE_WALLPAPERS];








static bool isExiting = false;
static float masterScale = 1.0f;

static char searchBuffer[256] = {0};
static int searchBufferCount = 0;
static bool isSearching = false;

static float kenBurnsTimer = 0.0f;

static EffectType g_startupEffect = EFFECT_NONE;
static EffectType g_keypressEffect = EFFECT_NONE;
static EffectType g_exitEffect = EFFECT_NONE;

static float g_effectIntensity = 0.0f;
static float g_effectTimer = 0.0f;
static float g_effectDuration = 0.0f;
static EffectType g_activeEffect = EFFECT_NONE;

static LayoutMode g_currentMode = MODE_GRID;
static LayoutMode g_targetMode = MODE_GRID;
static float g_modeTransitionTimer = 0.0f;
static const float g_modeTransitionDuration = 1.0f;

static Vector2 g_scroll = {0, 0};
static float g_smoothScrollY = 0.0f;
static float g_smoothScrollX = 0.0f;

static int g_hoveredIndex = -1;
static int selected_index = -1;


int g_win_width;
int g_win_height;

void LogMessage(int level, const char *format, ...)
{
    const char* level_str = "";
    const char* color_code = "";
    const char* color_reset = "\033[0m";

    switch (level) {
        case LOG_INFO:
            level_str = "INFO";
            color_code = "\033[0;34m";
            break;
        case LOG_WARNING:
            level_str = "WARN";
            color_code = "\033[0;33m";
            break;
        case LOG_ERROR:
            level_str = "ERROR";
            color_code = "\033[0;31m";
            break;
    }

    fprintf(stderr, "%s[%s] ", color_code, level_str);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "%s\n", color_reset);
}

static const char *get_home_dir()
{
    const char *home = getenv("HOME");
    if (!home)
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
        {
            home = pw->pw_dir;
        }
    }
    return home ? home : ".";
}

const char* stristr(const char* haystack, const char* needle)
{
    if (!needle || !*needle)
    {
        return haystack;
    }
    for (; *haystack; ++haystack)
    {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n)))
        {
            h++;
            n++;
        }
        if (!*n)
        {
            return haystack;
        }
    }
    return NULL;
}

void TriggerParticleBurst(Vector2 pos)
{
    for (int i = 0; i < g_particle_count; i++)
    {
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float speed = (float)GetRandomValue(200, 400);
        particles[i] = (Particle){
            .pos = pos,
            .vel = {sinf(angle) * speed, cosf(angle) * speed},
            .life = 1.0f,
            .color = Fade(AppTheme.ripple, 0.5f + (float)GetRandomValue(0, 50) / 100.0f)
        };
    }
}

static Image LoadThumbnailImage(const char *fp)
{
    Image i = *fp ? LoadImage(fp) : (Image){0};
    if (!i.data) i = GenImageColor(150,150,(Color){34,44,48,255});
    if (i.data)
    {
        if(popupView){
            int cs=i.width<i.height?i.width:i.height;
            ImageCrop(&i,(Rectangle){(i.width-cs)/2.f,(i.height-cs)/2.f,cs,cs});
            ImageResize(&i,g_base_thumb_size,g_base_thumb_size);
        }else{
            float scale=fminf(1,g_base_thumb_size/(float)(i.width>i.height?i.width:i.height));
            ImageResize(&i,fmaxf(1,i.width*scale),fmaxf(1,i.height*scale));
        }
    }
    else
    {
        LogMessage(LOG_WARNING, "Failed to load thumbnail for: %s", fp);
    }
    return i;
}

void *LoaderThread(void* a)
{
    while (atomic_load(&loader_running))
    {
        int i = atomic_fetch_add(&next_load_index, 1);
        if (i >= wallpaper_count)
        {
            if(popupView)break;
            usleep(100000);
            continue;
        }
        if(popupView && !lab_favorite(i)&&i!=popupAnchor)continue;
        if (!atomic_load(&imagePending[i]) && !atomic_load(&wallpapers[i].loaded))
        {
            Image img = LoadThumbnailImage(wallpapers[i].path);
            if (img.data)
            {
                pendingImages[i] = img;
                atomic_store(&imagePending[i], true);
            }
        }
    }
    return NULL;
}

void TriggerEffect(EffectType type, float duration)
{
    if (type == EFFECT_NONE)
    {
        return;
    }
    g_activeEffect = type;
    g_effectTimer = duration;
    g_effectDuration = duration;
}

EffectType ParseEffect(const char* arg)
{
    if (strcasecmp(arg, "none") == 0) return EFFECT_NONE;
    if (strcasecmp(arg, "glitch") == 0) return EFFECT_GLITCH;
    if (strcasecmp(arg, "blur") == 0) return EFFECT_BLUR;
    if (strcasecmp(arg, "pixelate") == 0) return EFFECT_PIXELATE;
    if (strcasecmp(arg, "reveal") == 0) return EFFECT_REVEAL;
    if (strcasecmp(arg, "shake") == 0) return EFFECT_SHAKE;
    if (strcasecmp(arg, "collapse") == 0) return EFFECT_COLLAPSE;

    LogMessage(LOG_WARNING, "Unrecognized effect '%s'. Defaulting to 'none'.", arg);
    return EFFECT_NONE;
}

static void LoadWallpapers(const char *dir)
{
    (void)dir;
    wallpaper_count = lab_count();
    for (int i=0; i<wallpaper_count; ++i) {
        wallpapers[i] = (Wallpaper){.path=strdup(lab_preview(i)), .filename=strdup(lab_title(i)),
            .loaded=false, .hoverAnim=0, .animPos={-200,800}, .animSize={g_base_thumb_size,g_base_thumb_size}};
        atomic_store(&imagePending[i],false);
    }
}

void LoadDefaultConfig()
{
    AppTheme.bg = (Color){13, 18, 23, 255};
    AppTheme.idle = (Color){30, 30, 46, 255};
    AppTheme.hover = (Color){49, 50, 68, 255};
    AppTheme.border = (Color){133, 223, 197, 255};
    AppTheme.ripple = (Color){245, 194, 231, 255};
    AppTheme.overlay = (Color){10, 10, 15, 200};
    AppTheme.text = (Color){202, 212, 241, 255};

    g_startupEffect = EFFECT_BLUR;
    g_keypressEffect = EFFECT_NONE;
    g_exitEffect = EFFECT_COLLAPSE;

    g_max_wallpapers = 512;
    g_base_thumb_size = 180;
    g_base_padding = 24;
    g_border_thickness_bloom = 3.0f;

    g_max_threads = 4;
    g_anim_speed = 12.0f;
    g_particle_count = 50;
    g_ken_burns_duration = 15.0f;
    g_max_fps = 60;

    g_win_width = 1280;
    g_win_height = 720;
}

char* trim_whitespace(char* str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void ParseConfigFile()
{
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/.config/wallpaper-lab", get_home_dir());
    mkdir(config_path, 0755);
    snprintf(config_path, sizeof(config_path), "%s/.config/wallpaper-lab/selector.conf", get_home_dir());

    FILE *file = fopen(config_path, "r");
    if (!file) {
        LogMessage(LOG_INFO, "Config file not found at '%s'. Using default settings.", config_path);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n') continue;

        char* key = strtok(line, "=");
        char* value = strtok(NULL, "\n");

        if (key && value) {
            key = trim_whitespace(key);
            value = trim_whitespace(value);

            if (strcmp(key, "bg") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.bg.r, &AppTheme.bg.g, &AppTheme.bg.b, &AppTheme.bg.a);
            else if (strcmp(key, "width") == 0) g_win_width = atoi(value);
            else if (strcmp(key, "height") == 0) g_win_height = atoi(value);
            else if (strcmp(key, "idle") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.idle.r, &AppTheme.idle.g, &AppTheme.idle.b, &AppTheme.idle.a);
            else if (strcmp(key, "hover") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.hover.r, &AppTheme.hover.g, &AppTheme.hover.b, &AppTheme.hover.a);
            else if (strcmp(key, "border") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.border.r, &AppTheme.border.g, &AppTheme.border.b, &AppTheme.border.a);
            else if (strcmp(key, "ripple") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.ripple.r, &AppTheme.ripple.g, &AppTheme.ripple.b, &AppTheme.ripple.a);
            else if (strcmp(key, "overlay") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.overlay.r, &AppTheme.overlay.g, &AppTheme.overlay.b, &AppTheme.overlay.a);
            else if (strcmp(key, "text") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.text.r, &AppTheme.text.g, &AppTheme.text.b, &AppTheme.text.a);

            else if (strcmp(key, "max_wallpapers") == 0) g_max_wallpapers = atoi(value);
            else if (strcmp(key, "base_thumb_size") == 0) g_base_thumb_size = atoi(value);
            else if (strcmp(key, "base_padding") == 0) g_base_padding = atoi(value);
            else if (strcmp(key, "border_thickness_bloom") == 0) g_border_thickness_bloom = atof(value);
            else if (strcmp(key, "max_threads") == 0) g_max_threads = atoi(value);
            else if (strcmp(key, "anim_speed") == 0) g_anim_speed = atof(value);
            else if (strcmp(key, "particle_count") == 0) g_particle_count = atoi(value);
            else if (strcmp(key, "ken_burns_duration") == 0) g_ken_burns_duration = atof(value);
            else if (strcmp(key, "max_fps") == 0) g_max_fps = atoi(value);

            else if (strcmp(key, "startup_effect") == 0) g_startupEffect = ParseEffect(value);
            else if (strcmp(key, "keypress_effect") == 0) g_keypressEffect = ParseEffect(value);
            else if (strcmp(key, "exit_effect") == 0) g_exitEffect = ParseEffect(value);
        }
    }
    fclose(file);
}

static void UiText(const char*,int,int,int,Color);
static int UiMeasure(const char*,int);
#define DrawText UiText
#define MeasureText UiMeasure

void UpdateAndDrawScene(int filteredCount, int* filteredIndices, float delta, bool isPreviewing, bool isSearching)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    if (g_modeTransitionTimer > 0)
    {
        g_modeTransitionTimer -= delta;
        if (g_modeTransitionTimer <= 0)
        {
            g_currentMode = g_targetMode;
        }
    }

    for (int j = 0; j < filteredCount; j++)
    {
        int i = filteredIndices[j];
        Vector2 targetPos2D = {0}, targetSize2D = {0};
        switch (g_targetMode)
        {
            case MODE_GRID:
            {
                float ts = g_base_thumb_size * masterScale, p = g_base_padding * masterScale;
                int c = (sw-p) / (ts + p);
                if (c < 1) c = 1;
                targetPos2D = (Vector2){(j % c) * (ts + p) + p, (j / c) * (ts + p + 38) + p + sceneTop - g_smoothScrollY};
                targetSize2D = (Vector2){ts, ts};
                break;
            }
            case MODE_RIVER_H:
            {
                float x = j * (g_base_thumb_size * 0.7f * masterScale) - g_smoothScrollX + sw / 2.f - g_base_thumb_size * masterScale / 2.f;
                float d = fabs(x - (sw / 2.f - g_base_thumb_size / 2.f));
                float s = Remap(d, 0, sw / 2.f, 1.5f, 0.5f);
                s = Clamp(s, 0.5, 1.5) * masterScale;
                targetPos2D = (Vector2){x, sh / 2.f - (g_base_thumb_size * s) / 2.f};
                targetSize2D = (Vector2){g_base_thumb_size * s, g_base_thumb_size * s};
                break;
            }
            case MODE_RIVER_V:
            {
                float y = j * (g_base_thumb_size * 0.7f * masterScale) - g_smoothScrollY + sh / 2.f - g_base_thumb_size * masterScale / 2.f;
                float d = fabs(y - (sh / 2.f - g_base_thumb_size / 2.f));
                float s = Remap(d, 0, sh / 2.f, 1.5f, 0.5f);
                s = Clamp(s, 0.5, 1.5) * masterScale;
                targetPos2D = (Vector2){sw / 2.f - (g_base_thumb_size * s) / 2.f, y};
                targetSize2D = (Vector2){g_base_thumb_size * s, g_base_thumb_size * s};
                break;
            }
            case MODE_WAVE:
            {
                float x = j * (g_base_thumb_size * 0.8f * masterScale) - g_smoothScrollX + sw / 2.f - g_base_thumb_size * masterScale / 2.f;
                float yOff = sinf(j * 0.2f + GetTime() * 2.f) * 80.f * masterScale;
                targetPos2D = (Vector2){x, sh / 2.f + yOff - g_base_thumb_size / 2.f};
                targetSize2D = (Vector2){g_base_thumb_size * masterScale, g_base_thumb_size * masterScale};
                break;
            }
        }
        wallpapers[i].animPos = Vector2Lerp(wallpapers[i].animPos, targetPos2D, delta * g_anim_speed);
        wallpapers[i].animSize = Vector2Lerp(wallpapers[i].animSize, targetSize2D, delta * g_anim_speed);
    }

    if (true)
    {
        g_hoveredIndex = -1;
        for (int j = filteredCount - 1; j >= 0; j--)
        {
            int i = filteredIndices[j];
            Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
            if (mouse.y>sceneTop && mouse.y<sh-sceneBottom && CheckCollisionPointRec(mouse, rect) && !isPreviewing && !isSearching)
            {
                g_hoveredIndex = i;
                break;
            }
        }
    }

    int focus = !popupView && g_hoveredIndex >= 0 ? g_hoveredIndex : selected_index;
    int highlighted_j = -1;
    for (int j = 0; j < filteredCount; j++)
    {
        int i = filteredIndices[j];
        wallpapers[i].hoverAnim = Lerp(wallpapers[i].hoverAnim, (focus == i) ? 1.f : 0.f, delta * g_anim_speed * 2.0f);
        if (focus == i)
        {
            highlighted_j = j;
        }
    }

    for (int j = 0; j < filteredCount; j++)
    {
        if (j == highlighted_j) continue;
        int i = filteredIndices[j];
        Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
        DrawRectangleRounded(rect, 0.1f, 8, ColorLerp(AppTheme.idle, AppTheme.hover, wallpapers[i].hoverAnim));
        if(g_targetMode==MODE_GRID) {
            DrawText(TextFormat("%.22s",wallpapers[i].filename),rect.x,rect.y+rect.height+4,15,AppTheme.text);
            DrawText(lab_type(i),rect.x,rect.y+rect.height+23,12,Fade(AppTheme.border,0.7f));
        }
        if (atomic_load(&wallpapers[i].loaded))
        {
            DrawTexturePro(wallpapers[i].texture, (Rectangle){0, 0, g_base_thumb_size, g_base_thumb_size}, rect, (Vector2){0, 0}, 0, WHITE);
        }
        else
        {
            DrawCircleSectorLines(Vector2Add((Vector2){rect.x, rect.y}, (Vector2){rect.width / 2, rect.height / 2}), rect.width / 3, fmodf(GetTime() * 360, 360), fmodf(GetTime() * 360, 360) + 90, 30, Fade(AppTheme.border, 0.6f));
        }
    }

    if (highlighted_j != -1)
    {
        int i = filteredIndices[highlighted_j];
        Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
        if (!isPreviewing)
            DrawRectangleRounded(rect, 0.1f, 8, ColorLerp(AppTheme.idle, AppTheme.hover, wallpapers[i].hoverAnim));
        if(g_targetMode==MODE_GRID) {
            DrawText(TextFormat("%.22s",wallpapers[i].filename),rect.x,rect.y+rect.height+4,15,AppTheme.text);
            DrawText(lab_type(i),rect.x,rect.y+rect.height+23,12,Fade(AppTheme.border,0.7f));
        }
        if (atomic_load(&wallpapers[i].loaded))
        {
            DrawTexturePro(wallpapers[i].texture, (Rectangle){0, 0, g_base_thumb_size, g_base_thumb_size}, rect, (Vector2){0, 0}, 0, WHITE);
        }
        else
        {
            DrawCircleSectorLines(Vector2Add((Vector2){rect.x, rect.y}, (Vector2){rect.width / 2, rect.height / 2}), rect.width / 3, fmodf(GetTime() * 360, 360), fmodf(GetTime() * 360, 360) + 90, 30, Fade(AppTheme.border, 0.6f));
        }
        const char *text = TextFormat("%.65s  /  %s",wallpapers[i].filename,lab_type(i));
        float fontSize = 14.f * Clamp(masterScale, 0.8f, 1.5f);
        int textWidth = MeasureText(text, fontSize);
        if(g_targetMode!=MODE_GRID)DrawText(text, fmaxf(24,rect.x + (rect.width - textWidth) / 2), rect.y + rect.height + 5, fontSize, AppTheme.text);
    }

}

static void DrawSwitcherDemo(Rectangle area,float elapsed);
static void InitPopupImages(void);
static void PopupImages(PopupLayout layout,int count,int* indices,int position,float phase,bool enlarged,Vector2 origin);
#include "library_ui.h"

// A shared rounded texture mask keeps borders crisp at any animated size.
static Rectangle popupHitRects[MAX_POSSIBLE_WALLPAPERS];
typedef struct {int index;Rectangle rect;} PopupHit;static PopupHit popupHits[MAX_POSSIBLE_WALLPAPERS*8];static int popupHitCount;static float popupHitOffset;
static Shader popupImageShader;
static int popupSizeLoc,popupRadiusLoc,popupBorderLoc,popupCoverLoc,popupCapsLoc;
static const char* popupImageFs=
    "#version 330 core\n"
    "in vec2 fragTexCoord; in vec4 fragColor; out vec4 finalColor;\n"
    "uniform sampler2D texture0; uniform vec4 colDiffuse;\n"
    "uniform vec2 cardSize; uniform vec2 cover; uniform float radius; uniform float border; uniform float edgeCaps;\n"
    "void main(){\n"
    " vec2 uv=fragTexCoord; vec2 q=abs((uv-0.5)*cardSize)-(cardSize*0.5-vec2(radius));\n"
    " float d=length(max(q,vec2(0)))+min(max(q.x,q.y),0.0)-radius;\n"
    " float aa=max(fwidth(d),0.65); float mask=1.0-smoothstep(-aa,0.0,d);\n"
    " vec4 pixel=texture(texture0,(uv-0.5)*cover+0.5);\n"
    " float line=border>0.0?smoothstep(-border-aa,-border,d):0.0;\n"
    " float originalAlpha=pixel.a;\n"
    " if(edgeCaps>0.5){float edge=min(uv.x,1.0-uv.x)*cardSize.x;line=1.0-smoothstep(border-aa,border+aa,edge); }\n"
    " pixel=mix(pixel,vec4(0.97,0.98,1.0,1.0),line); if(edgeCaps>0.5)pixel.a=originalAlpha;\n"
    " finalColor=pixel*colDiffuse*fragColor; finalColor.a*=mask;\n"
    "}\n";
static void InitPopupImages(void){if(popupImageShader.id)return;popupImageShader=LoadShaderFromMemory(NULL,popupImageFs);popupSizeLoc=GetShaderLocation(popupImageShader,"cardSize");popupCoverLoc=GetShaderLocation(popupImageShader,"cover");popupRadiusLoc=GetShaderLocation(popupImageShader,"radius");popupBorderLoc=GetShaderLocation(popupImageShader,"border");popupCapsLoc=GetShaderLocation(popupImageShader,"edgeCaps");}
static void PopupTexture(Texture2D texture, Rectangle source, Rectangle rect, float radius, float border, bool crop, Color tint) {
    Vector2 size={rect.width,rect.height},cover={1,1};
    if(crop){
        float aspect=rect.width/rect.height;
        if(aspect>1)cover.y=1/aspect;else cover.x=aspect;
    }
    float caps=!crop&&g_targetMode==MODE_WAVE?1:0;
    rlDrawRenderBatchActive();
    SetShaderValue(popupImageShader,popupCapsLoc,&caps,SHADER_UNIFORM_FLOAT);
    SetShaderValue(popupImageShader,popupSizeLoc,&size,SHADER_UNIFORM_VEC2);
    SetShaderValue(popupImageShader,popupCoverLoc,&cover,SHADER_UNIFORM_VEC2);
    SetShaderValue(popupImageShader,popupRadiusLoc,&radius,SHADER_UNIFORM_FLOAT);
    SetShaderValue(popupImageShader,popupBorderLoc,&border,SHADER_UNIFORM_FLOAT);
    BeginShaderMode(popupImageShader);
    DrawTexturePro(texture,source,rect,(Vector2){0,0},0,tint);
    EndShaderMode();
}
static void PopupCard(int index, Rectangle rect, float radius, Vector2 origin) {
    rect.x+=origin.x;rect.y+=origin.y;popupHitRects[index]=rect;if(popupView&&popupHitCount<MAX_POSSIBLE_WALLPAPERS*8){Rectangle hit=rect;hit.x+=popupHitOffset;hit.y+=popupHitOffset;popupHits[popupHitCount++]=(PopupHit){index,hit};}
    if(atomic_load(&wallpapers[index].loaded)) {
        Texture2D texture=wallpapers[index].texture;
        PopupTexture(texture,(Rectangle){0,0,texture.width,texture.height},rect,radius,2.6f,true,WHITE);
    } else {
        DrawRectangleRounded(rect,2*radius/fminf(rect.width,rect.height),12,AppTheme.idle);
        DrawRectangleRoundedLinesEx(rect,2*radius/fminf(rect.width,rect.height),12,2.6f,WHITE);
    }
    if(index==lab_current()){Vector2 dot={rect.x+14,rect.y+rect.height-14};DrawCircleV(dot,6.5f,Fade(BLACK,.6f));DrawCircleV(dot,3.8f,WHITE);}
}
static void PopupImages(PopupLayout layout, int count, int* indices, int position, float phase, bool enlarged, Vector2 origin) {
    if(g_targetMode==MODE_GRID) {
        int capacity=layout.columns*layout.rows,start=position/capacity*capacity;
        if(!enlarged){
            for(int n=0;n<capacity;n++){
                Rectangle cell={(n%layout.columns)*layout.tile,(n/layout.columns)*layout.tile,layout.tile,layout.tile};
                if(start+n<count)PopupCard(indices[start+n],cell,10,origin);
                else{
                    cell.x+=origin.x;cell.y+=origin.y;DrawRectangleRounded(cell,0.13f,12,AppTheme.idle);
                    DrawRectangleRoundedLinesEx(cell,0.13f,12,1.2f,Fade(WHITE,0.25f));
                    StarIcon((Vector2){cell.x+cell.width/2,cell.y+cell.height/2-12},14,Fade(WHITE,0.4f),false);
                    const char* label=lab_tr("Spazio libero");UiText(label,cell.x+(cell.width-UiMeasure(label,14))/2,cell.y+cell.height/2+14,14,Fade(WHITE,0.55f));
                }
            }
            return;
        }
        for(int pass=0;pass<2;pass++)for(int n=0;n<capacity&&start+n<count;n++){int i=indices[start+n];float focus=wallpapers[i].hoverAnim;if(focus>.002f&&(pass==1)==(n+start==position)){float size=layout.tile*(1+(lab_option("switcher_zoom")/100.f-1)*focus);Rectangle cell={origin.x+(n%layout.columns)*layout.tile+(layout.tile-size)/2,origin.y+(n/layout.columns)*layout.tile+(layout.tile-size)/2,size,size};PopupCard(i,cell,10,(Vector2){0});}}

        return;
    }
    // A single spatial focus travels above the fixed, clipped strip;
    // repeated key events cannot leave multiple independently growing outlines.
    bool vertical=g_targetMode==MODE_RIVER_V,wave=g_targetMode==MODE_WAVE;
    float extent=vertical?layout.height:layout.width;
    float stride=layout.tile*(wave?0.8f:1.f);
    int closest=count>0?((int)floorf(phase+0.5f)%count):0;
    for(int pass=0;pass<(enlarged?3:1);pass++)for(int j=0;j<count;j++) {
        float d=popup_distance(j,phase,count);
        for(int copy=count==1?0:-1;copy<=(count==1?0:1);copy++) {
            float offset=(d+copy*count)*(lab_option("switcher_reverse")?-1:1);
            float focus=popup_focus(offset);
            if(enlarged&&(focus<=0||pass!=(j==closest?2:1)))continue;
            float size=layout.tile*(1+(enlarged?(lab_option("switcher_zoom")/100.f-1)*focus:0));
            float along=extent/2+offset*stride-size/2;
            float cross=(layout.tile-size)/2;
            if(wave)cross=(layout.height-size)/2+sinf(offset*0.65f+(float)GetTime()*2*lab_option("wave_speed")/100.f)*fminf(lab_option("wave_height"),(layout.height-layout.tile)/2.f);
            if(along+size < -36||along>extent+36)continue;
            Rectangle rect=vertical?(Rectangle){cross,along,size,size}:(Rectangle){along,cross,size,size};
            PopupCard(indices[j],rect,10,origin);
        }
    }
}

// The in-app demo uses the same card geometry and rounded shader as the real switcher.
static void DrawSwitcherDemo(Rectangle area,float elapsed){
 static RenderTexture2D panel={0},canvas={0};int order[MAX_POSSIBLE_WALLPAPERS],count=0;for(int j=0;j<lab_carousel_count();j++)order[count++]=lab_carousel_item(j);
 if(!count){UiText(lab_tr("Aggiungi preferiti dalla Libreria"),area.x+24,area.y+90,18,muted);return;}
 LayoutMode old=g_targetMode;g_targetMode=lab_option("carousel_layout");InitPopupImages();PopupFrame frame=popup_frame_scaled(g_targetMode,count,1100,660,lab_option("switcher_size"),lab_option("switcher_zoom"));PopupLayout layout=frame.content;
 if(panel.texture.width!=layout.width||panel.texture.height!=layout.height){if(panel.id)UnloadRenderTexture(panel);panel=LoadRenderTexture(layout.width,layout.height);}
 if(canvas.texture.width!=frame.width||canvas.texture.height!=frame.height){if(canvas.id)UnloadRenderTexture(canvas);canvas=LoadRenderTexture(frame.width,frame.height);SetTextureFilter(canvas.texture,TEXTURE_FILTER_BILINEAR);}
 int baseline=0;for(int j=0;j<count;j++)if(order[j]==lab_current())baseline=j;
 float duration=lab_option("switcher_motion")/1000.f,cycle=duration+.7f,step=floorf(elapsed/cycle),t=Clamp(fmodf(elapsed,cycle)/duration,0,1),phase=baseline+step+popup_ease(t);int position=(baseline+(int)step+1)%count;
 for(int j=0;j<count;j++)wallpapers[order[j]].hoverAnim=popup_focus(popup_distance(j,phase,count));
 BeginTextureMode(panel);ClearBackground(BLANK);if(g_targetMode!=MODE_WAVE)DrawBackdrop(layout.width,layout.height);PopupImages(layout,count,order,position,fmodf(phase,count),false,(Vector2){0});if(g_targetMode==MODE_WAVE)PopupImages(layout,count,order,position,fmodf(phase,count),true,(Vector2){0});EndTextureMode();
 BeginTextureMode(canvas);ClearBackground(BLANK);PopupTexture(panel.texture,(Rectangle){0,0,layout.width,-layout.height},(Rectangle){frame.padding,frame.padding,layout.width,layout.height},10,2.6f,false,WHITE);if(g_targetMode!=MODE_WAVE)PopupImages(layout,count,order,position,fmodf(phase,count),true,(Vector2){frame.padding,frame.padding});EndTextureMode();
 if(uiCanvasActive)rlEnableFramebuffer(uiCanvas.id);PopupFrame reference=popup_frame_scaled(g_targetMode,count,1100,660,140,150);float scale=fminf(area.width/reference.width,area.height/reference.height);Rectangle dest={area.x+(area.width-frame.width*scale)/2,area.y+(area.height-frame.height*scale)/2,frame.width*scale,frame.height*scale};float fadeIn=lab_option("switcher_fade_in")/1000.f,fadeOut=lab_option("switcher_fade_out")/1000.f,loop=fmodf(elapsed,cycle*count+fadeIn+fadeOut+.5f),visibility=loop<fadeIn?loop/fadeIn:loop>cycle*count+fadeIn?1-(loop-cycle*count-fadeIn)/fadeOut:1;
 DrawTexturePro(canvas.texture,(Rectangle){0,0,frame.width,-frame.height},dest,(Vector2){0},0,Fade(WHITE,Clamp(visibility,0,1)));g_targetMode=old;
}

// Transient carousel: lightweight previews while held, one commit on release.
int carousel_popup_main(void) {
    popupView=true;
    LoadDefaultConfig();ApplyTheme();
    g_max_threads=3;
    g_targetMode=g_currentMode=(LayoutMode)Clamp(lab_option("carousel_layout"),0,3);
    LoadWallpapers(NULL);
    int indices[MAX_POSSIBLE_WALLPAPERS],count=0;
    for(int j=0;j<lab_carousel_count();j++){
        int i=lab_carousel_item(j);
        if(i>=0&&i<wallpaper_count&&!*lab_reason(i))indices[count++]=i;
    }
    int baseline=-1;int current=lab_current();popupAnchor=current;
    for(int j=0;j<count;j++)if(indices[j]==current)baseline=j;
    if(count>0&&baseline<0&&current>=0&&current<wallpaper_count&&!*lab_reason(current)&&count<MAX_POSSIBLE_WALLPAPERS){memmove(indices+1,indices,count*sizeof(int));indices[0]=current;count++;baseline=0;}
    bool transient=baseline==0&&count>0&&!lab_favorite(indices[0]);int rotationBase=baseline,stepBase=1;
    int steps=0,released=0;
    if(lab_popup_poll(&steps,&released)<0)return -1;
    int position=popup_rotation_selection(count,rotationBase,steps,stepBase,transient);
    if(released)return position>=0?indices[position]:-1;
    PopupFrame frame=popup_frame_scaled(g_targetMode,count,lab_popup_max_width(),lab_popup_max_height(),lab_option("switcher_size"),lab_option("switcher_zoom"));
    PopupLayout layout=frame.content;
    g_base_thumb_size=256; // Keep enlarged previews sharp; layout still uses 160px cells.
    SetTraceLogLevel(LOG_WARNING);
    // Decode in parallel with window creation, not after the first frame.
    pthread_t loaders[3];for(int t=0;t<3;t++)pthread_create(&loaders[t],NULL,LoaderThread,NULL);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED|FLAG_WINDOW_TRANSPARENT|FLAG_MSAA_4X_HINT);
    InitWindow(frame.width,frame.height,"Wallpaper Lab Carousel");InitBackdrop();SetExitKey(KEY_NULL);SetTargetFPS(120);
    uiFont=LoadSystemFont(lab_font(),28,NULL,0);if(!uiFont.texture.id)uiFont=GetFontDefault();SetTextureFilter(uiFont.texture,TEXTURE_FILTER_BILINEAR);
    InitPopupImages();
    RenderTexture2D panel=LoadRenderTexture(layout.width,layout.height);
    RenderTexture2D canvas=LoadRenderTexture(frame.width,frame.height);
    Vector2 previousMouse=GetMousePosition();
    bool confirm=false,closing=false,hadFocus=false;int lastSteps=steps;double lastEvent=GetTime();
    float targetPhase=position,visibility=0;
    PopupMotion scrollMotion={position,0},zoomMotion[MAX_POSSIBLE_WALLPAPERS]={0};
    // Explicit test-only export of this app's own canvas, before the global
    // fade. It remains usable when desktop screenshots are covered by a lock.
    const char* captureDir=getenv("WALLPAPER_LAB_POPUP_CAPTURE");
    int captureStep=-1,captureStage=0;double captureStart=GetTime();
    int choice=-1;
    while(IsRenderTextureValid(canvas)&&IsRenderTextureValid(panel)){
        ApplyTheme();
        // Freeze the confirmed session during fade-out. A newer session stays
        // queued in the host instead of being swallowed by the closing window.
        if(!closing){
            int reset=lab_popup_poll(&steps,&released);
            if(reset<0)closing=true;
            else {
                if(reset){baseline=-1;for(int j=0;j<count;j++)if(indices[j]==lab_current())baseline=j;rotationBase=baseline;stepBase=1;lastSteps=-1;}
                if(lastSteps!=steps){
                    int previousPosition=position;position=popup_rotation_selection(count,rotationBase,steps,stepBase,transient);
                    if(lastSteps>=0&&steps>=lastSteps)targetPhase+=popup_rotation_distance(count,previousPosition,steps-lastSteps,transient);
                    else {targetPhase=position;scrollMotion=(PopupMotion){position,0};}
                    lastSteps=steps;lastEvent=GetTime();
                }
                Vector2 mouse=GetMousePosition();bool moved=GetTime()>.2&&Vector2Distance(mouse,previousMouse)>1;previousMouse=mouse;int hovered=-1;
                for(int hit=popupHitCount-1;hit>=0&&hovered<0;hit--)if(CheckCollisionPointRec(mouse,popupHits[hit].rect))for(int j=0;j<count;j++)if(indices[j]==popupHits[hit].index){hovered=j;break;}
                if(hovered>=0&&((g_targetMode==MODE_GRID&&moved)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT))){float delta=popup_distance(hovered,position,count);position=hovered;targetPhase+=delta;rotationBase=position;stepBase=steps;lastEvent=GetTime();}
                if(hovered>=0){SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){confirm=true;closing=true;}}
                bool focused=IsWindowFocused();if(focused)hadFocus=true;bool outside=IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!CheckCollisionPointRec(GetMousePosition(),(Rectangle){frame.padding,frame.padding,layout.width,layout.height});
                if(WindowShouldClose()||IsKeyPressed(KEY_ESCAPE)||outside||GetTime()-lastEvent>60)closing=true;
                else if(released){confirm=true;closing=true;}
            }
        }
        float dt=fminf(GetFrameTime(),0.1f);
        visibility=lab_option("reduce_motion")?(closing?0:1):popup_visibility_timed(visibility,closing,dt,lab_option(closing?"switcher_fade_out":"switcher_fade_in")/1000.f);
        if(closing&&visibility<=0)break;
        for(int j=0;j<count;j++){
            int i=indices[j];
            popup_motion_timed(&zoomMotion[i],j==position?1:0,dt,lab_option("switcher_motion")/1000.f);
            wallpapers[i].hoverAnim=Clamp(zoomMotion[i].value,0,1);
        }
        popup_motion_timed(&scrollMotion,targetPhase,dt,lab_option("switcher_motion")/1000.f);
        if(lab_option("reduce_motion")){scrollMotion=(PopupMotion){targetPhase,0};for(int j=0;j<count;j++)wallpapers[indices[j]].hoverAnim=j==position?1:0;}
        float phase=count>0?fmodf(scrollMotion.value+count,count):0;
        // Upload the selected preview first, then its neighbours.
        for(int n=0,loaded=0;n<count&&loaded<8;n++){
            int i=indices[(position+n)%count];
            if(atomic_load(&imagePending[i])){wallpapers[i].texture=LoadTextureFromImage(pendingImages[i]);SetTextureFilter(wallpapers[i].texture,TEXTURE_FILTER_BILINEAR);UnloadImage(pendingImages[i]);atomic_store(&wallpapers[i].loaded,true);atomic_store(&imagePending[i],false);loaded++;}
        }
        const char* popupTrace=getenv("WALLPAPER_LAB_POPUP_STATE");if(popupTrace){FILE* f=fopen(popupTrace,"w");if(f){fprintf(f,"{\"selected\":%d,\"position\":%d,\"baseline\":%d,\"steps\":%d,\"padding\":%d,\"width\":%d,\"height\":%d,\"tile\":%d,\"focus\":%.3f,\"hits\":[",position>=0?indices[position]:-1,position,baseline,steps,frame.padding,layout.width,layout.height,layout.tile,position>=0?wallpapers[indices[position]].hoverAnim:0);for(int hit=0;hit<popupHitCount;hit++){PopupHit h=popupHits[hit];fprintf(f,"%s[%d,%.1f,%.1f,%.1f,%.1f]",hit?",":"",h.index,h.rect.x,h.rect.y,h.rect.width,h.rect.height);}fprintf(f,"]}\n");fclose(f);}}

        memset(popupHitRects,0,sizeof(popupHitRects));popupHitCount=0;popupHitOffset=frame.padding;
        BeginTextureMode(panel);ClearBackground(BLANK);
        if(g_targetMode!=MODE_WAVE)DrawBackdrop(layout.width,layout.height);
        if(count)PopupImages(layout,count,indices,position,phase,false,(Vector2){0,0});
        if(count&&g_targetMode==MODE_WAVE)PopupImages(layout,count,indices,position,phase,true,(Vector2){0,0});
        if(!count) {
            DrawRectangleRounded((Rectangle){0,0,layout.width,layout.height},0.15f,8,AppTheme.bg);
            UiText(lab_tr("Carousel vuoto"),16,16,20,AppTheme.text);
            UiText(lab_tr("Aggiungi preferiti dalla Libreria"),16,45,14,AppTheme.border);
        }
        EndTextureMode();
        popupHitOffset=0;BeginTextureMode(canvas);ClearBackground(BLANK);
        PopupTexture(panel.texture,(Rectangle){0,0,layout.width,-layout.height},(Rectangle){frame.padding,frame.padding,layout.width,layout.height},10,2.6f,false,WHITE);
        if(count&&g_targetMode!=MODE_WAVE)PopupImages(layout,count,indices,position,phase,true,(Vector2){frame.padding,frame.padding});
        EndTextureMode();
        BeginDrawing();ClearBackground(BLANK);
        DrawTexturePro(canvas.texture,(Rectangle){0,0,frame.width,-frame.height},(Rectangle){0,0,frame.width,frame.height},(Vector2){0,0},0,Fade(WHITE,popup_ease(visibility)));
        EndDrawing();
        if(captureDir&&!closing){
            if(captureStep!=steps){captureStep=steps;captureStage=0;captureStart=GetTime();}
            const double captureTimes[]={0.04,0.15,0.48};
            if(captureStage<3&&GetTime()-captureStart>=captureTimes[captureStage]){
                Image shot=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&shot);
                ExportImage(shot,TextFormat("%s/step-%d-%d.png",captureDir,captureStep,captureStage));
                UnloadImage(shot);captureStage++;
            }
        }
    }
    SetWindowState(FLAG_WINDOW_HIDDEN);
    if(confirm&&position>=0)choice=indices[position];
    atomic_store(&loader_running,false);for(int t=0;t<3;t++)pthread_join(loaders[t],NULL);
    for(int i=0;i<wallpaper_count;i++){
        if(atomic_load(&wallpapers[i].loaded))UnloadTexture(wallpapers[i].texture);
        if(atomic_load(&imagePending[i]))UnloadImage(pendingImages[i]);
        free(wallpapers[i].path);free(wallpapers[i].filename);
    }
    if(IsRenderTextureValid(canvas))UnloadRenderTexture(canvas);
    if(IsRenderTextureValid(panel))UnloadRenderTexture(panel);
    if(backdropShader.id)UnloadShader(backdropShader);UnloadShader(popupImageShader);if(uiFont.texture.id!=GetFontDefault().texture.id)UnloadFont(uiFont);
    CloseWindow();return choice;
}
