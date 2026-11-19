#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ConsentPopup : public BasePopup {
public:
    static ConsentPopup* create(geode::Function<void()> onAnswered = {});

private:
    bool init(geode::Function<void()> onAnswered);
    void answer(bool accepted);
    geode::Function<void()> m_onAnswered;
    void addRow(float y, std::string const& line1, std::string const& line2, const char* infoTitle, const char* infoText, bool checked, CCMenuItemToggler** out);
    void refreshAccept();

    CCMenuItemToggler* m_rules = nullptr;
    CCMenuItemToggler* m_privacy = nullptr;
    CCMenuItemSpriteExtra* m_accept = nullptr;
    ButtonSprite* m_acceptSpr = nullptr;
};

}
