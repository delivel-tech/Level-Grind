#include "GrindListsLayer.hpp"
#include "MainLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/ui/Label.hpp>
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <iterator>
#include <random>

#include "../../utils/globals.hpp"
#include "../../managers/APIClient.hpp"
#include "CustomBrowserLayer.hpp"

#include "../popups/DifficultySelectorPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

namespace {

struct KeyedInt {
    char const* key;
    int value;
};

KeyedInt const kGrindTypeKeys[] = {
    { "star-enabled", 0 }, { "moon-enabled", 0 }, { "coin-enabled", 0 }, { "demon-enabled", 0 },
};
char const* const kGrindTypeNames[] = { "star", "moon", "coin", "demon" };

KeyedInt const kLengthKeys[] = {
    { "short-enabled", int(Length::Short) },
    { "medium-enabled", int(Length::Medium) },
    { "long-enabled", int(Length::Long) },
    { "xl-enabled", int(Length::XL) },
};

KeyedInt const kVersionKeys[] = {
    { "version-22-enabled", 22 },
    { "version-21-enabled", 21 },
    { "version-20-enabled", 20 },
    { "version-19-enabled", 19 },
    { "version-18-enabled", 108 },
    { "version-17-enabled", 107 },
    { "version-16-enabled", 106 },
    { "version-15-enabled", 105 },
    { "version-14-enabled", 104 },
    { "version-13-enabled", 103 },
    { "version-12-enabled", 102 },
    { "version-11-enabled", 101 },
    { "version-10-enabled", 100 },
};
char const* const kVersionNames[] = { "2.2", "2.1", "2.0", "1.9", "1.8", "1.7", "1.6", "1.5", "1.4", "1.3", "1.2", "1.1", "1.0" };

constexpr char const* kLegacyOldVersions = "version-lower-than-19-enabled";

KeyedInt const kDemonKeys[] = {
    { "easy-demon-enabled", 3 },
    { "medium-demon-enabled", 4 },
    { "hard-demon-enabled", 0 },
    { "insane-demon-enabled", 5 },
    { "extreme-demon-enabled", 6 },
};

constexpr char const* kOnlyUncompleted = "only-uncompleted";
constexpr char const* kOnlyCompleted = "only-completed";
constexpr char const* kNewerFirst = "newer-first";
constexpr char const* kRecentlyAdded = "recently-added";

std::string coinKey(int coins) { return fmt::format("filter-coins-{}-enabled", coins); }
std::string ratingKey(int rating) { return fmt::format("filter-rating-{}-enabled", rating); }

int const kRatings[] = {
    LevelRating::Rate, LevelRating::Featured, LevelRating::Epic, LevelRating::Legendary, LevelRating::Mythic,
};

bool saved(std::string const& key) {
    return Mod::get()->getSavedValue<bool>(key);
}

void readSplitPair(char const* a, char const* b, bool& first, bool& second) {
    first = saved(a);
    second = saved(b);
    if (!first && !second) first = second = true;
}

void addDifficultyPair(std::vector<int>& out, char const* key, int first, int second, bool firstOn, bool secondOn) {
    if (!saved(key)) return;
    if (firstOn) out.push_back(first);
    if (secondOn) out.push_back(second);
}

ccColor3B const kDim = { 110, 110, 110 };

void tint(CCNode* node, ccColor3B color) {
    if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(node)) rgba->setColor(color);
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) tint(child, color);
}

void dim(CCNode* node) { tint(node, kDim); }

