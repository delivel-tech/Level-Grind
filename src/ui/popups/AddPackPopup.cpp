#include "AddPackPopup.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/ColorPickPopup.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"

#include "../../managers/APIClient.hpp"

#include "Geode/utils/cocos.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <algorithm>

namespace levelgrind {

namespace {

constexpr int kMinDifficulty = CustomDifficultyEnum::Auto;
constexpr int kMaxDifficulty = CustomDifficultyEnum::ExtremeDemon;

const char* difficultyFrame(int difficulty) {
    switch (difficulty) {
        case CustomDifficultyEnum::Auto: return "diffIcon_auto_btn_001.png";
        case CustomDifficultyEnum::Easy: return "diffIcon_01_btn_001.png";
        case CustomDifficultyEnum::Normal: return "diffIcon_02_btn_001.png";
        case CustomDifficultyEnum::Hard: return "diffIcon_03_btn_001.png";
        case CustomDifficultyEnum::Harder: return "diffIcon_04_btn_001.png";
        case CustomDifficultyEnum::Insane: return "diffIcon_05_btn_001.png";
        case CustomDifficultyEnum::EasyDemon: return "diffIcon_07_btn_001.png";
        case CustomDifficultyEnum::MediumDemon: return "diffIcon_08_btn_001.png";
        case CustomDifficultyEnum::HardDemon: return "diffIcon_06_btn_001.png";
        case CustomDifficultyEnum::InsaneDemon: return "diffIcon_09_btn_001.png";
        case CustomDifficultyEnum::ExtremeDemon: return "diffIcon_10_btn_001.png";
        default: return "diffIcon_00_btn_001.png";
    }
}

const char* difficultyName(int difficulty) {
    switch (difficulty) {
        case CustomDifficultyEnum::Auto: return "Auto";
        case CustomDifficultyEnum::Easy: return "Easy";
        case CustomDifficultyEnum::Normal: return "Normal";
        case CustomDifficultyEnum::Hard: return "Hard";
        case CustomDifficultyEnum::Harder: return "Harder";
        case CustomDifficultyEnum::Insane: return "Insane";
        case CustomDifficultyEnum::EasyDemon: return "Easy Demon";
        case CustomDifficultyEnum::MediumDemon: return "Medium Demon";
        case CustomDifficultyEnum::HardDemon: return "Hard Demon";
        case CustomDifficultyEnum::InsaneDemon: return "Insane Demon";
        case CustomDifficultyEnum::ExtremeDemon: return "Extreme Demon";
        default: return "N/A";
    }
}

void setFrame(CCSprite* spr, const char* frame) {
    if (!spr) return;
    if (auto sf = CCSpriteFrameCache::get()->spriteFrameByName(frame)) {
        spr->setDisplayFrame(sf);
    }
}

CCNode* makePanel(CCNode* parent, CCPoint pos, CCSize size, const char* title) {
    auto panel = Build<CCNode>::create()
        .contentSize(size)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos(pos)
        .parent(parent)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(size)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(75)
        .parent(panel);

    if (title) {
        Build(CCLabelBMFont::create(title, "goldFont.fnt"))
            .scale(0.55f)
            .anchorPoint({ 0.f, 0.5f })
            .pos({ 10.f, size.height - 13.f })
            .parent(panel);

        Build(CCLayerColor::create({ 0, 0, 0, 60 }, size.width - 16.f, 1.f))
            .pos({ 8.f, size.height - 26.f })
            .parent(panel);
    }

    return panel;
}

CCMenu* makePanelMenu(CCNode* panel) {
    return Build<CCMenu>::create()
        .contentSize(panel->getContentSize())
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(panel)
        .collect();
}

CCLabelBMFont* rowLabel(CCNode* panel, const char* text, float y) {
    return Build(CCLabelBMFont::create(text, "goldFont.fnt"))
        .scale(0.45f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 10.f, y })
        .parent(panel)
        .collect();
}

}

