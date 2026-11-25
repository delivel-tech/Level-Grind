#include "RainbowLabel.hpp"
#include <Geode/ui/Label.hpp>

#include <chrono>

// this label was kinda stolen from globed. thanks dankmeme!! :3

namespace levelgrind {

namespace {

constexpr auto kVertex = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordRaw;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_texCoordRaw;
#endif

uniform vec2 uSize;

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoordRaw = a_texCoord;
    v_texCoord = vec2(a_position.x / uSize.x, a_position.y / uSize.y);
}
)";

constexpr auto kFragment = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_texCoordRaw;
uniform int colorCount;
uniform vec3 colors[32];
uniform float customTime;
uniform sampler2D CC_Texture0;

void main() {
    float t = fract(v_texCoord.x + customTime * 0.2);
    vec3 col = colors[0];
    if (colorCount > 1) {
        float scaledT = t * float(colorCount - 1);
        int idx = int(floor(scaledT));
        col = mix(colors[idx], colors[idx + 1], scaledT - float(idx));
    }
    gl_FragColor = vec4(col, 1.0) * v_fragmentColor * texture2D(CC_Texture0, v_texCoordRaw);
}
)";

CCGLProgram* rainbowShader() {
    auto cache = CCShaderCache::sharedShaderCache();
    if (auto program = cache->programForKey("rainbow-label"_spr)) return program;

    auto program = new CCGLProgram();
    program->initWithVertexShaderByteArray(kVertex, kFragment);
    program->addAttribute("a_position", kCCVertexAttrib_Position);
    program->addAttribute("a_texCoord", kCCVertexAttrib_TexCoords);
    program->addAttribute("a_color", kCCVertexAttrib_Color);
    if (!program->link()) {
        log::error("rainbow label shader failed to link");
        program->release();
        return nullptr;
    }
    program->updateUniforms();
    cache->addProgram(program, "rainbow-label"_spr);
    program->release();
    return program;
}

float secondsSinceStart() {
    static auto const start = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
}

}

RainbowLabel* RainbowLabel::create(std::string const& text, const char* font) {
    auto ret = new RainbowLabel;
    if (ret->init(text, font)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RainbowLabel::init(std::string const& text, const char* font) {
    if (!CCNodeRGBA::init()) return false;
    m_label = geode::Label::create(text, font);
    m_label->setAnchorPoint({ 0.f, 0.f });
    this->addChild(m_label);
    this->setCascadeOpacityEnabled(true);
    this->setContentSize(m_label->getContentSize());

    m_shader = rainbowShader();
    if (m_shader) m_label->setShaderProgram(m_shader);

    this->setColors({
        { 255, 80, 80 }, { 255, 170, 60 }, { 255, 240, 80 }, { 90, 255, 110 },
        { 70, 200, 255 }, { 130, 110, 255 }, { 240, 100, 255 },
    });
    return true;
}

void RainbowLabel::setColors(std::vector<ccColor3B> const& colors) {
    if (colors.empty()) return;
    size_t i = 0;
    for (auto const& c : colors) {
        if (i == kMaxColors) break;
        m_colors[i++] = { c.r / 255.f, c.g / 255.f, c.b / 255.f };
    }
    m_colors[i] = m_colors[0];
    m_colorCount = static_cast<int>(i + 1);
}

void RainbowLabel::setLimitLabelWidth(float width, float defaultScale, float minScale) {
    m_label->setLimitLabelWidth(width, defaultScale, minScale);
    float s = m_label->getScale();
    m_label->setScale(1.f);
    this->setContentSize(m_label->getContentSize());
    this->setScale(s);
}

void RainbowLabel::draw() {
    if (!m_shader) return;
    m_shader->use();
    auto size = m_label->getContentSize();
    m_shader->setUniformLocationWith2f(m_shader->getUniformLocationForName("uSize"), std::max(1.f, size.width), std::max(1.f, size.height));
    m_shader->setUniformLocationWith1i(m_shader->getUniformLocationForName("colorCount"), m_colorCount);
    m_shader->setUniformLocationWith3fv(m_shader->getUniformLocationForName("colors"), reinterpret_cast<GLfloat*>(m_colors.data()), static_cast<unsigned>(m_colors.size()));
    m_shader->setUniformLocationWith1f(m_shader->getUniformLocationForName("customTime"), secondsSinceStart() * m_speed);
}

}