ButtonSprite* chip(char const* text, int width, bool on) {
    return ButtonSprite::create(text, width, true, "bigFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 30.f, 0.6f);
}

CCSprite* iconChip(char const* icon, bool on) {
    auto bg = CCSprite::create(on ? "GJ_button_02.png" : "GJ_button_04.png");
    auto top = CCSprite::createWithSpriteFrameName(icon);
    top->setPosition(bg->getContentSize() / 2);
    bg->addChild(top);
    return bg;
}

CCSprite* face(char const* frame, bool on) {
    auto spr = CCSprite::createWithSpriteFrameName(frame);
    if (!on) spr->setColor(kDim);
    return spr;
}

ButtonSprite* coinChip(int coins, bool on) {
    auto spr = chip(numToString(coins).c_str(), 46, on);
    auto w = spr->getContentWidth();
    auto h = spr->getContentHeight();
    if (spr->m_label) spr->m_label->setPositionX(w * 0.36f);
    auto coin = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
    coin->setScale(1.45f);
    coin->setPosition({ w * 0.68f, h / 2 });
    if (!on) coin->setColor(kDim);
    spr->addChild(coin, 2);
    return spr;
}

CCNode* ratingIcon(int rating, bool on) {
    auto node = CCNode::create();
    node->setContentSize({ 44.f, 44.f });
    node->setAnchorPoint({ 0.5f, 0.5f });

    char const* frame = nullptr;
    switch (rating) {
        case LevelRating::Featured: frame = "GJ_featuredCoin_001.png"; break;
        case LevelRating::Epic: frame = "GJ_epicCoin_001.png"; break;
        case LevelRating::Legendary: frame = "GJ_epicCoin2_001.png"; break;
        case LevelRating::Mythic: frame = "GJ_epicCoin3_001.png"; break;
        default: break;
    }

    if (frame) {
        auto coin = CCSprite::createWithSpriteFrameName(frame);
        coin->setPosition({ 22.f, 17.5f });
        coin->setScale(0.8f);
        node->addChild(coin);
        auto diff = CCSprite::createWithSpriteFrameName("diffIcon_04_btn_001.png");
        diff->setPosition({ 22.f, 22.f });
        diff->setScale(0.8f);
        node->addChild(diff, 1);
    } else {
        auto star = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        star->setPosition({ 22.f, 22.f });
        star->setScale(0.62f);
        node->addChild(star);
    }

    auto bg = CCSprite::create(on ? "GJ_button_02.png" : "GJ_button_04.png");
    node->setScale(0.82f);
    node->setPosition(bg->getContentSize() / 2);
    bg->addChild(node);
    if (!on) {
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
            if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(child)) rgba->setColor(ccColor3B{ 170, 170, 170 });
        }
    }
    return bg;
}

void setScaleMult(CCMenuItemToggler* toggler) {
    for (auto obj : CCArrayExt<CCNode*>(toggler->getChildren())) {
        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj)) btn->m_scaleMultiplier = 1.15f;
    }
}

}

GetLevelsBody MainLayer::savedSearchBody() {
    GetLevelsBody body;

    for (size_t i = 0; i < std::size(kGrindTypeKeys); ++i) {
        if (saved(kGrindTypeKeys[i].key)) body.grindTypes.push_back(kGrindTypeNames[i]);
    }

    if (saved("auto-enabled")) body.difficulties.push_back(1);
    if (saved("easy-enabled")) body.difficulties.push_back(2);
    if (saved("normal-enabled")) body.difficulties.push_back(3);

    bool a, b;
    readSplitPair("split-4-enabled", "split-5-enabled", a, b);
    addDifficultyPair(body.difficulties, "hard-enabled", 4, 5, a, b);
    readSplitPair("split-6-enabled", "split-7-enabled", a, b);
    addDifficultyPair(body.difficulties, "harder-enabled", 6, 7, a, b);
    readSplitPair("split-8-enabled", "split-9-enabled", a, b);
    addDifficultyPair(body.difficulties, "insane-enabled", 8, 9, a, b);

    for (auto const& k : kDemonKeys) if (saved(k.key)) body.demonDifficulties.push_back(k.value);
    for (auto const& k : kLengthKeys) if (saved(k.key)) body.lengths.push_back(k.value);
    for (auto const& k : kVersionKeys) if (saved(k.key)) body.versions.push_back(k.value);

    for (int c = 0; c <= 3; ++c) if (saved(coinKey(c))) body.coins.push_back(c);
    for (int r : kRatings) if (saved(ratingKey(r))) body.ratings.push_back(r);

    body.isNewerFirst = saved(kNewerFirst);
    body.isRecentlyAdded = saved(kRecentlyAdded);
    return body;
}

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

    readSplitPair("split-4-enabled", "split-5-enabled", m_splitHard4, m_splitHard5);
    readSplitPair("split-6-enabled", "split-7-enabled", m_splitHarder6, m_splitHarder7);
    readSplitPair("split-8-enabled", "split-9-enabled", m_splitInsane8, m_splitInsane9);

    if (saved(kLegacyOldVersions)) {
        for (size_t i = 4; i < std::size(kVersionKeys); ++i) Mod::get()->setSavedValue<bool>(kVersionKeys[i].key, true);
        Mod::get()->setSavedValue<bool>(kLegacyOldVersions, false);
    }

    if (!initSearchPanel()) return false;

    sendCountRequest();

    this->scheduleUpdate();
    return true;
}

