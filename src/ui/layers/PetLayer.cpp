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

#include "../../core/BackendManager.hpp"
#include "../popups/PetRenamePopup.hpp"
#include "../popups/PetUpgradePopup.hpp"
#include "../popups/PetCustomizePopup.hpp"
#include "Geode/ui/ProgressBar.hpp"
#include "Geode/utils/async.hpp"

#include "../popups/GuidePopup.hpp"

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
        .contentSize({460.f, 280.f})
        .parent(this)
        .center()
        .id("main-panel")
        .collect();

    CCSize mainPanelCS = mainPanel->getContentSize();

    auto loading = LoadingSpinner::create(50.f);
    mainPanel->addChild(loading);
    loading->setPosition(
        mainPanelCS.width / 2, mainPanelCS.height / 2
    );

    async::spawn(this->onLoadPetData(Ref(loading), Ref(mainPanel)));

    return true;
}

arc::Future<> PetLayer::onLoadPetData(Ref<LoadingSpinner> loadingRef, Ref<geode::NineSlice> mainPanelRef) {
    Ref<PetLayer> self = this;

    auto data = co_await BackendManager::getInstance().syncPet();

    if (!self || !loadingRef || !mainPanelRef) co_return;

    if (!data.ok) {
        log::error("invalid json in response");
        Notification::create("Failed to load pet data. Try again later.", NotificationIcon::Error)->show();
        loadingRef->removeFromParent();
        PetManager::getInstance().resetPetDeltas();
        co_return;
    }

    Mod::get()->setSavedValue("last-pet-lvl", data.petLevel);
    if (data.isBanned) {
        Notification::create(fmt::format("Banned: {}", data.banReason), NotificationIcon::Error)->show();
        log::info("pet is banned: {}", data.banReason);
    }

    PetManager::getInstance().resetPetDeltas();

    loadingRef->removeFromParent();
    self->drawUIFromData(data, mainPanelRef);

    co_return;
}