AddPackPopup* AddPackPopup::create() {
    auto ret = new AddPackPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AddPackPopup::init() {
    if (!BasePopup::init({ 360.f, 290.f })) return false;

    setTitle(DataManager::getInstance().getUserPosition() == GrindPosition::Owner ? "Grind Coordinator: Add Pack" : "Grind Admin: Add Pack");

    this->buildPreview();

    float const panelsY = centerY() - 18.f;
    auto infoPanel = makePanel(m_mainLayer, { centerX() - 70.f, panelsY }, { 190.f, 150.f }, "Pack");
    auto levelsPanel = makePanel(m_mainLayer, { centerX() + 98.f, panelsY }, { 130.f, 150.f }, "Levels");

    this->buildInfoPanel(infoPanel);
    this->buildLevelsPanel(levelsPanel);

    Build(ButtonSprite::create("Add Pack", "goldFont.fnt", "GJ_button_01.png", 0.9f))
        .scale(0.8f)
        .intoMenuItem([this] { this->onAdd(); })
        .pos({ m_buttonMenu->getContentWidth() / 2.f, 22.f })
        .id("add-btn")
        .parent(m_buttonMenu);

    this->updatePreview();

    return true;
}

void AddPackPopup::buildPreview() {
    CCSize const size { 330.f, 40.f };
    auto strip = makePanel(m_mainLayer, { centerX(), top() - 62.f }, size, nullptr);
    strip->setID("preview");

    m_previewDiff = Build<CCSprite>::createSpriteName(difficultyFrame(m_difficulty))
        .scale(0.75f)
        .pos({ 24.f, size.height / 2.f })
        .parent(strip)
        .collect();

    m_previewTitle = Build(CCLabelBMFont::create("Pack Title", "bigFont.fnt"))
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 46.f, size.height / 2.f + 1.f })
        .parent(strip)
        .collect();

    m_previewType = Build<CCSprite>::createSpriteName("GJ_bigStar_noShadow_001.png")
        .scale(0.5f)
        .pos({ size.width - 22.f, size.height / 2.f })
        .parent(strip)
        .collect();
}

void AddPackPopup::buildInfoPanel(CCNode* panel) {
    float const w = panel->getContentWidth();
    auto menu = makePanelMenu(panel);

    m_titleInput = Build(TextInput::create(220.f, "Pack title", "chatFont.fnt"))
        .scale(0.75f)
        .pos({ w / 2.f, 106.f })
        .id("title-input")
        .parent(panel)
        .collect();
    m_titleInput->setMaxCharCount(40);
    m_titleInput->setCommonFilter(CommonFilter::Any);
    m_titleInput->setCallback([this](auto const&) { this->updatePreview(); });

    float const diffY = 76.f;
    rowLabel(panel, "Difficulty", diffY);

    float const diffX = w - 42.f;
    m_diffIcon = Build<CCSprite>::createSpriteName(difficultyFrame(m_difficulty))
        .scale(0.6f)
        .pos({ diffX, diffY + 3.f })
        .parent(panel)
        .collect();

    m_diffLabel = Build(CCLabelBMFont::create(difficultyName(m_difficulty), "chatFont.fnt"))
        .scale(0.45f)
        .pos({ diffX, diffY - 13.f })
        .parent(panel)
        .collect();

    Build<CCSprite>::createSpriteName("edit_leftBtn_001.png")
        .scale(0.65f)
        .intoMenuItem([this] { this->stepDifficulty(-1); })
        .pos({ diffX - 27.f, diffY })
        .parent(menu);

    Build<CCSprite>::createSpriteName("edit_rightBtn_001.png")
        .scale(0.65f)
        .intoMenuItem([this] { this->stepDifficulty(1); })
        .pos({ diffX + 27.f, diffY })
        .parent(menu);

    float const rewardY = 46.f;
    rowLabel(panel, "Reward", rewardY);

    m_starSpr = Build<CCSprite>::createSpriteName("GJ_bigStar_001.png")
        .scale(0.45f)
        .collect();
    Build(m_starSpr)
        .intoMenuItem([this] { this->setStar(true); })
        .pos({ diffX - 16.f, rewardY })
        .parent(menu);

    m_moonSpr = Build<CCSprite>::createSpriteName("GJ_bigMoon_001.png")
        .scale(0.45f)
        .collect();
    Build(m_moonSpr)
        .intoMenuItem([this] { this->setStar(false); })
        .pos({ diffX + 16.f, rewardY })
        .parent(menu);

    float const colorY = 18.f;
    rowLabel(panel, "Color", colorY);

    auto colorPreview = ColorChannelSprite::create();
    colorPreview->setColor(m_color);
    colorPreview->setScale(0.65f);

    auto colorBtn = CCMenuItemExt::createSpriteExtra(colorPreview, [this, colorPreview](CCObject*) {
        auto popup = geode::ColorPickPopup::create(m_color);
        if (!popup) return;

        popup->setColorTarget(colorPreview);
        popup->setCallback([this, colorPreview](cocos2d::ccColor4B const& color) {
            m_color = { color.r, color.g, color.b };
            colorPreview->setColor(m_color);
            this->updatePreview();
        });
        popup->show();
    });
    colorBtn->setPosition({ diffX, colorY });
    colorBtn->setID("color-btn");
    menu->addChild(colorBtn);

    this->setStar(m_star);
}