CCMenu* MainLayer::addSection(CCNode* parent, char const* title, CCPoint center, CCSize size, float titleWidth, float menuScale, float gap) {
    auto section = Build<CCNode>::create()
        .contentSize(size)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos(center)
        .id(fmt::format("{}-section", string::toLower(title)))
        .parent(parent)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(size)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(70)
        .id("background")
        .parent(section);

    Build(geode::Label::create(title, "goldFont.fnt"))
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 8.f, size.height / 2 + 1.f })
        .with([&](geode::Label* label) { label->setLimitLabelWidth(titleWidth - 12.f, 0.5f, 0.1f); })
        .id("title")
        .parent(section);

    float const menuWidth = size.width - titleWidth - 6.f;
    return Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.5f, 0.5f })
        .contentSize({ menuWidth / menuScale, size.height / menuScale })
        .scale(menuScale)
        .pos({ titleWidth + menuWidth / 2, size.height / 2 })
        .layout(RowLayout::create()->setGap(gap))
        .id("menu")
        .parent(section)
        .collect();
}

CCMenuItemToggler* MainLayer::addToggle(CCMenu* menu, CCNode* off, CCNode* on, std::string const& key, char const* id) {
    auto toggler = CCMenuItemToggler::create(off, on, this, menu_selector(MainLayer::onKeyToggle));
    toggler->setID(id);
    toggler->toggle(saved(key));
    setScaleMult(toggler);
    menu->addChild(toggler);
    m_toggleKeys[toggler] = key;
    m_togglersByKey[key] = toggler;
    return toggler;
}

