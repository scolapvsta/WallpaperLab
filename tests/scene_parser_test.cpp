#include "WallpaperEngine/Data/Parsers/ObjectParser.h"
#include "WallpaperEngine/Data/Parsers/PropertyParser.h"
#include "WallpaperEngine/Data/Model/Property.h"
#include "WallpaperEngine/Data/Model/Project.h"
#include "WallpaperEngine/Data/Model/Wallpaper.h"
#include "WallpaperEngine/Render/Shaders/TexCoordCompatibility.h"
#include "WallpaperEngine/Render/Shaders/GLSLContext.h"
#include <iostream>
#include <stdexcept>
using WallpaperEngine::Data::JSON::JSON;
using namespace WallpaperEngine::Data::Model;
using WallpaperEngine::Data::Parsers::ObjectParser;
static void check(bool ok) { if (!ok) throw std::runtime_error("Scene parser regression"); }
int main() {
    Project project {};
    JSON object = {{"id",1},{"name","Text"},{"text",""},{"size","2 2"},{"padding","32.00000 16.00000"},{"visible",false}};
    auto parsed = ObjectParser::parse(object, project);
    auto* text = dynamic_cast<Text*>(parsed.get());
    check(text && text->padding == glm::vec2(32,16));
    object["padding"] = 12;
    parsed = ObjectParser::parse(object, project);
    check(dynamic_cast<Text*>(parsed.get())->padding == glm::vec2(12));
    object.erase("padding");
    parsed = ObjectParser::parse(object, project);
    check(dynamic_cast<Text*>(parsed.get())->padding == glm::vec2(0));
    // Legacy scene serializers leave null slots for every unrelated object type.
    object["particle"] = nullptr; object["image"] = nullptr; object["model"] = nullptr;
    parsed = ObjectParser::parse(object, project);check(dynamic_cast<Text*>(parsed.get()) != nullptr);
    auto input = WallpaperEngine::Data::Parsers::PropertyParser::parse(JSON{{"type","textinput"},{"text","City"}}, "city");
    check(input && input->getString().empty());
    input = WallpaperEngine::Data::Parsers::PropertyParser::parse(JSON{{"type","textinput"},{"value","Paris"}}, "city");check(input->getString()=="Paris");
    JSON soundObject={{"id",2},{"name","Playlist"},{"sound",JSON::array({"one.ogg","two.ogg"})},{"startsilent",true},{"playbackmode","single"},{"volume",{{"value",.35}}}};
    parsed=ObjectParser::parse(soundObject,project);auto* sound=dynamic_cast<Sound*>(parsed.get());check(sound&&sound->startSilent&&sound->sounds.size()==2&&std::abs(sound->volume->value->getFloat()-.35f)<.001f);
    // Invalid optional metadata must throw to the renderer's error boundary.
    object["pointless"] = "invalid";
    bool caught = false;
    try { (void)object.optional("pointless", 0); }
    catch (const std::exception&) { caught = true; }
    check(caught);
    using namespace WallpaperEngine::Render::Shaders;
    const std::string fragment = R"(
#version 330
#define varying in
varying vec4 v_TexCoord;
uniform vec2 u_offset;
out vec4 color;
void main() {
    vec2 uv = ((v_TexCoord * 2.0 - 1.0 - u_offset) + 1.0) / 2.0;
    vec2 explicitUV = v_TexCoord.zw + u_offset;
    vec4 wide = v_TexCoord * 2.0;
    color = vec4(uv, explicitUV) + wide;
})";
    auto converted = narrowTexCoordExpressions(fragment);
    check(converted.find("v_TexCoord.xy * 2.0") != std::string::npos);
    check(converted.find("v_TexCoord.zw + u_offset") != std::string::npos);
    check(converted.find("vec4 wide = v_TexCoord * 2.0") != std::string::npos);
    check(narrowTexCoordExpressions(converted) == converted);
    const std::string vertex = R"(#version 330
out vec4 v_TexCoord;
void main() { v_TexCoord = vec4(0.5); gl_Position = vec4(0,0,0,1); })";
    const auto compiled = GLSLContext::get().toGlsl(vertex, converted);
    check(!compiled.first.empty() && !compiled.second.empty());
    const std::string narrow = "varying vec2 v_TexCoord; vec2 uv = v_TexCoord;";
    check(narrowTexCoordExpressions(narrow) == narrow);
    std::cout << "Vector/uniform/default padding and recoverable type errors passed\n";
}