void AddPackPopup::buildLevelsPanel(CCNode* panel) {
    float const w = panel->getContentWidth();

    for (int i = 0; i < 3; i++) {
        float const y = 100.f - 32.f * i;

        Build(CCLabelBMFont::create(numToString(i + 1).c_str(), "goldFont.fnt"))
            .scale(0.55f)
            .pos({ 15.f, y })
            .parent(panel);

        m_levelInputs[i] = Build(TextInput::create(130.f, "Level ID", "chatFont.fnt"))
            .scale(0.75f)
            .pos({ w / 2.f + 8.f, y })
            .parent(panel)
            .collect();
        m_levelInputs[i]->setCommonFilter(CommonFilter::Uint);
        m_levelInputs[i]->setMaxCharCount(10);
    }
}

void AddPackPopup::stepDifficulty(int delta) {
    int range = kMaxDifficulty - kMinDifficulty + 1;
    m_difficulty = kMinDifficulty + ((m_difficulty - kMinDifficulty + delta) % range + range) % range;

    setFrame(m_diffIcon, difficultyFrame(m_difficulty));
    if (m_diffLabel) m_diffLabel->setString(difficultyName(m_difficulty));
    this->updatePreview();
}

void AddPackPopup::setStar(bool star) {
    m_star = star;
    ccColor3B const on { 255, 255, 255 };
    ccColor3B const off { 80, 80, 80 };
    if (m_starSpr) m_starSpr->setColor(star ? on : off);
    if (m_moonSpr) m_moonSpr->setColor(star ? off : on);
    this->updatePreview();
}

void AddPackPopup::updatePreview() {
    if (!m_previewTitle) return;

    std::string title = m_titleInput ? m_titleInput->getString() : "";
    m_previewTitle->setString(title.empty() ? "Pack Title" : title.c_str());
    m_previewTitle->setOpacity(title.empty() ? 110 : 255);
    m_previewTitle->setColor(m_color);
    m_previewTitle->limitLabelWidth(240.f, 0.6f, 0.2f);

    setFrame(m_previewDiff, difficultyFrame(m_difficulty));
    setFrame(m_previewType, m_star ? "GJ_bigStar_noShadow_001.png" : "GJ_bigMoon_noShadow_001.png");
}

void AddPackPopup::onAdd() {
    if (m_adding) return;

    std::string title = m_titleInput->getString();
    if (title.empty()) {
        Notification::create("Give the pack a title", NotificationIcon::Error)->show();
        return;
    }

    int ids[3];
    for (int i = 0; i < 3; i++) {
        auto id = numFromString<int>(m_levelInputs[i]->getString());
        if (!id || id.unwrap() <= 0) {
            Notification::create(fmt::format("Enter a valid ID for level {}", i + 1), NotificationIcon::Error)->show();
            return;
        }
        ids[i] = id.unwrap();
    }

    m_adding = true;

    auto uPopup = UploadActionPopup::create(nullptr, "Adding pack...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<AddPackPopup> self = this;

    NewGrindPackBody body {
        title,
        m_star,
        ids[0], ids[1], ids[2],
        m_color.r, m_color.g, m_color.b,
        m_difficulty
    };

    m_listener.spawn(
        APIClient::getInstance().newGrindPack(body),
        [uPopupRef, self](web::WebResponse res) {
            if (!self) return;
            self->m_adding = false;

            bool ok = APIClient::getInstance().newGrindPackParse(res);
            if (!uPopupRef) return;

            if (ok) {
                uPopupRef->showSuccessMessage("Success! Pack added.");
            } else {
                uPopupRef->showFailMessage("Failed! Try again later.");
            }
        }
    );
}

}