bool MainLayer::initSearchPanel() {
    auto winSize = m_winSize;

    auto logo = Build<CCSprite>::create("lg-logo.png"_spr)
        .scale(0.72f)
        .pos({ winSize.width / 2, winSize.height - 27.f })
        .id("level-grind-logo")
        .parent(this)
        .collect();

    m_countMaxWidth = winSize.width / 2 - logo->getScaledContentSize().width / 2 - 14.f;
    m_countLabel = Build(geode::Label::create("", "goldFont.fnt"))
        .anchorPoint({ 1.f, 1.f })
        .pos({ winSize.width - 7.f, winSize.height - 6.f })
        .id("levels-found-label")
        .parent(this)
        .collect();
    setCountText("Loading...");

    m_starsRow = Build<CCNode>::create()
        .pos({ winSize.width - 7.f, winSize.height - 26.f })
        .id("stars-left-row")
        .parent(this)
        .collect();

    CCSize const panelSize = { 480.f, 174.f };
    float const leftWidth = 282.f;
    float const rightWidth = panelSize.width - leftWidth - 8.f;
    float const rightX = leftWidth + 8.f + rightWidth / 2;

    float const actionScale = 0.8f;
    float const actionY = 26.f;
    float const actionTop = actionY + 48.f / 2 * actionScale;
    float const logoBottom = logo->getPositionY() - logo->getScaledContentSize().height / 2;
    CCSize const panelOuter = { panelSize.width + 20.f, panelSize.height + 20.f };
    float const panelScale = std::min({ 1.08f, (winSize.width - 24.f) / panelOuter.width, (logoBottom - actionTop - 12.f) / panelOuter.height });
    float const panelY = (logoBottom + actionTop) / 2;

    auto panel =Build<CCNode>::create()
        .contentSize(panelSize)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos({ winSize.width / 2, panelY })
        .scale(panelScale)
        .id("search-panel")
        .parent(this)
        .collect();
    m_searchPanel = panel;

    Build(NineSlice::create("GJ_square02.png"))
        .contentSize({ panelSize.width + 20.f, panelSize.height + 20.f })
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ panelSize.width / 2, panelSize.height / 2 })
        .zOrder(-1)
        .id("panel-background")
        .parent(panel);

    float const leftH = 30.f;
    float const leftGap = (panelSize.height - leftH * 5) / 4;
    auto leftY = [&](int row) { return panelSize.height - leftH / 2 - row * (leftH + leftGap); };
    float const leftTitle = 70.f;

    auto typeMenu = addSection(panel, "Type", { leftWidth / 2, leftY(0) }, { leftWidth, leftH }, leftTitle, 0.6f, 10.f);
    char const* typeIcons[] = { "GJ_starsIcon_001.png", "GJ_moonsIcon_001.png", "GJ_coinsIcon2_001.png", "GJ_demonIcon_001.png" };
    char const* typeIds[] = { "star-toggler", "moon-toggler", "coin-toggler", "demon-toggler" };
    for (size_t i = 0; i < 4; ++i) {
        addToggle(typeMenu, iconChip(typeIcons[i], false), iconChip(typeIcons[i], true), kGrindTypeKeys[i].key, typeIds[i]);
    }
    typeMenu->updateLayout();

    auto diffMenu = addSection(panel, "Difficulty", { leftWidth / 2, leftY(1) }, { leftWidth, leftH }, leftTitle, 0.68f, 8.f);
    struct { char const* frame; char const* key; char const* id; } const diffs[] = {
        { "diffIcon_auto_btn_001.png", "auto-enabled", "auto-difficulty-toggler" },
        { "diffIcon_01_btn_001.png", "easy-enabled", "easy-difficulty-toggler" },
        { "diffIcon_02_btn_001.png", "normal-enabled", "normal-difficulty-toggler" },
        { "diffIcon_03_btn_001.png", "hard-enabled", "hard-difficulty-toggler" },
        { "diffIcon_04_btn_001.png", "harder-enabled", "harder-difficulty-toggler" },
        { "diffIcon_05_btn_001.png", "insane-enabled", "insane-difficulty-toggler" },
    };
    for (auto const& d : diffs) addToggle(diffMenu, face(d.frame, false), face(d.frame, true), d.key, d.id);
    diffMenu->updateLayout();

    auto diffSection = diffMenu->getParent();
    auto selectorMenu = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize({ 20.f, 20.f })
        .pos({ leftWidth - 13.f, leftH / 2 })
        .id("difficulty-selector-menu")
        .parent(diffSection)
        .collect();
    Build(CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png"))
        .scale(0.55f)
        .intoMenuItem([this] { this->onOpenDifficultySelector(nullptr); })
        .scaleMult(1.2f)
        .pos({ 10.f, 10.f })
        .id("difficulty-selector-btn")
        .parent(selectorMenu);
    m_difficultySelector = selectorMenu;

    m_demonsMenu = addSection(panel, "Demons", { leftWidth / 2, leftY(2) }, { leftWidth, leftH }, leftTitle, 0.6f, 12.f);
    char const* demonFrames[] = {
        "diffIcon_07_btn_001.png", "diffIcon_08_btn_001.png", "diffIcon_06_btn_001.png", "diffIcon_09_btn_001.png", "diffIcon_10_btn_001.png",
    };
    char const* demonIds[] = { "easy-demon-toggler", "medium-demon-toggler", "hard-demon-toggler", "insane-demon-toggler", "extreme-demon-toggler" };
    for (size_t i = 0; i < 5; ++i) {
        addToggle(m_demonsMenu, face(demonFrames[i], false), face(demonFrames[i], true), kDemonKeys[i].key, demonIds[i]);
    }
    m_demonsMenu->updateLayout();

    m_demonsHint = Build<CCNode>::create()
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .contentSize({ 120.f, 20.f })
        .pos(m_demonsMenu->getPosition())
        .layout(RowLayout::create()->setGap(4.f)->setAutoScale(false))
        .id("demons-hint")
        .parent(m_demonsMenu->getParent())
        .collect();
    Build(CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png"))
        .scale(0.6f)
        .id("demons-hint-icon")
        .parent(m_demonsHint);
    Build(geode::Label::create("only", "chatFont.fnt"))
        .color({ 200, 200, 200 })
        .scale(0.7f)
        .id("demons-hint-label")
        .parent(m_demonsHint);
    m_demonsHint->updateLayout();

    auto lengthMenu = addSection(panel, "Length", { leftWidth / 2, leftY(3) }, { leftWidth, leftH }, leftTitle, 0.55f, 6.f);
    char const* lengthNames[] = { "Short", "Medium", "Long", "XL" };
    char const* lengthIds[] = { "short-length", "medium-length", "long-length", "xl-length" };
    for (size_t i = 0; i < 4; ++i) {
        addToggle(lengthMenu, chip(lengthNames[i], 70, false), chip(lengthNames[i], 70, true), kLengthKeys[i].key, lengthIds[i]);
    }
    lengthMenu->updateLayout();

    auto versionMenu = addSection(panel, "Version", { panelSize.width / 2, leftY(4) }, { panelSize.width, leftH }, leftTitle, 0.55f, 3.f);
    for (size_t i = 0; i < std::size(kVersionKeys); ++i) {
        auto id = fmt::format("version-{}", kVersionNames[i]);
        addToggle(versionMenu, chip(kVersionNames[i], 50, false), chip(kVersionNames[i], 50, true), kVersionKeys[i].key, id.c_str());
    }
    versionMenu->updateLayout();

    float const rightH = leftH;
    auto rightY = leftY;
    float const rightTitle = 56.f;

    auto coinsMenu = addSection(panel, "Coins", { rightX, rightY(0) }, { rightWidth, rightH }, rightTitle, 0.55f, 5.f);
    for (int c = 0; c <= 3; ++c) {
        addToggle(coinsMenu, coinChip(c, false), coinChip(c, true), coinKey(c), fmt::format("coins-{}-toggler", c).c_str());
    }
    coinsMenu->updateLayout();

    auto ratingMenu = addSection(panel, "Rating", { rightX, rightY(1) }, { rightWidth, rightH }, rightTitle, 0.56f, 4.f);
    for (int r : kRatings) {
        addToggle(ratingMenu, ratingIcon(r, false), ratingIcon(r, true), ratingKey(r), fmt::format("rating-{}-toggler", r).c_str());
    }
    ratingMenu->updateLayout();

    auto showMenu = addSection(panel, "Show", { rightX, rightY(2) }, { rightWidth, rightH }, rightTitle, 0.5f, 6.f);
    addToggle(showMenu, chip("Uncompleted", 118, false), chip("Uncompleted", 118, true), kOnlyUncompleted, "only-uncompleted-toggler");
    addToggle(showMenu, chip("Completed", 104, false), chip("Completed", 104, true), kOnlyCompleted, "only-completed-toggler");
    showMenu->updateLayout();

    auto sortMenu = addSection(panel, "Sort", { rightX, rightY(3) }, { rightWidth, rightH }, rightTitle, 0.5f, 6.f);
    addToggle(sortMenu, chip("Newer First", 112, false), chip("Newer First", 112, true), kNewerFirst, "newer-first-toggler");
    addToggle(sortMenu, chip("Recently Added", 126, false), chip("Recently Added", 126, true), kRecentlyAdded, "recently-added-toggler");
    sortMenu->updateLayout();

    Build(NineSlice::create("GJ_square02.png"))
        .contentSize({ 330.f, 48.f })
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ winSize.width / 2, actionY })
        .scale(actionScale)
        .id("search-menu-background")
        .parent(this);

    auto actionMenu = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize({ 310.f, 40.f })
        .pos({ winSize.width / 2, actionY })
        .scale(actionScale)
        .layout(RowLayout::create()->setGap(14.f))
        .id("search-menu")
        .parent(this)
        .collect();

    m_resetBtn = Build(ButtonSprite::create("Reset", 110, true, "goldFont.fnt", "GJ_button_06.png", 32.f, 0.85f))
        .intoMenuItem([this] { this->onReset(nullptr); })
        .scaleMult(1.1f)
        .id("reset-btn")
        .parent(actionMenu)
        .collect();

    Build(ButtonSprite::create("Search", 110, true, "goldFont.fnt", "GJ_button_01.png", 32.f, 0.85f))
        .intoMenuItem([this] { this->onSearch(nullptr); })
        .scaleMult(1.1f)
        .id("search-btn")
        .parent(actionMenu);

    m_randomBtn = Build<CCSprite>::create("random_btn.png"_spr)
        .scale(0.58f)
        .intoMenuItem([this] { this->onRandom(nullptr); })
        .scaleMult(1.1f)
        .id("random-btn")
        .parent(actionMenu)
        .collect();

    actionMenu->updateLayout();

    updateDemonSection();
    updateDifficultySelectorVisibility();
    initListsMode();
    updateResetButton();
    return true;
}

