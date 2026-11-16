#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class RainbowLabel : public CCNodeRGBA {
public:
    static RainbowLabel* create(std::string const& text, const char* font);
    void setColors(std::vector<ccColor3B> const& colors);
    void setSpeed(float speed) { m_speed = speed; }
    void limitLabelWidth(float width, float defaultScale, float minScale);
    CCLabelBMFont* label() const { return m_label; }

    void draw() override;

private:
    bool init(std::string const& text, const char* font);

    static constexpr size_t kMaxColors = 31;
    CCLabelBMFont* m_label = nullptr;
    CCGLProgram* m_shader = nullptr;
    struct Rgb { float r, g, b; };
    std::array<Rgb, kMaxColors + 1> m_colors{};
    int m_colorCount = 0;
    float m_speed = 1.f;
};

}
