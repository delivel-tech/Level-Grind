#include "MainLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <UIBuilder.hpp>
#include <array>
#include <fmt/format.h>

#include "../../utils/globals.hpp"
#include "../../utils/utils.hpp"

#include "../BasePopup.hpp"
#include "../popups/DiscordPopup.hpp"
#include "SettingsLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

struct DifficultyFaceInfo {
    const char* spriteFrame;
    const char* id;
    cocos2d::SEL_MenuHandler callback;
};

struct SplitDifficultyRowInfo {
    const char* title;
    int firstValue;
    int secondValue;
    const char* firstId;
    const char* secondId;
    cocos2d::SEL_MenuHandler firstCallback;
    cocos2d::SEL_MenuHandler secondCallback;
    float y;
};

bool hasDifficulty(const std::vector<int>& values, int value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void applyDifficultyPair(std::vector<int>& values, int first, int second, bool enabled, bool firstSelected, bool secondSelected) {
    safeErase(values, first);
    safeErase(values, second);

    if (!enabled) return;

    if (firstSelected) safeAdd(values, first);
    if (secondSelected) safeAdd(values, second);
}

class DifficultySelectorPopup final : public BasePopup {
public:
    static DifficultySelectorPopup* create(MainLayer* owner) {
        auto ret = new DifficultySelectorPopup;
        if (ret && ret->init(owner)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    MainLayer* m_owner = nullptr;

    bool init(MainLayer* owner) {
        if (!BasePopup::init(250.f, 200.f, "GJ_square01.png")) return false;

        m_owner = owner;
        setTitle("Difficulty Splitter");

        auto hintLabel = Build(CCLabelBMFont::create("Select exact values", "goldFont.fnt"))
            .scale(0.45f)
            .opacity(180)
            .pos(fromTop({ 0.f, 38.f }))
            .parent(m_mainLayer)
            .collect();

        static const std::array<SplitDifficultyRowInfo, 3> rows = {{
            { "Hard", 4, 5, "split-4-toggler", "split-5-toggler", menu_selector(DifficultySelectorPopup::onFourToggler), menu_selector(DifficultySelectorPopup::onFiveToggler), 38.f },
            { "Harder", 6, 7, "split-6-toggler", "split-7-toggler", menu_selector(DifficultySelectorPopup::onSixToggler), menu_selector(DifficultySelectorPopup::onSevenToggler), -4.f },
            { "Insane", 8, 9, "split-8-toggler", "split-9-toggler", menu_selector(DifficultySelectorPopup::onEightToggler), menu_selector(DifficultySelectorPopup::onNineToggler), -46.f },
        }};

        for (auto const& row : rows) {
            auto label = Build(CCLabelBMFont::create(row.title, "bigFont.fnt"))
                .scale(0.5f)
                .pos(fromCenter({ 0.f, row.y }))
                .parent(m_mainLayer)
                .collect();

            auto rowMenu = Build<CCMenu>::create()
                .layout(RowLayout::create()->setGap(12.f))
                .pos(fromCenter({ 0.f, row.y - 22.f }))
                .scale(0.7f)
                .parent(m_mainLayer)
                .collect();

            auto firstText = fmt::format("{}", row.firstValue);
            auto firstToggler = Build(CCMenuItemToggler::create(
                ButtonSprite::create(firstText.c_str(), "goldFont.fnt", "GJ_button_04.png"),
                ButtonSprite::create(firstText.c_str(), "goldFont.fnt", "GJ_button_02.png"),
                this,
                row.firstCallback
            ))
                .id(row.firstId)
                .parent(rowMenu)
                .collect();

            auto secondText = fmt::format("{}", row.secondValue);
            auto secondToggler = Build(CCMenuItemToggler::create(
                ButtonSprite::create(secondText.c_str(), "goldFont.fnt", "GJ_button_04.png"),
                ButtonSprite::create(secondText.c_str(), "goldFont.fnt", "GJ_button_02.png"),
                this,
                row.secondCallback
            ))
                .id(row.secondId)
                .parent(rowMenu)
                .collect();

            if (firstToggler) firstToggler->toggle(m_owner->isSplitDifficultySelected(row.firstValue));
            if (secondToggler) secondToggler->toggle(m_owner->isSplitDifficultySelected(row.secondValue));

            rowMenu->updateLayout();
        }

        return true;
    }

    void syncVisibleState() {
        if (!m_owner) return;

        static constexpr std::array<std::pair<int, const char*>, 6> togglers = {{
            { 4, "split-4-toggler" },
            { 5, "split-5-toggler" },
            { 6, "split-6-toggler" },
            { 7, "split-7-toggler" },
            { 8, "split-8-toggler" },
            { 9, "split-9-toggler" },
        }};

        for (auto const& [value, id] : togglers) {
            if (auto toggler = typeinfo_cast<CCMenuItemToggler*>(m_mainLayer->getChildByIDRecursive(id))) {
                toggler->toggle(m_owner->isSplitDifficultySelected(value));
            }
        }
    }

    void onFourToggler(CCObject* sender) { onSplitToggler(sender, 4); }
    void onFiveToggler(CCObject* sender) { onSplitToggler(sender, 5); }
    void onSixToggler(CCObject* sender) { onSplitToggler(sender, 6); }
    void onSevenToggler(CCObject* sender) { onSplitToggler(sender, 7); }
    void onEightToggler(CCObject* sender) { onSplitToggler(sender, 8); }
    void onNineToggler(CCObject* sender) { onSplitToggler(sender, 9); }

    void onSplitToggler(CCObject* sender, int difficulty) {
        auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
        if (!toggler || !m_owner) return;

        m_owner->setSplitDifficultySelected(difficulty, !toggler->isToggled());
        m_owner->refreshSplitDifficultyFilters();
        syncVisibleState();
    }
};

MainLayer* MainLayer::create() {
    auto ret = new MainLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MainLayer::init() {
    if (!BaseLayer::init()) return false;
    if (!initFarMenus()) return false;
    if (!initMainPanel()) return false;
    if (!initMD()) return false;

    return true;
}

bool MainLayer::initMainPanel() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto mainPanel = Build(NineSlice::create("GJ_square02.png"))
        .parent(this)
        .center()
        .contentSize({ 420.f, 290.f })
        .id("main-panel")
        .collect();

    if (!mainPanel) return false;

    auto levelGrindLogo = Build<CCSprite>::create("lg-logo.png"_spr)
        .scale(1.2f)
        .pos(centerX(), centerY() + 100.f)
        .id("level-grind-logo")
        .parent(this)
        .collect();

    if (!levelGrindLogo) return false;

    auto panel = Build(NineSlice::create("square02_small.png"))
        .id("search-panel")
        .contentSize({390.f, 200.f})
        .pos(centerX(), centerY() - 30.f)
        .opacity(100)
        .parent(this)
        .collect();

    auto optionsPanelFirst = Build(NineSlice::create("square02_small.png"))
        .id("options-panel-first")
        .contentSize({ winSize.width / 2, 30.f })
        .pos(centerX(), centerY())
        .opacity(100)
        .parent(this)
        .collect();

    auto optionsPanelSecond = Build(NineSlice::create("square02_small.png"))
        .id("options-panel-second")
        .contentSize({ winSize.width / 2, 30.f })
        .pos(centerX(), centerY() - 40.f)
        .opacity(100)
        .parent(this)
        .collect();
    
    if (!optionsPanelFirst) return false;
    if (!optionsPanelSecond) return false;

    auto ratingsMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(28.f))
        .parent(this)
        .pos(optionsPanelFirst->getPosition())
        .id("ratings-menu")
        .scale(0.65f)
        .collect();

    if (!ratingsMenu) return false;

    std::vector<DifficultyFaceInfo> difficulties = {
        { "diffIcon_01_btn_001.png", "easy-difficulty-toggler", makeCb(MainLayer::onEasyToggler) },
        { "diffIcon_02_btn_001.png", "normal-difficulty-toggler", makeCb(MainLayer::onNormalToggler) },
        { "diffIcon_03_btn_001.png", "hard-difficulty-toggler", makeCb(MainLayer::onHardToggler) },
        { "diffIcon_04_btn_001.png", "harder-difficulty-toggler", makeCb(MainLayer::onHarderToggler) },
        { "diffIcon_05_btn_001.png", "insane-difficulty-toggler", makeCb(MainLayer::onInsaneToggler) }
    };

    for (auto const& difficulty : difficulties) {
        auto spriteOff = Build(CCSprite::createWithSpriteFrameName(difficulty.spriteFrame))
            .color({ 100, 100, 100 })
            .collect();
        auto spriteOn = Build(CCSprite::createWithSpriteFrameName(difficulty.spriteFrame))
            .collect();

        Build(CCMenuItemToggler::create(spriteOff, spriteOn, this, difficulty.callback))
            .id(difficulty.id)
            .parent(ratingsMenu)
            .collect();
    }

    ratingsMenu->updateLayout();

    auto selectorMenu = Build<CCMenu>::create()
        .parent(this)
        .pos(optionsPanelFirst->getPosition() + CCPoint { optionsPanelFirst->getScaledContentWidth() / 2.f - 18.f, 0.f })
        .id("difficulty-selector-menu")
        .collect();

    if (!selectorMenu) return false;

    Build(CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png"),
        this,
        makeCb(MainLayer::onOpenDifficultySelector)
    ))
        .id("difficulty-selector-btn")
        .parent(selectorMenu)
        .collect();

    selectorMenu->setVisible(false);

    auto lengthsMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(15))
        .parent(this)
        .pos(optionsPanelSecond->getPosition())
        .id("lengths-menu")
        .scale(0.6f)
        .collect();

    std::vector<LengthInfo> lengths = {
        { Length::Short, "Short", "short-length", makeCb(MainLayer::onShortToggler) },
        { Length::Medium, "Medium", "medium-length", makeCb(MainLayer::onMediumToggler) },
        { Length::Long, "Long", "long-length", makeCb(MainLayer::onLongToggler) },
        { Length::XL, "XL", "xl-length", makeCb(MainLayer::onXLToggler) }
    };

    for (const auto& length : lengths) {
        auto labelOn = Build(CCLabelBMFont::create(length.lengthName.c_str(), "bigFont.fnt"))
            .collect();
        auto labelOff = Build(CCLabelBMFont::create(length.lengthName.c_str(), "bigFont.fnt"))
            .color({ 100, 100, 100 })
            .collect();

        auto lengthToggler = Build(CCMenuItemToggler::create(labelOff, labelOn, this, length.cb))
            .parent(lengthsMenu)
            .id(length.lengthId)
            .collect();
    }

    lengthsMenu->updateLayout();

    

    updateDifficultySelectorVisibility();

    return true;
}