void MainLayer::setKey(std::string const& key, bool value) {
    Mod::get()->setSavedValue<bool>(key, value);
    auto it = m_togglersByKey.find(key);
    if (it != m_togglersByKey.end() && it->second->isToggled() != value) it->second->toggle(value);
}

void MainLayer::onKeyToggle(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return;
    auto it = m_toggleKeys.find(toggler);
    if (it == m_toggleKeys.end()) return;

    std::string const key = it->second;
    bool const on = !toggler->isToggled();
    Mod::get()->setSavedValue<bool>(key, on);

    if (on) {
        if (key == kOnlyUncompleted) setKey(kOnlyCompleted, false);
        if (key == kOnlyCompleted) setKey(kOnlyUncompleted, false);
        if (key == kNewerFirst) setKey(kRecentlyAdded, false);
        if (key == kRecentlyAdded) setKey(kNewerFirst, false);
    }

    updateDemonSection();
    updateDifficultySelectorVisibility();
    updateResetButton();
    scheduleCountRefresh();
}

void MainLayer::updateDemonSection() {
    bool const demonOn = saved("demon-enabled");
    if (m_demonsMenu) m_demonsMenu->setVisible(demonOn);
    if (m_demonsHint) m_demonsHint->setVisible(!demonOn);
}

void MainLayer::updateDifficultySelectorVisibility() {
    if (!m_difficultySelector) return;
    m_difficultySelector->setVisible(saved("hard-enabled") || saved("harder-enabled") || saved("insane-enabled"));
}

