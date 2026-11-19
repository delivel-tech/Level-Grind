#include "ConsentPopup.hpp"
#include "../ClanUI.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/Consent.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/MDPopup.hpp>
#include <Geode/ui/NineSlice.hpp>

namespace levelgrind {

namespace {

constexpr CCSize kSize { 380.f, 215.f };
constexpr float kRowW = 340.f;
constexpr float kRowH = 46.f;

}

ConsentPopup* ConsentPopup::create(geode::Function<void()> onAnswered) {
    auto ret = new ConsentPopup;
    if (ret->init(std::move(onAnswered))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ConsentPopup::init(geode::Function<void()> onAnswered) {
    if (!BasePopup::init(kSize.width, kSize.height)) return false;
    m_onAnswered = std::move(onAnswered);
    this->setTitle("Before you continue...");

    auto info = CCLabelBMFont::create("You must accept these to use online features:", "chatFont.fnt");
    info->setScale(0.7f);
    info->setPosition({ centerX(), kSize.height - 46.f });
    m_mainLayer->addChild(info);

    bool on = consent::granted();
    this->addRow(kSize.height - 80.f, "I agree to follow the <cg>Level Grind rules</c>", "while using the mod",
        "Level Grind Rules", consent::kRules, on, &m_rules);
    this->addRow(kSize.height - 130.f, "I accept the <cy>privacy policy</c> and allow <cl>Level Grind</c>", "to store my level completions and stats",
        "Privacy Policy", consent::kPrivacy, on, &m_privacy);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu, 5);

    auto summary = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), [](CCObject*) {
        MDPopup::create("Summary", consent::kTerms, "OK")->show();
    });
    summary->setScale(0.7f);
    summary->m_baseScale = 0.7f;
    summary->setPosition({ kSize.width - 18.f, kSize.height - 18.f });
    menu->addChild(summary);

    Ref<ConsentPopup> self = this;
    auto declineSpr = ButtonSprite::create("Decline", 90, true, "goldFont.fnt", "GJ_button_06.png", 32.f, 0.8f);
    auto decline = CCMenuItemExt::createSpriteExtra(declineSpr, [self](CCObject*) {
        if (self) self->answer(false);
    });
    decline->setPosition({ centerX() - 60.f, 28.f });
    menu->addChild(decline);

    m_acceptSpr = ButtonSprite::create("Accept", 90, true, "goldFont.fnt", "GJ_button_01.png", 32.f, 0.8f);
    m_accept = CCMenuItemExt::createSpriteExtra(m_acceptSpr, [self](CCObject*) {
        if (!self || !self->m_rules->isToggled() || !self->m_privacy->isToggled()) return;
        self->answer(true);
    });
    m_accept->setPosition({ centerX() + 60.f, 28.f });
    menu->addChild(m_accept);

    this->refreshAccept();
    return true;
}

void ConsentPopup::addRow(float y, std::string const& line1, std::string const& line2, const char* infoTitle, const char* infoText, bool checked, CCMenuItemToggler** out) {
    auto bg = NineSlice::create("square02_small.png");
    bg->setContentSize({ kRowW, kRowH });
    bg->setOpacity(80);
    bg->setPosition({ centerX(), y });
    m_mainLayer->addChild(bg);

    float const left = centerX() - kRowW / 2.f + 12.f;
    auto first = clanui::coloredLine(line1, 0.7f, kRowW - 90.f);
    first->setPosition({ left, y + 8.f });
    m_mainLayer->addChild(first);
    auto second = clanui::coloredLine(line2, 0.7f, kRowW - 90.f);
    second->setPosition({ left, y - 8.f });
    m_mainLayer->addChild(second);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu, 5);

    std::string title = infoTitle;
    std::string text = infoText;
    auto infoBtn = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), [title, text](CCObject*) {
        MDPopup::create(title, text, "OK")->show();
    });
    infoBtn->setScale(0.75f);
    infoBtn->m_baseScale = 0.75f;
    infoBtn->setPosition({ centerX() + kRowW / 2.f - 52.f, y });
    menu->addChild(infoBtn);

    Ref<ConsentPopup> self = this;
    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [self](CCMenuItemToggler*) {
        if (self) Loader::get()->queueInMainThread([self] { if (self) self->refreshAccept(); });
    });
    toggler->toggle(checked);
    toggler->setPosition({ centerX() + kRowW / 2.f - 20.f, y });
    menu->addChild(toggler);
    *out = toggler;
}

void ConsentPopup::answer(bool accepted) {
    consent::set(accepted);
    if (accepted) APIClient::getInstance().performGetToken();
    auto callback = std::move(m_onAnswered);
    this->onClose(nullptr);
    if (callback) callback();
}

void ConsentPopup::refreshAccept() {
    if (!m_accept || !m_rules || !m_privacy) return;
    bool ready = m_rules->isToggled() && m_privacy->isToggled();
    m_accept->setEnabled(ready);
    m_acceptSpr->setColor(ready ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 120, 120, 120 });
    m_acceptSpr->setOpacity(ready ? 255 : 200);
}

}
