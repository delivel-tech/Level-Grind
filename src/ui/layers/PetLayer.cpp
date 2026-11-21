#include "PetLayer.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <UIBuilder.hpp>

#include "../../managers/APIClient.hpp"
#include "../popups/PetRenamePopup.hpp"
#include "../popups/PetUpgradePopup.hpp"
#include "../popups/PetCustomizePopup.hpp"
#include "Geode/ui/ProgressBar.hpp"
#include "Geode/utils/web.hpp"

#include "../popups/GuidePopup.hpp"
#include "../popups/ReportPopups.hpp"
#include "../PetAnim.hpp"
#include <fmt/format.h>
#include "../ClanUI.hpp"
#include "../../managers/Auth.hpp"
#include "../../managers/NotifyManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace levelgrind {

PetLayer* PetLayer::create() {
    auto ret = new PetLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void PetLayer::changeMusic(MusicMode mode) {
    if (mode == MusicMode::In) {
        auto fmod = FMODAudioEngine::sharedEngine();
        fmod->pauseMusic(0);
        fmod->playMusic("secretShop.mp3", true, 1.f, 0);
    }
    if (mode == MusicMode::Out) {
        auto gm = GameManager::sharedState();
        gm->fadeInMenuMusic();
    }
}

bool PetLayer::init() {
    if (!BaseLayer::init()) return false;

    changeMusic(MusicMode::In);

    PetManager::getInstance().m_isLevelUpgraded = false;
    PetManager::getInstance().m_isRarityUpgraded = false;

    addSideArt(
        this, SideArt::Bottom, SideArtStyle::LayerGray, false
    );

    CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    m_backBtn->removeFromParent();
    m_backBtn = nullptr;

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this,
        menu_selector(PetLayer::onBack));
    backBtn->setPosition(25, winSize.height - 25);

    auto backBtnMenu = CCMenu::create();
    backBtnMenu->setPosition(0.f, 0.f);
    backBtnMenu->setZOrder(1);
    this->addChild(backBtnMenu);
    backBtnMenu->addChild(backBtn);

    auto infoBtnMenu = Build<CCMenu>::create()
        .id("info-btn-menu")
        .pos(
            winSize.width - 25.f, 25.f
        )
        .parent(this)
        .collect();

    auto infoBtn = Build(CCSprite::create("info_btn.png"_spr))
        .scale(0.7f)
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::PetGuide, GuidePopupState::FromOutside)->show();
        })
        .id("info-btn")
        .parent(infoBtnMenu)
        .collect();

    auto mainPanel = Build(NineSlice::create("GJ_square02.png"))
        .contentSize({460.f, 232.f})
        .parent(this)
        .center()
        .id("main-panel")
        .collect();

    m_mainPanel = mainPanel;
    CCSize mainPanelCS = mainPanel->getContentSize();

    m_content = CCNode::create();
    m_content->setID("content");
    m_content->setAnchorPoint({ 0.f, 0.f });
    m_content->setContentSize(mainPanelCS);
    m_content->setPosition({ 0.f, 0.f });
    mainPanel->addChild(m_content);

    loadData(true);

    return true;
}

void PetLayer::loadData(bool firstLoad) {
    if (!m_mainPanel || !m_content) return;

    if (auto old = m_mainPanel->getChildByID("loading-spinner")) {
        old->removeFromParent();
    }

    m_content->setVisible(false);

    auto loading = LoadingSpinner::create(50.f);
    loading->setID("loading-spinner");
    loading->setPosition(m_mainPanel->getContentSize() / 2.f);
    m_mainPanel->addChild(loading);

    WeakRef<PetLayer> weak = this;
    Ref<LoadingSpinner> loadingRef = loading;

    m_listener.spawn(
        APIClient::getInstance().syncPet(),
        [weak, loadingRef, firstLoad](web::WebResponse res) {
            auto self = weak.lock();
            if (!self) return;

            auto data = PetManager::getInstance().parsePetData(res, loadingRef);
            if (firstLoad) PetManager::getInstance().resetPetDeltas();

            if (!data.ok) {
                if (!firstLoad) {
                    if (loadingRef->getParent()) loadingRef->removeFromParent();
                    self->m_content->setVisible(true);
                }
                return;
            }

            if (loadingRef->getParent()) loadingRef->removeFromParent();
            self->m_petLabel = nullptr;
            self->m_content->removeAllChildren();
            self->m_content->setVisible(true);
            self->drawUIFromData(data, self->m_content);
        }
    );
}

