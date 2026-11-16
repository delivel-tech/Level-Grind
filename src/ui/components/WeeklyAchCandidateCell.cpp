#include "WeeklyAchCandidateCell.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/TextInput.hpp"

#include <Geode/Enums.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>

#include "../../managers/WeeklyAchievementsManager.hpp"

namespace levelgrind {

namespace {

struct PlaceStyle {
    const char* name;
    const char* rankIcon;
    ccColor3B color;
};

PlaceStyle placeStyle(AchievementCellType placement) {
    switch (placement) {
        case AchievementCellType::First: return { "1st Place", "rankIcon_1_001.png", { 255, 210, 70 } };
        case AchievementCellType::Second: return { "2nd Place", "rankIcon_top50_001.png", { 200, 215, 235 } };
        case AchievementCellType::Third: return { "3rd Place", "rankIcon_top100_001.png", { 225, 145, 85 } };
    }
    return { "?", "rankIcon_top100_001.png", { 255, 255, 255 } };
}

constexpr const char* kTypeIcons[] = {
    "GJ_starsIcon_001.png",
    "GJ_moonsIcon_001.png",
    "GJ_coinsIcon2_001.png",
    "GJ_demonIcon_001.png",
};

CCLabelBMFont* fieldLabel(CCNode* parent, const char* text, float y) {
    return Build(CCLabelBMFont::create(text, "goldFont.fnt"))
        .scale(0.42f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 10.f, y })
        .parent(parent)
        .collect();
}

}