void MainLayer::updateResetButton() {
    if (!m_resetBtn) return;
    bool any = false;
    if (lists::listsMode()) any = !listFiltersDefault();
    else for (auto const& [toggler, key] : m_toggleKeys) {
        if (saved(key)) { any = true; break; }
    }
    m_resetBtn->setEnabled(any);

    geode::Function<void(CCNode*, ccColor3B)> flat = [&](CCNode* node, ccColor3B color) {
        if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(node)) {
            rgba->setCascadeColorEnabled(false);
            rgba->setColor(color);
        }
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) flat(child, color);
    };
    if (auto spr = m_resetBtn->getNormalImage()) flat(spr, any ? ccColor3B{ 255, 255, 255 } : kDim);
}

void MainLayer::onReset(CCObject*) {
    if (lists::listsMode()) {
        m_listFilters = lists::Filters();
        m_listFilters.save();
        buildListsPanel();
        updateResetButton();
        return;
    }
    for (auto const& [toggler, key] : m_toggleKeys) {
        Mod::get()->setSavedValue<bool>(key, false);
        toggler->toggle(false);
    }
    updateDemonSection();
    updateDifficultySelectorVisibility();
    updateResetButton();
    scheduleCountRefresh();
}

void MainLayer::onOpenDifficultySelector(CCObject*) {
    DifficultySelectorPopup::create(this)->show();
}

void MainLayer::onSearch(CCObject*) {
    if (lists::listsMode()) {
        GrindListsLayer::create()->open();
        return;
    }
    CustomBrowserLayer::create(savedSearchBody(), "Grinding Levels", CustomBrowserType::Search, EventType::Daily)->open(); // eventtype is just a placeholder
}