void PetLayer::drawUIFromData(PetManager::PetData data, CCNode* mainPanel) {
    if (data.isBanned) {
        changeMusic(MusicMode::Out);
        this->onBack(nullptr);
        return;
    }
    using levelgrind::clanui::darkBox;
    auto& pm = PetManager::getInstance();
    CCSize const size = mainPanel->getContentSize();
    float const cx = size.width / 2.f;
    WeakRef<PetLayer> weakLayer = this;

    m_petLabel = CCLabelBMFont::create(data.petName.c_str(), "goldFont.fnt");
    m_petLabel->setPosition({ cx, size.height - 23.f });
    m_petLabel->limitLabelWidth(size.width - 80.f, 0.85f, 0.35f);
    m_petLabel->setID("pet-name-label");
    mainPanel->addChild(m_petLabel);

    float const pad = 10.f;
    float const rowH = 20.f;
    float const barH = 26.f;
    float const btnH = 30.f;
    float const gap = 6.f;
    float const cardH = pad * 2.f + rowH * 2.f + barH + btnH + gap * 3.f;
    CCSize const sideSize { 124.f, cardH };
    CCSize const petSize { 150.f, cardH };
    float const innerW = sideSize.width - pad * 2.f;
    float const cardsY = size.height - 44.f - cardH / 2.f;
    float const sideX = petSize.width / 2.f + 10.f + sideSize.width / 2.f;

    auto makeCard = [&](CCSize cs, float x, const char* id) {
        auto card = CCNode::create();
        card->setContentSize(cs);
        card->setAnchorPoint({ 0.5f, 0.5f });
        card->setPosition({ cx + x, cardsY });
        card->setID(id);
        auto box = darkBox(cs, 85);
        box->setPosition(cs / 2.f);
        card->addChild(box, -2);
        mainPanel->addChild(card);
        return card;
    };
    auto petPanel = makeCard(petSize, 0.f, "pet-panel");
    auto leftCard = makeCard(sideSize, -sideX, "left-card");
    auto rightCard = makeCard(sideSize, sideX, "right-card");

    auto column = [&](CCNode* card) {
        auto col = CCNode::create();
        col->setContentSize({ innerW, cardH - pad * 2.f });
        col->setAnchorPoint({ 0.5f, 0.5f });
        col->setPosition(sideSize / 2.f);
        col->setLayout(ColumnLayout::create()->setGap(gap)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::Between)->setAutoScale(false));
        card->addChild(col);
        return col;
    };
    auto statRow = [&](CCNode* col, CCSprite* icon, std::string const& value) {
        auto row = CCNode::create();
        row->setContentSize({ innerW, rowH });
        row->setAnchorPoint({ 0.5f, 0.5f });
        row->setLayout(RowLayout::create()->setGap(5.f)->setAutoScale(false));
        icon->setScale(17.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
        row->addChild(icon);
        auto label = CCLabelBMFont::create(value.c_str(), "bigFont.fnt");
        label->limitLabelWidth(innerW - 24.f, 0.42f, 0.15f);
        row->addChild(label);
        row->updateLayout();
        col->addChild(row);
    };
    auto bigButton = [&](CCNode* col, std::string const& text, const char* bg, geode::Function<void()> action) {
        auto menu = CCMenu::create();
        menu->setContentSize({ innerW, btnH });
        menu->ignoreAnchorPointForPosition(false);
        auto spr = ButtonSprite::create(text.c_str(), 84, true, "bigFont.fnt", bg, 30.f, 0.6f);
        spr->setScale(0.8f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [action = std::move(action)](CCObject*) mutable { action(); });
        btn->setPosition(menu->getContentSize() / 2.f);
        menu->addChild(btn);
        col->addChild(menu);
    };
    auto barBlock = [&](CCNode* col, float percent, std::string const& text) {
        auto node = CCNode::create();
        node->setContentSize({ innerW, barH });
        node->setAnchorPoint({ 0.5f, 0.5f });
        auto bar = ProgressBar::create(ProgressBar::Style::Slider);
        float const w = bar->getContentWidth();
        bar->setScale(w > 0.f ? innerW / w : 0.5f);
        bar->setAnchorPoint({ 0.5f, 0.5f });
        bar->setPosition({ innerW / 2.f, 7.f });
        bar->updateProgress(std::clamp(percent, 0.f, 100.f));
        node->addChild(bar);
        auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        label->limitLabelWidth(innerW, 0.3f, 0.12f);
        label->setPosition({ innerW / 2.f, barH - 6.f });
        node->addChild(label);
        col->addChild(node);
    };

    auto left = column(leftCard);
    statRow(left, CCSprite::create("grind_point_icon.png"_spr), numToString(data.gp));
    auto streakLabel = CCLabelBMFont::create(fmt::format("{} day streak", data.streak).c_str(), "bigFont.fnt");
    streakLabel->limitLabelWidth(innerW, 0.36f, 0.15f);
    streakLabel->setColor(data.streak > 0 ? ccColor3B{ 255, 200, 90 } : ccColor3B{ 170, 170, 170 });
    left->addChild(streakLabel);
    int const nextBonus = std::min((data.streak / 7 + 1) * 7, (data.streak / 30 + 1) * 30);
    barBlock(left, data.streak * 100.f / std::max(1, nextBonus), fmt::format("{} / {}", data.streak, nextBonus));
    bool const fedToday = data.mood >= 2;
    bigButton(left, fedToday ? "Fed" : fmt::format("Feed {}", data.feedCost), fedToday ? "GJ_button_04.png" : "GJ_button_02.png",
        [data, fedToday, weakLayer] {
            if (fedToday) return;
            if (data.gp < data.feedCost) {
                Notification::create("Not enough Grind Points", NotificationIcon::Error)->show();
                return;
            }
            if (auto layer = weakLayer.lock()) layer->feed();
        });
    left->updateLayout();

    auto right = column(rightCard);
    statRow(right, CCSprite::createWithSpriteFrameName("shard0201ShardBig_001.png"), pm.getRarityFromInt(data.petRarity));
    statRow(right, CCSprite::createWithSpriteFrameName("shard0203ShardBig_001.png"),
        fmt::format("Lvl {}", pm.isMaxLevel(data.petLevel) ? "MAX" : numToString(data.petLevel)));
    bool const maxed = pm.isMaxLevel(data.petLevel);
    int const nextCost = pm.getUpgradeLvlCostByLevel(data.petLevel);
    float pct = maxed || nextCost <= 0 ? 100.f : std::min(100.f, static_cast<float>(data.gp) * 100.f / static_cast<float>(nextCost));
    if (!maxed && data.gp > 0) pct = std::max(pct, 3.f);
    barBlock(right, pct, maxed ? "MAX" : fmt::format("Next: {} GP", nextCost));
    bigButton(right, "Upgrade", "GJ_button_01.png", [data, weakLayer] {
        PetUpgradePopup::create(data, [weakLayer] {
            if (auto layer = weakLayer.lock()) layer->reload();
        })->show();
    });
    right->updateLayout();

    auto ageLabel = CCLabelBMFont::create(pm.getPetAgeFromLevel(data.petLevel).c_str(), "goldFont.fnt");
    ageLabel->setScale(0.6f);
    ageLabel->setPosition({ petSize.width / 2.f, cardH - 18.f });
    petPanel->addChild(ageLabel, 10);

    auto moodLabel = CCLabelBMFont::create(PetManager::moodName(data.mood).c_str(), "bigFont.fnt");
    moodLabel->setScale(0.34f);
    moodLabel->setColor(PetManager::moodColor(data.mood));
    moodLabel->setPosition({ petSize.width / 2.f, cardH - 35.f });
    moodLabel->setID("mood-label");
    petPanel->addChild(moodLabel, 10);

    drawPet(pm.getStyleByLevel(data.petLevel), data.petLevel, data.mood);

    auto secondary = CCMenu::create();
    secondary->setLayout(RowLayout::create()->setGap(8.f)->setAutoScale(false));
    secondary->setContentSize({ size.width - 40.f, 26.f });
    secondary->setPosition({ cx, cardsY - cardH / 2.f - 22.f });
    secondary->setID("secondary-menu");
    mainPanel->addChild(secondary);

    auto addSecondary = [&](const char* text, const char* bg, geode::Function<void()> action) {
        auto spr = ButtonSprite::create(text, "bigFont.fnt", bg, 0.8f);
        spr->setScale(0.62f);
        secondary->addChild(CCMenuItemExt::createSpriteExtra(spr, [action = std::move(action)](CCObject*) mutable { action(); }));
    };
    addSecondary("Rename", "GJ_button_01.png", [data] {
        PetRenamePopup::create(data.petName)->show();
    });
    addSecondary("Customize", data.petRarity < 2 ? "GJ_button_04.png" : "GJ_button_01.png", [data, weakLayer] {
        if (data.petRarity < 2) {
            FLAlertLayer::create("Low Rarity!", "You need to have <co>Rare</c> to start customizing your pet.", "OK")->show();
            return;
        }
        PetCustomisePopup::create(data, [weakLayer] {
            if (auto layer = weakLayer.lock()) layer->reload();
        })->show();
    });
    addSecondary("Leaderboard", "GJ_button_01.png", [] {
        PetLeaderboardPopup::create()->show();
    });
    secondary->updateLayout();

    if (m_lastRarity >= 0 && data.petRarity > m_lastRarity) this->celebrate(true, data);
    else if (m_lastLevel >= 0 && data.petLevel > m_lastLevel) this->celebrate(false, data);
    m_lastLevel = data.petLevel;
    m_lastRarity = data.petRarity;
}