void PetLayer::drawUIFromData(SyncPetResponse data, NineSlice* mainPanel) {
    CCSize mainPanelCS = mainPanel->getContentSize();

    m_petLabel = Build(CCLabelBMFont::create(data.petName.c_str(), "goldFont.fnt"))
        .pos(
            {mainPanelCS.width / 2.f, mainPanelCS.height - 28.f}
        )
        .limitLabelWidth(mainPanelCS.width - 40.f, 1.f, 0.35f)
        .id("pet-name-label")
        .parent(mainPanel)
        .collect();

    auto petPanel = Build(NineSlice::create("GJ_square05.png"))
        .contentSize({ 120, 140 })
        .id("pet-panel")
        .pos(
            {mainPanelCS.width / 2.f, mainPanelCS.height / 2.f + 12.f}
        )
        .parent(mainPanel)
        .collect();

    auto statsPanel = Build(NineSlice::create("GJ_square05.png"))
        .contentSize(
            {80, 80}
        )
        .id("stats-panel")
        .pos(
            {mainPanelCS.width / 2.f - 115.f, mainPanelCS.height / 2.f + 15.f}
        )
        .parent(mainPanel)
        .collect();

    auto infoPanel = Build(NineSlice::create("GJ_square05.png"))
        .contentSize(
            {80, 80}
        )
        .id("info-panel")
        .pos(
            {mainPanelCS.width / 2.f + 115.f, mainPanelCS.height / 2.f + 15.f}
        )
        .parent(mainPanel)
        .collect();

    auto statsPanelCS = statsPanel->getContentSize();

    auto createStatRow = [&](float y, char const* iconFrame, int value) {
        auto row = CCNode::create();
        row->setContentSize({statsPanelCS.width - 15.f, 20.f});
        row->setAnchorPoint({0.f, 0.5f});
        row->setPosition({10.f, y});
        statsPanel->addChild(row);

        auto icon = CCSprite::create(iconFrame);
        icon->setAnchorPoint({0.f, 0.5f});
        icon->setScale(1.2f);
        icon->setPosition({0.f, row->getContentSize().height / 2.f});
        row->addChild(icon);

        auto valueLabel =
            CCLabelBMFont::create(numToString(value).c_str(), "bigFont.fnt");
        valueLabel->setAnchorPoint({0.f, 0.5f});
        valueLabel->setScale(0.5f);

        float const iconWidth = icon->getContentSize().width * icon->getScaleX();
        float const gap = 2.f;
        valueLabel->setPosition({iconWidth + gap, row->getContentSize().height / 2.f});
        row->addChild(valueLabel);

        float const maxLabelWidth = row->getContentSize().width - valueLabel->getPositionX();
        limitNodeWidth(valueLabel, maxLabelWidth, 0.5f, 0.2f);
    };

    createStatRow(statsPanelCS.height - 27.f, "star_spr.png"_spr, data.petStars);
    createStatRow(27.f, "moon_spr.png"_spr, data.petMoons);

    auto downBtnMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(15))
        .id("down-btn-menu")
        .pos(
            {mainPanelCS.width / 2.f, mainPanelCS.height / 2.f - 110.f}
        )
        .scale(0.8f)
        .parent(mainPanel)
        .collect();

    auto renamePetBtn = Build(ButtonSprite::create("Rename", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([data] {
            PetRenamePopup::create(data.petName)->show();
        })
        .id("rename-pet-btn")
        .parent(downBtnMenu)
        .collect();

    auto upgradeBtn = Build(ButtonSprite::create("Upgrade", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([data] {
            PetUpgradePopup::create(data)->show();
        })
        .id("upgrade-btn")
        .parent(downBtnMenu)
        .collect();

    auto iconBtn = Build(ButtonSprite::create("Customize", "bigFont.fnt", "GJ_button_01.png"))
        .with([data](ButtonSprite* spr) {
            if (data.petRarity < 2) spr->setColor({ 100, 100, 100 });
        })
        .intoMenuItem([data] {
            if (data.petRarity < 2) {
                FLAlertLayer::create(
                    "Low Rarity!",
                    "You need to have <co>Rare</c> to start customizing your pet.",
                    "OK"
                )->show();
                return;
            }

            PetCustomisePopup::create(data)->show();
        })
        .id("customize-btn")
        .parent(downBtnMenu)
        .collect();

    downBtnMenu->updateLayout();

    auto progressBar = Build(ProgressBar::create(ProgressBarStyle::Slider))
        .id("progress-bar")
        .pos(
            {mainPanelCS.width / 2.f - 105.f, mainPanelCS.height / 2.f - 80.f}
        )
        .parent(mainPanel)
        .collect();

    drawPet(
        PetManager::getInstance().getStyleByLevel(data.petLevel), data.petLevel
    );

    std::string petAgeS = PetManager::getInstance().getPetAgeFromLevel(data.petLevel);

    auto ageLabel = Build<CCLabelBMFont>::create(petAgeS.c_str(), "goldFont.fnt")
        .parent(petPanel)
        .pos(
            { petPanel->getContentWidth() / 2,
            petPanel->getContentHeight() - 20 }
        )
        .scale(0.6f)
        .collect();

    int nextLvlCost = PetManager::getInstance().getUpgradeLvlCostByLevel(data.petLevel);
    float nextLvlCostPercentage = PetManager::getInstance().getNextLevelPercentage(data.petStars, nextLvlCost);

    progressBar->updateProgress(nextLvlCostPercentage);

    auto createInfoRow = [&](float y, char const* iconFrame, std::string text) {
        auto row = CCNode::create();
        row->setContentSize({infoPanel->getContentWidth() - 18.f, 20.f});
        row->setAnchorPoint({0.f, 0.5f});
        row->setPosition({10.f, y});
        infoPanel->addChild(row);

        auto icon = CCSprite::createWithSpriteFrameName(iconFrame);
        icon->setAnchorPoint({0.f, 0.5f});
        icon->setScale(0.5f);
        icon->setPosition({0.f, row->getContentSize().height / 2.f});
        row->addChild(icon);

        auto valueLabel =
            CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        valueLabel->setAnchorPoint({0.f, 0.5f});
        valueLabel->setScale(0.5f);

        float const iconWidth = icon->getContentSize().width * icon->getScaleX();
        float const gap = 5.f;
        valueLabel->setPosition({iconWidth + gap, row->getContentSize().height / 2.f});
        row->addChild(valueLabel);

        float const maxLabelWidth = row->getContentSize().width - valueLabel->getPositionX();
        limitNodeWidth(valueLabel, maxLabelWidth, 0.5f, 0.1f);
    };

    createInfoRow(
        27, 
        "shard0203ShardBig_001.png", 
        fmt::format("Lvl: {}", PetManager::getInstance().isMaxLevel(data.petLevel) ? "MAX" : numToString(data.petLevel))
    );
    createInfoRow(
        infoPanel->getContentWidth() - 27.f, 
        "shard0201ShardBig_001.png", 
        PetManager::getInstance().getRarityFromInt(data.petRarity)
    );

    if (PetManager::getInstance().isMaxLevel(data.petLevel)) {
        progressBar->updateProgress(100);
        return;
    }

    auto currStarsLabel = CCLabelBMFont::create(numToString(data.petStars).c_str(), "bigFont.fnt");
    auto upgStarsLabel = CCLabelBMFont::create(numToString(nextLvlCost).c_str(), "bigFont.fnt");

    currStarsLabel->setAnchorPoint({ 1.f, 0.5f });
    upgStarsLabel->setAnchorPoint({ 0.f, 0.5f });

    currStarsLabel->setScale(0.4f);
    upgStarsLabel->setScale(0.4f);
  
    progressBar->addChildAtPosition(currStarsLabel, Anchor::Left, { -5.f, 0.f });
    progressBar->addChildAtPosition(upgStarsLabel, Anchor::Right, { 5.f, 0.f });

    return;
}

void PetLayer::drawPet(PetManager::PetStyle style, int petLevel) {
    auto petPanel = getChildByIDRecursive("pet-panel");

    auto petMenu = CCMenu::create();
    petMenu->setID("pet-menu");
    petMenu->setLayout(ColumnLayout::create()->setGap(6));

    auto petShadow = CCSprite::createWithSpriteFrameName("chest_shadow_001.png");
    petShadow->setOpacity(90);
    auto petSpr = PetManager::getInstance().getPetSprByStyle(style);

    petMenu->addChild(petShadow);
    petMenu->addChild(petSpr);

    petPanel->addChildAtPosition(petMenu, Anchor::Center, { 0.f, -20.f });

    petMenu->setScale(PetManager::getInstance().getPetScale(petLevel));

    petMenu->updateLayout();
    petShadow->setScale(0.55f);
}

void PetLayer::onBack(CCObject *sender) {
    changeMusic(MusicMode::Out);

    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade
    );
}

}