void MainLayer::onRandom(CCObject*) {
    if (lists::listsMode()) return;
    if (m_randomPending) return;

    auto body = savedSearchBody();
    body.isNewerFirst = true;
    body.isRecentlyAdded = false;

    auto ll = levelgrind::LoadingLayer::create("Loading random level...");
    ll->show();

    m_randomLoadingLayer = Ref(ll);

    auto loadingRef = Ref(ll);
    auto self = Ref(this);

    auto resetState = [loadingRef, self]() {
        self->m_randomPending = false;
        self->m_randomTimer = 0.f;
        self->m_randomLevelID = -1;
        self->m_randomKey.clear();
        if (loadingRef) loadingRef->hide();
        self->m_randomLoadingLayer = nullptr;
    };

    m_listener.spawn(
        APIClient::getInstance().getLevels(body),
        [self, loadingRef, resetState](web::WebResponse res) {
            if (!self) {
                resetState();
                return;
            }

            auto parsed = APIClient::getInstance().getLevelsParse(res);

            if (!parsed.ok) {
                resetState();
                Notification::create("Failed to get random level", NotificationIcon::Error)->show();
                return;
            }

            if (parsed.ids.empty()) {
                Notification::create("No levels found", NotificationIcon::Info)->show();
                resetState();
                return;
            }

            std::shuffle(parsed.ids.begin(), parsed.ids.end(), std::mt19937{ std::random_device{}() });

            bool const onlyUncompleted = saved(kOnlyUncompleted);
            bool const onlyCompleted = saved(kOnlyCompleted);

            auto gsm = GameStatsManager::sharedState();

            int chosenID = -1;
            for (auto id : parsed.ids) {
                if (gsm) {
                    bool isCompleted = gsm->hasCompletedOnlineLevel(id);
                    if (onlyUncompleted && isCompleted) continue;
                    if (onlyCompleted && !isCompleted) continue;
                }
                chosenID = id;
                break;
            }

            if (chosenID <= 0) {
                if (onlyUncompleted)
                    Notification::create("No uncompleted levels found", NotificationIcon::Info)->show();
                else if (onlyCompleted)
                    Notification::create("No completed levels found", NotificationIcon::Info)->show();
                else
                    Notification::create("No levels found", NotificationIcon::Info)->show();
                resetState();
                return;
            }

            auto searchObj = GJSearchObject::create(SearchType::Search, numToString(chosenID));
            auto key = searchObj->getKey();
            auto glm = GameLevelManager::sharedState();

            if (!glm) {
                Notification::create("Failed to access level manager.", NotificationIcon::Error)->show();
                resetState();
                return;
            }

            auto stored = glm->getStoredOnlineLevels(key);
            if (stored && stored->count() > 0) {
                auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
                if (level && level->m_levelID == chosenID) {
                    auto scene = LevelInfoLayer::scene(level, false);
                    auto trans = CCTransitionFade::create(0.5f, scene);
                    resetState();
                    CCDirector::sharedDirector()->pushScene(trans);
                    return;
                }
            }

            self->m_randomTimer = 10.f;
            self->m_randomLevelID = chosenID;
            self->m_randomKey = key;
            self->m_randomPending = true;

            if (glm->m_levelManagerDelegate) {
                glm->m_levelManagerDelegate = nullptr;
            }
            glm->getOnlineLevels(searchObj);
        }
    );
}

void MainLayer::update(float dt) {
    if (m_countDelay >= 0.f) {
        m_countDelay -= dt;
        if (m_countDelay < 0.f) sendCountRequest();
    }

    if (!m_randomPending) return;

    if (m_randomTimer > 0.f && m_randomLevelID > 0) {
        m_randomTimer -= dt;
        if (auto glm = GameLevelManager::sharedState()) {
            auto stored = glm->getStoredOnlineLevels(m_randomKey.c_str());
            if (stored && stored->count() > 0) {
                for (size_t i = 0; i < stored->count(); ++i) {
                    auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
                    if (lvl && lvl->m_levelID == m_randomLevelID) {
                        auto scene = LevelInfoLayer::scene(lvl, false);
                        auto trans = CCTransitionFade::create(0.5f, scene);
                        CCDirector::sharedDirector()->pushScene(trans);
                        m_randomPending = false;
                        m_randomKey.clear();
                        m_randomLevelID = -1;
                        m_randomTimer = 0.f;
                        if (m_randomLoadingLayer) {
                            m_randomLoadingLayer->hide();
                            m_randomLoadingLayer = nullptr;
                        }
                        return;
                    }
                }
            }
        }
    }

    if (m_randomTimer <= 0.f) {
        m_randomPending = false;
        m_randomKey.clear();
        m_randomLevelID = -1;
        if (m_randomLoadingLayer) {
            m_randomLoadingLayer->hide();
            m_randomLoadingLayer = nullptr;
        }
        Notification::create("Failed to fetch random level", NotificationIcon::Error)->show();
    }
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
        if (!first && !second) first = second = true;
    };

    auto mod = Mod::get();
    if (difficulty == 4 || difficulty == 5) {
        normalizePair(m_splitHard4, m_splitHard5);
        mod->setSavedValue<bool>("split-4-enabled", m_splitHard4);
        mod->setSavedValue<bool>("split-5-enabled", m_splitHard5);
    } else if (difficulty == 6 || difficulty == 7) {
        normalizePair(m_splitHarder6, m_splitHarder7);
        mod->setSavedValue<bool>("split-6-enabled", m_splitHarder6);
        mod->setSavedValue<bool>("split-7-enabled", m_splitHarder7);
    } else {
        normalizePair(m_splitInsane8, m_splitInsane9);
        mod->setSavedValue<bool>("split-8-enabled", m_splitInsane8);
        mod->setSavedValue<bool>("split-9-enabled", m_splitInsane9);
    }
}

void MainLayer::refreshSplitDifficultyFilters() {
    updateDifficultySelectorVisibility();
    scheduleCountRefresh();
}