void PetLayer::celebrate(bool rarity, PetManager::PetData const& data) {
    auto petPanel = getChildByIDRecursive("pet-panel");
    if (!petPanel) return;
    CCPoint center = { petPanel->getContentWidth() / 2.f, petPanel->getContentHeight() / 2.f - 12.f };
    if (auto spr = typeinfo_cast<CCNode*>(petPanel->getChildByIDRecursive("pet-sprite"))) {
        petanim::stopMood(spr);
        petanim::pop(spr, 1.f, rarity);
        Ref<CCNode> ref = spr;
        int mood = data.mood;
        spr->runAction(CCSequence::create(CCDelayTime::create(rarity ? 0.75f : 0.7f), CallFuncExt::create([ref, mood] {
            petanim::playMood(ref, mood);
        }), nullptr));
    }
    if (!petanim::enabled()) return;
    petanim::burst(petPanel, center, rarity ? ccColor3B{ 255, 200, 255 } : ccColor3B{ 255, 230, 120 }, rarity ? 14 : 8);
}

void PetLayer::drawPet(PetManager::PetStyle style, int petLevel, int mood) {
    auto petPanel = getChildByIDRecursive("pet-panel");
    if (!petPanel) return;

    auto petMenu = CCMenu::create();
    petMenu->setID("pet-menu");
    petMenu->setContentSize({ 40.f, 40.f });
    petMenu->ignoreAnchorPointForPosition(false);
    petMenu->setAnchorPoint({ 0.5f, 0.5f });

    auto petSpr = PetManager::getInstance().getPetSprByStyle(style);
    CCSize petBox { 30.f, 30.f };
    if (auto player = petSpr->getChildByType<SimplePlayer>(0)) {
        if (player->m_firstLayer) petBox = player->m_firstLayer->getContentSize() * player->getScale();
    }
    auto petShadow = CCSprite::createWithSpriteFrameName("chest_shadow_001.png");
    petShadow->setOpacity(90);
    float const shadowW = petBox.width * 1.8f;
    petShadow->setScaleX(shadowW / std::max(1.f, petShadow->getContentWidth()));
    petShadow->setScaleY(shadowW * 0.22f / std::max(1.f, petShadow->getContentHeight()));
    petShadow->setPosition({ 20.f, 17.f - petBox.height / 2.f });
    petMenu->addChild(petShadow, -1);
    auto holder = CCNode::create();
    holder->setContentSize({ 40.f, 40.f });
    holder->setAnchorPoint({ 0.5f, 0.5f });
    petSpr->setPosition({ 20.f, 17.f });
    holder->addChild(petSpr);
    Ref<CCNode> sprRef = petSpr;
    auto petBtn = CCMenuItemExt::createSpriteExtra(holder, [sprRef](CCObject*) {
        if (!sprRef) return;
        petanim::jiggle(sprRef, 1.f);
        petanim::hearts(sprRef->getParent(), sprRef->getPosition() + CCPoint{ 0.f, 18.f });
    });
    petBtn->m_scaleMultiplier = 1.f;
    petBtn->setPosition({ 20.f, 20.f });
    petMenu->addChild(petBtn);

    float const top = petPanel->getContentHeight() - 46.f;
    petMenu->setPosition({ petPanel->getContentWidth() / 2.f, (top + 6.f) / 2.f });
    petMenu->setScale(std::min(1.6f, PetManager::getInstance().getPetScale(petLevel) * 1.15f));
    petPanel->addChild(petMenu);
    petanim::playMood(petSpr, mood);
}