void MainLayer::onShortToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Short));
}

void MainLayer::onMediumToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Medium));
}

void MainLayer::onLongToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Long));
}

void MainLayer::onXLToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::XL));
}

void MainLayer::onEasyToggler(CCObject* sender) {
    processValueOnToggler(sender, m_difficulties, 2);
    updateDifficultySelectorVisibility();
}

void MainLayer::onNormalToggler(CCObject* sender) {
    processValueOnToggler(sender, m_difficulties, 3);
    updateDifficultySelectorVisibility();
}

void MainLayer::onHardToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 4, 5, getNewTogglerState(sender), m_splitHard4, m_splitHard5);
    updateDifficultySelectorVisibility();
}

void MainLayer::onHarderToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 6, 7, getNewTogglerState(sender), m_splitHarder6, m_splitHarder7);
    updateDifficultySelectorVisibility();
}

void MainLayer::onInsaneToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 8, 9, getNewTogglerState(sender), m_splitInsane8, m_splitInsane9);
    updateDifficultySelectorVisibility();
}

void MainLayer::onOpenDifficultySelector(CCObject* sender) {
    DifficultySelectorPopup::create(this)->show();
}

bool MainLayer::isSplitDifficultySelected(int difficulty) const {
    switch (difficulty) {
        case 4: return m_splitHard4;
        case 5: return m_splitHard5;
        case 6: return m_splitHarder6;
        case 7: return m_splitHarder7;
        case 8: return m_splitInsane8;
        case 9: return m_splitInsane9;
        default: return false;
    }
}