void MainLayer::scheduleCountRefresh() {
    m_countDelay = 0.4f;
    setCountText("Loading...");
}

void MainLayer::setCountText(std::string const& value) {
    if (!m_countLabel) return;
    m_countLabel->setString(fmt::format("Levels found: {}", value).c_str());
    m_countLabel->setLimitLabelWidth(m_countMaxWidth, 0.5f, 0.2f);
}

static std::string withCommas(long long n) {
    auto digits = numToString(n);
    for (int i = int(digits.size()) - 3; i > 0; i -= 3) digits.insert(size_t(i), ",");
    return digits;
}

void MainLayer::sendCountRequest() {
    m_countDelay = -1.f;
    if (lists::listsMode()) return;
    int const seq = ++m_countSeq;

    auto body = savedSearchBody();
    auto key = APIClient::getInstance().getLevelsJson(body).dump(matjson::NO_INDENTATION);

    if (auto it = m_countCache.find(key); it != m_countCache.end()) {
        this->showCountResult(it->second);
        return;
    }

    setCountText("Loading...");
    if (m_starsRow) m_starsRow->setVisible(false);

    auto self = Ref(this);
    m_countListener.spawn(
        APIClient::getInstance().getLevels(body),
        [self, seq, key](web::WebResponse res) {
            if (!self || seq != self->m_countSeq) return;

            auto parsed = APIClient::getInstance().getLevelsParse(res);
            if (!parsed.ok) {
                self->setCountText("?");
                return;
            }
            if (self->m_countCache.size() > 32) self->m_countCache.clear();
            self->m_countCache[key] = parsed;
            self->showCountResult(parsed);
        }
    );
}

void MainLayer::showCountResult(GetLevelsResponse const& parsed) {
    int count = parsed.count;
    bool const onlyUncompleted = saved(kOnlyUncompleted);
    bool const onlyCompleted = saved(kOnlyCompleted);
    auto gsm = GameStatsManager::sharedState();

    long long starsTotal = 0, starsLeft = 0, moonsTotal = 0, moonsLeft = 0;
    bool const hasStars = parsed.stars.size() == parsed.ids.size();

    if (gsm && (onlyUncompleted || onlyCompleted || hasStars)) {
        if (onlyUncompleted || onlyCompleted) count = 0;
        for (size_t i = 0; i < parsed.ids.size(); i++) {
            bool const done = gsm->hasCompletedOnlineLevel(parsed.ids[i]);
            if ((onlyUncompleted || onlyCompleted) && (onlyUncompleted ? !done : done)) ++count;
            if (!hasStars) continue;
            if (onlyUncompleted && done) continue;
            if (onlyCompleted && !done) continue;
            long long const reward = parsed.stars[i];
            if (parsed.moon[i]) {
                moonsTotal += reward;
                if (!done) moonsLeft += reward;
            } else {
                starsTotal += reward;
                if (!done) starsLeft += reward;
            }
        }
    }
    setCountText(withCommas(count));

    if (!m_starsRow) return;
    m_starsRow->removeAllChildren();
    m_starsRow->setVisible(hasStars && (starsTotal > 0 || moonsTotal > 0));

    float y = 0.f;
    auto addLine = [&](const char* icon, long long left, long long total) {
        if (total <= 0) return;
        auto line = CCNode::create();
        auto lbl = geode::Label::create(fmt::format("{} left / {}", withCommas(left), withCommas(total)).c_str(), "goldFont.fnt");
        lbl->setScale(0.4f);
        lbl->setAnchorPoint({ 1.f, 0.5f });
        line->addChild(lbl);
        auto spr = CCSprite::createWithSpriteFrameName(icon);
        spr->setScale(0.45f);
        spr->setPosition({ -lbl->getScaledContentWidth() - 5.f - spr->getScaledContentWidth() / 2.f, 0.f });
        line->addChild(spr);
        float w = lbl->getScaledContentWidth() + 5.f + spr->getScaledContentWidth();
        if (w > m_countMaxWidth) line->setScale(m_countMaxWidth / w);
        line->setPosition({ 0.f, y });
        m_starsRow->addChild(line);
        y -= 14.f;
    };
    addLine("GJ_starsIcon_001.png", starsLeft, starsTotal);
    addLine("GJ_moonsIcon_001.png", moonsLeft, moonsTotal);
}

}