WeeklyAchCandidateCell* WeeklyAchCandidateCell::create(AchievementCellType placement, GJUserScore* userScore) {
    auto ret = new WeeklyAchCandidateCell;
    if (ret && ret->init(placement, userScore)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchCandidateCell::init(AchievementCellType placement, GJUserScore* userScore) {
    if (!CCNode::init()) return false;

    m_placement = placement;
    m_userScore = userScore;

    this->setContentSize(kSize);
    this->setAnchorPoint({ 0.5f, 0.5f });
    this->ignoreAnchorPointForPosition(false);

    auto style = placeStyle(placement);

    Build(NineSlice::create("square02_small.png"))
        .contentSize(kSize)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(80)
        .parent(this);

    Build(CCLayerColor::create({ style.color.r, style.color.g, style.color.b, 200 }, kSize.width - 16.f, 2.f))
        .pos({ 8.f, kSize.height - 5.f })
        .parent(this);

    this->buildHeader();
    this->rebuild();

    return true;
}

void WeeklyAchCandidateCell::buildHeader() {
    auto style = placeStyle(m_placement);
    float const y = kSize.height - 20.f;

    if (auto rank = CCSprite::createWithSpriteFrameName(style.rankIcon)) {
        rank->setScale(18.f / rank->getContentHeight());
        rank->setPosition({ 20.f, y });
        this->addChild(rank);
    }

    Build(CCLabelBMFont::create(style.name, "goldFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 34.f, y + 1.f })
        .color(style.color)
        .parent(this);

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, kSize.width - 16.f, 1.f))
        .pos({ 8.f, kSize.height - 34.f })
        .parent(this);
}

void WeeklyAchCandidateCell::rebuild() {
    if (m_content) m_content->removeFromParent();
    m_titleInput = nullptr;
    m_infoInput = nullptr;
    m_typeSprites.clear();

    m_content = Build<CCNode>::create()
        .contentSize(kSize)
        .pos({ 0.f, 0.f })
        .parent(this)
        .collect();

    auto saved = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(m_placement);
    m_filled = saved.first;
    m_info = saved.second;
    m_type = std::clamp(m_info.type, 0, static_cast<int>(std::size(kTypeIcons)) - 1);

    if (m_filled) this->buildFilled();
    else this->buildEmpty();
}

void WeeklyAchCandidateCell::buildEmpty() {
    auto menu = Build<CCMenu>::create()
        .contentSize(kSize)
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(m_content)
        .collect();

    float const midY = (kSize.height - 34.f) / 2.f + 6.f;

    Build<CCSprite>::createSpriteName("GJ_plusBtn_001.png")
        .scale(0.9f)
        .intoMenuItem([this] {
            if (!m_userScore) return;
            WeeklyAchievementsManager::getInstance().saveConfigData(
                true,
                m_placement,
                AchievementCellInfo {
                    m_userScore->m_userName,
                    m_userScore->m_accountID,
                    m_userScore->m_playerCube,
                    m_userScore->m_color1,
                    m_userScore->m_color2,
                    m_userScore->m_glowEnabled ? m_userScore->m_color3 : 0,
                    "", 0, ""
                }
            );
            this->rebuild();
        })
        .pos({ kSize.width / 2.f, midY + 10.f })
        .id("add-btn")
        .parent(menu);

    Build(CCLabelBMFont::create("Empty spot", "goldFont.fnt"))
        .scale(0.5f)
        .pos({ kSize.width / 2.f, midY - 22.f })
        .opacity(200)
        .parent(m_content);

    std::string hint = m_userScore ? fmt::format("Tap + to add {}", std::string(m_userScore->m_userName)) : "Open from a profile";
    Build(CCLabelBMFont::create(hint.c_str(), "chatFont.fnt"))
        .scale(0.5f)
        .pos({ kSize.width / 2.f, midY - 38.f })
        .opacity(160)
        .limitLabelWidth(kSize.width - 20.f, 0.5f, 0.2f)
        .parent(m_content);
}

void WeeklyAchCandidateCell::buildFilled() {
    auto menu = Build<CCMenu>::create()
        .contentSize(kSize)
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(m_content)
        .collect();

    float const w = kSize.width;

    float const playerY = kSize.height - 52.f;

    auto icon = Build(cue::PlayerIcon::create(cue::Icons {
        IconType::Cube, m_info.icon, m_info.colorFirst, m_info.colorSecond, m_info.glow
    }))
        .scale(0.6f)
        .pos({ 24.f, playerY })
        .parent(m_content)
        .collect();
    if (m_info.glow == 0) icon->disableGlowOutline();

    Build(CCLabelBMFont::create(m_info.username.c_str(), "goldFont.fnt"))
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 42.f, playerY + 1.f })
        .limitLabelWidth(w - 52.f, 0.55f, 0.2f)
        .parent(m_content);

    fieldLabel(m_content, "Title", 124.f);
    m_titleInput = Build(TextInput::create(170.f, "e.g. 200K", "chatFont.fnt"))
        .scale(0.75f)
        .pos({ w / 2.f, 106.f })
        .parent(m_content)
        .collect();
    m_titleInput->setMaxCharCount(255);
    m_titleInput->setCommonFilter(CommonFilter::Any);
    m_titleInput->setString(m_info.title);
    m_titleInput->setCallback([this](auto const&) { this->saveFromUI(); });

    float const typeY = 82.f;
    fieldLabel(m_content, "Type", typeY);
    for (int i = 0; i < static_cast<int>(std::size(kTypeIcons)); i++) {
        auto spr = CCSprite::createWithSpriteFrameName(kTypeIcons[i]);
        spr->setScale(0.75f);
        m_typeSprites.push_back(spr);
        Build(spr)
            .intoMenuItem([this, i] { this->selectType(i); })
            .pos({ 62.f + 22.f * i, typeY })
            .parent(menu);
    }

    fieldLabel(m_content, "Info", 62.f);
    m_infoInput = Build(TextInput::create(170.f, "Description", "chatFont.fnt"))
        .scale(0.75f)
        .pos({ w / 2.f, 44.f })
        .parent(m_content)
        .collect();
    m_infoInput->setMaxCharCount(255);
    m_infoInput->setCommonFilter(CommonFilter::Any);
    m_infoInput->setString(m_info.info);
    m_infoInput->setCallback([this](auto const&) { this->saveFromUI(); });

    Build(ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_06.png", 0.8f))
        .scale(0.55f)
        .intoMenuItem([this] {
            WeeklyAchievementsManager::getInstance().wipeConfigData(m_placement);
            this->rebuild();
        })
        .pos({ w / 2.f, 15.f })
        .id("clear-btn")
        .parent(menu);

    this->selectType(m_type);
}

void WeeklyAchCandidateCell::selectType(int type) {
    m_type = type;
    for (int i = 0; i < static_cast<int>(m_typeSprites.size()); i++) {
        bool on = i == type;
        m_typeSprites[i]->setColor(on ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 90, 90, 90 });
        m_typeSprites[i]->setOpacity(on ? 255 : 170);
    }
    this->saveFromUI();
}

void WeeklyAchCandidateCell::saveFromUI() {
    if (!m_filled || !m_titleInput || !m_infoInput) return;

    m_info.title = m_titleInput->getString();
    m_info.info = m_infoInput->getString();
    m_info.type = m_type;

    WeeklyAchievementsManager::getInstance().saveConfigData(true, m_placement, m_info);
}

}