void MainLayer::setSplitDifficultySelected(int difficulty, bool selected) {
    switch (difficulty) {
        case 4: m_splitHard4 = selected; break;
        case 5: m_splitHard5 = selected; break;
        case 6: m_splitHarder6 = selected; break;
        case 7: m_splitHarder7 = selected; break;
        case 8: m_splitInsane8 = selected; break;
        case 9: m_splitInsane9 = selected; break;
        default: return;
    }

    auto normalizePair = [](bool& first, bool& second) {
        if (!first && !second) {
            first = true;
            second = true;
        }
    };

    if (difficulty == 4 || difficulty == 5) normalizePair(m_splitHard4, m_splitHard5);
    if (difficulty == 6 || difficulty == 7) normalizePair(m_splitHarder6, m_splitHarder7);
    if (difficulty == 8 || difficulty == 9) normalizePair(m_splitInsane8, m_splitInsane9);
}

void MainLayer::refreshSplitDifficultyFilters() {
    auto hardToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("hard-difficulty-toggler"));
    auto harderToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("harder-difficulty-toggler"));
    auto insaneToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("insane-difficulty-toggler"));

    applyDifficultyPair(m_difficulties, 4, 5, hardToggler && hardToggler->isToggled(), m_splitHard4, m_splitHard5);
    applyDifficultyPair(m_difficulties, 6, 7, harderToggler && harderToggler->isToggled(), m_splitHarder6, m_splitHarder7);
    applyDifficultyPair(m_difficulties, 8, 9, insaneToggler && insaneToggler->isToggled(), m_splitInsane8, m_splitInsane9);

    updateDifficultySelectorVisibility();
}