void PetLayer::feed() {
    if (m_feeding) return;
    m_feeding = true;
    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    WeakRef<PetLayer> weak = this;
    m_feedTask.spawn(auth::post(fmt::format("{}/pet_feed", APIClient::getInstance().baseUrl), std::move(body)), [weak](web::WebResponse res) {
        auto self = weak.lock();
        if (!self) return;
        self->m_feeding = false;
        auto json = res.json();
        if (!res.ok() || !json || !json.unwrap()["ok"].asBool().unwrapOr(false)) {
            Notification::create(APIClient::errorMessage(res, "Failed! Try again later."), NotificationIcon::Error)->show();
            return;
        }
        int reward = static_cast<int>(json.unwrap()["reward"].asInt().unwrapOr(0));
        int streak = static_cast<int>(json.unwrap()["streak"].asInt().unwrapOr(1));
        if (auto panel = self->getChildByIDRecursive("pet-panel")) {
            if (auto spr = panel->getChildByIDRecursive("pet-sprite")) {
                petanim::stopMood(spr);
                petanim::gulp(spr, 1.f);
                petanim::hearts(spr->getParent(), spr->getPosition() + CCPoint{ 0.f, 20.f });
            }
        }
        if (reward > 0) {
            notify::show(fmt::format("{} day streak!", streak), fmt::format("+{} GP for feeding your pet", reward), "grind_point_icon.png"_spr, true);
        }
        self->runAction(CCSequence::create(CCDelayTime::create(1.f), CallFuncExt::create([weak] {
            if (auto self = weak.lock()) self->reload();
        }), nullptr));
    });
}

void PetLayer::reload() {
    loadData(false);
}

void PetLayer::onBack(CCObject *sender) {
    changeMusic(MusicMode::Out);

    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade
    );
}

}