void MainLayer::updateDifficultySelectorVisibility() {
    auto selectorMenu = this->getChildByIDRecursive("difficulty-selector-menu");
    if (!selectorMenu) return;

    auto const hasSplitDifficulty =
        hasDifficulty(m_difficulties, 4) || hasDifficulty(m_difficulties, 5) ||
        hasDifficulty(m_difficulties, 6) || hasDifficulty(m_difficulties, 7) ||
        hasDifficulty(m_difficulties, 8) || hasDifficulty(m_difficulties, 9);

    selectorMenu->setVisible(hasSplitDifficulty);
}

bool MainLayer::initMD() {
    auto versionLabel = Build(CCLabelBMFont::create(fmt::format("{}", Mod::get()->getVersion()).c_str(), "chatFont.fnt"))
        .opacity(128)
        .anchorPoint({ 1.f, 1.f })
        .pos(fromTopRight({ 5, 5 }))
        .scale(0.6f)
        .id("version-label")
        .parent(this)
        .collect();

    if (!versionLabel) return false;

    return true;
}

bool MainLayer::initFarMenus() {
    auto leftSideMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()
                    ->setGap(5)
                    ->setAxisAlignment(AxisAlignment::Start)
                    ->setAxisReverse(true))
        .parent(this)
        .contentSize(48.f, 250.f)
        .anchorPoint(0.f, 0.f)
        .scale(0.75f)
        .pos(fromBottomLeft({ 15.f, 13.f }))
        .id("left-side-menu")
        .collect();

    if (!leftSideMenu) return false;

    auto settingsBtn = Build<CCSprite>::create("settings01.png"_spr)
        .intoMenuItem([] { SettingsLayer::create()->open(); })
        .scaleMult(1.1f)
        .id("settings-btn")
        .scale(0.75f)
        .parent(leftSideMenu)
        .intoParent()
        .updateLayout();

    auto rightSideMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()
                    ->setGap(5)
                    ->setAxisAlignment(AxisAlignment::Start)
                    ->setAxisReverse(true))
        .parent(this)
        .contentSize(48.f, 250.f)
        .scale(0.75f)
        .anchorPoint(1.f, 0.f)
        .pos(fromBottomRight({ 10.f, 13.f }))
        .id("right-side-menu")
        .collect();

    if (!rightSideMenu) return false;

    auto discordBtn = Build<CCSprite>::create("discord01.png"_spr)
        .intoMenuItem([] { DiscordPopup::create()->show(); })
        .scaleMult(1.1f)
        .id("discord-btn")
        .parent(rightSideMenu)
        .intoParent()
        .updateLayout();

    return true;
}

}