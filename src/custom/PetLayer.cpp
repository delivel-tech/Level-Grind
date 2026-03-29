#include "PetLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/cocos/sprite_nodes/CCSpriteFrameCache.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <string>

#include "Geode/ui/Notification.hpp"
#include "Geode/ui/ProgressBar.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"
#include "LGSettingsLayer.hpp"

#include "../popups/LGPetRenamePopup.hpp"
#include "../popups/LGPetUpgradePopup.hpp"
#include "../other/PetManager.hpp"

using namespace geode::prelude;

int PetLayer::getUpgradeRarityCostByCurrRarity(int rarity) {
    switch (rarity) {
        case 1: return 500; break;
        case 2: return 2000; break;
        case 3: return 4000; break;
        default: return 999999; break;
    }
}

PetLayer::PetData PetLayer::parsePetData(web::WebResponse res) {
    PetLayer::PetData data;

    auto jsonRes = res.json();

    if (!jsonRes) {
        log::error("invalid json in response");
        this->onBack(nullptr);
        Notification::create("Failed to load pet data. Try again later.", NotificationIcon::Error)->show();
        return data;
    }

    auto jsonUnwrapped = jsonRes.unwrap();
    auto json = jsonUnwrapped["pet"];

    data.petName = json["petName"].asString().unwrapOrDefault();
    data.petStars = json["petStars"].asInt().unwrapOrDefault();
    data.petMoons = json["petMoons"].asInt().unwrapOrDefault();
    data.petLevel = json["petLevel"].asInt().unwrapOrDefault();
    data.petRarity = json["petRarity"].asInt().unwrapOrDefault();
    data.isBanned = json["isBanned"].asBool().unwrapOrDefault();
    if (data.isBanned) {
        data.banReason = json["banReason"].asString().unwrapOrDefault();
        Notification::create(fmt::format("Banned: {}", data.banReason), NotificationIcon::Error)->show();
        log::info("pet is banned: {}", data.banReason);
        this->onBack(nullptr);
        return data;
    }

    return data;
}

float PetLayer::getNextLevelPercentage(int petStars, int nextLvlCost) {
    float percentage = ((float)petStars / nextLvlCost) * 100;
    if (percentage > 100) {
        percentage = 100;
    }

    return percentage;
}

void PetLayer::onRenameBtn(CCObject* sender) {
    LGPetRenamePopup::create(m_petName)->show();
}

void PetLayer::goToPetLayer(web::WebResponse response) {
    auto layer = PetLayer::create(response);
    if (!layer) {
        return;
    }

    auto scene = CCScene::create();
    scene->addChild(layer);

    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->pushScene(transition);
}

PetLayer* PetLayer::create(web::WebResponse response) {
  auto ret = new PetLayer;
  if (ret && ret->init(response)) {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

void PetLayer::onInfoBtn(CCObject* sender) {
    MDPopup::create(
        "Grinding Pet Information",
        "# <cg>Grinding Pet</c>\n\n"
        "<cg>Grinding Pet</c> is your own <cj>companion</c> that evolves when you <cy>grind stats</c>!\n\n"
        "## <cp>How does it work?</c>\n\n"
        "- <cy>Pet stars and Pet moons</c> are currencies of <cg>Grinding Pet</c>, you can use them to <cl>upgrade your pet</c>!\n"
        "- You can <cp>earn them</c> by grinding stats as usual.\n"
        "- There are <cr>30 levels in total</c>, for each one, your <cr>pet will become bigger</c> and may <cy>change its style</c>!\n"
        "- Currently, pet <cb>rarity</c> does nothing, however, in future it will give lower prices in <cb>Pet Shop</c>.\n"
        "# <cy>Enjoy the game</c>!",
        "OK"
    )->show();
}

int PetLayer::getUpgradeLvlCostByLevel(int petLevel) {
    const int MAX_LEVEL = 30;
    const int TOTAL_COST = 100000;
    
    auto baseCost = [&](int level) -> double {
        return pow(level, 2);
    };

    auto getTotalBase = [&]() -> double {
        double sum = 0;
        for (int i = 1; i <= MAX_LEVEL; i++) {
            sum += baseCost(i);
        }
        return sum;
    };

    static double scale = TOTAL_COST / getTotalBase();
    int nextLevel = petLevel + 1;

    return (int)(baseCost(nextLevel) * scale);
}

void PetLayer::onUpgradeBtn(CCObject* sender) {
    LGPetUpgradePopup::create(m_petData)->show();
}

std::string PetLayer::getPetAgeFromLevel(int petLevel) {
    if (petLevel < 5) {
        return "Baby";
    } else if (petLevel >= 5 && petLevel < 10) {
        return "Toddler";
    } else if (petLevel >= 10 && petLevel < 15) {
        return "Teen";
    } else if (petLevel >= 15 && petLevel < 20) {
        return "Adult";
    } else if (petLevel >= 20 && petLevel < 25) {
        return "Elder";
    } else if (petLevel >= 25 && petLevel < 30) {
        return "Master";
    } else {
        return "Ascended";
    }
}

bool PetLayer::init(web::WebResponse response) {
  if (!BaseLayer::init())
    return false;

  auto petData = parsePetData(std::move(response));
  if (!petData.petName.empty()) {
    m_petName = petData.petName;
  }

  Mod::get()->setSavedValue("last-pet-lvl", petData.petLevel);

  PetManager::get()->m_isLevelUpgraded = false;
  PetManager::get()->m_isRarityUpgraded = false;

  m_petData = petData;

  CCSize winSize = CCDirector::sharedDirector()->getWinSize();

  auto infoBtnSpr = CCSprite::create("info_btn.png"_spr);
  infoBtnSpr->setScale(0.7f);

  auto infoBtn = CCMenuItemSpriteExtra::create(
    infoBtnSpr,
    this,
    menu_selector(PetLayer::onInfoBtn)
  );

  infoBtn->setID("info-btn");

  auto infoBtnMenu = CCMenu::create();
  infoBtnMenu->setID("info-btn-menu");

  infoBtnMenu->addChild(infoBtn);
  infoBtnMenu->setPosition({ winSize.width - 25.f, 25.f });

  this->addChild(infoBtnMenu);

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

  if (!Mod::get()->getSavedValue<bool>("disable-star-particles")) {
    auto grindParticles = CCParticleSnow::create();
    auto texture = CCTextureCache::sharedTextureCache()->addImage(
        "GJ_bigStar_noShadow.png"_spr, true);
    grindParticles->m_fStartSpin = 0.f;
    grindParticles->m_fEndSpin = 180.f;
    grindParticles->m_fStartSize = 6.f;
    grindParticles->m_fEndSize = 3.f;
    grindParticles->setTexture(texture);

    this->addChild(grindParticles);
  }

  auto fmod = FMODAudioEngine::sharedEngine();
  fmod->pauseMusic(0);
  fmod->playMusic("secretShop.mp3", true, 1.f, 0);

  auto mainPanel = NineSlice::create("GJ_square02.png");
  mainPanel->setContentSize({460.f, 280.f});
  mainPanel->setPosition({winSize.width / 2.f, winSize.height / 2.f});
  mainPanel->setID("main-panel");
  this->addChild(mainPanel);

  auto mainPanelCS = mainPanel->getContentSize();

  m_petLabel = CCLabelBMFont::create(m_petName.c_str(), "goldFont.fnt");
  m_petLabel->setPosition({mainPanelCS.width / 2.f, mainPanelCS.height - 28.f});
  limitNodeWidth(m_petLabel, mainPanelCS.width - 40.f, 1.f, 0.35f);
  m_petLabel->setID("pet-name-label");
  mainPanel->addChild(m_petLabel);

  auto petPanel = NineSlice::create("GJ_square05.png");
  petPanel->setContentSize({120.f, 140.f});
  petPanel->setID("pet-panel");
  petPanel->setPosition({mainPanelCS.width / 2.f, mainPanelCS.height / 2.f + 12.f});
  mainPanel->addChild(petPanel);

  auto statsPanel = NineSlice::create("GJ_square05.png");
  statsPanel->setContentSize({80.f, 80.f});
  statsPanel->setID("stats-panel");
  statsPanel->setPosition({mainPanelCS.width / 2.f - 115.f, mainPanelCS.height / 2.f + 15.f});
  mainPanel->addChild(statsPanel);

  auto infoPanel = NineSlice::create("GJ_square05.png");
  infoPanel->setContentSize({80.f, 80.f});
  infoPanel->setID("info-panel");
  infoPanel->setPosition({mainPanelCS.width / 2.f + 115.f, mainPanelCS.height / 2.f + 15.f});
  mainPanel->addChild(infoPanel);

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

  createStatRow(statsPanelCS.height - 27.f, "star_spr.png"_spr, petData.petStars);
  createStatRow(27.f, "moon_spr.png"_spr, petData.petMoons);

  auto downBtnMenu = CCMenu::create();
  downBtnMenu->setLayout(RowLayout::create()->setGap(15));
  downBtnMenu->setID("down-btn-menu");
  downBtnMenu->setPosition({mainPanelCS.width / 2.f, mainPanelCS.height / 2.f - 110.f});
  mainPanel->addChild(downBtnMenu);

  auto renamePetBtn =
      CCMenuItemSpriteExtra::create(ButtonSprite::create("Rename"), this, menu_selector(PetLayer::onRenameBtn));
  renamePetBtn->setID("rename-pet-btn");
  downBtnMenu->addChild(renamePetBtn);

  auto upgradeBtn =
      CCMenuItemSpriteExtra::create(ButtonSprite::create("Upgrade"), this, menu_selector(PetLayer::onUpgradeBtn));
  upgradeBtn->setID("upgrade-btn");
  downBtnMenu->addChild(upgradeBtn);
  downBtnMenu->updateLayout();

  auto rightSideMenu = CCMenu::create();
  rightSideMenu->setLayout(
      ColumnLayout::create()->setAxisReverse(true)->setGap(15));
  rightSideMenu->setID("right-side-menu");
  rightSideMenu->setPosition({mainPanelCS.width - 25.f, mainPanelCS.height / 2.f});
  mainPanel->addChild(rightSideMenu);

  auto settingsBtn = CCMenuItemSpriteExtra::create(
      CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png"), this,
      menu_selector(PetLayer::onSettingsBtn));
  settingsBtn->setID("settings-btn");
  rightSideMenu->addChild(settingsBtn);
  rightSideMenu->updateLayout();

  auto progressBar = ProgressBar::create(ProgressBarStyle::Slider);
  progressBar->setID("progress-bar");
  progressBar->setPosition(
      {mainPanelCS.width / 2.f - 105.f, mainPanelCS.height / 2.f - 80.f});
  mainPanel->addChild(progressBar);

  drawPet(getStyleByLevel(petData.petLevel), petData.petLevel);

  std::string petAgeS = getPetAgeFromLevel(petData.petLevel);

  CCLabelBMFont* ageLabel = CCLabelBMFont::create(
    petAgeS.c_str(),
    "goldFont.fnt"
  );
  petPanel->addChildAtPosition(ageLabel, Anchor::Top, { 0.f, -20.f });
  ageLabel->setScale(0.6f);

  int nextLvlCost = getUpgradeLvlCostByLevel(petData.petLevel);

  float nextLvlCostPercentage = getNextLevelPercentage(petData.petStars, nextLvlCost);

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
  
  createInfoRow(27, "shard0203ShardBig_001.png", fmt::format("Lvl: {}", maxLevel(petData.petLevel) ? "MAX" : numToString(petData.petLevel)));
  createInfoRow(infoPanel->getContentWidth() - 27.f, "shard0201ShardBig_001.png", getRarityFromInt(petData.petRarity));

  if (maxLevel(petData.petLevel)) return true;

  auto currStarsLabel = CCLabelBMFont::create(numToString(petData.petStars).c_str(), "bigFont.fnt");
  auto upgStarsLabel = CCLabelBMFont::create(numToString(nextLvlCost).c_str(), "bigFont.fnt");

  currStarsLabel->setAnchorPoint({ 1.f, 0.5f });
  upgStarsLabel->setAnchorPoint({ 0.f, 0.5f });

  currStarsLabel->setScale(0.4f);
  upgStarsLabel->setScale(0.4f);
  
  progressBar->addChildAtPosition(currStarsLabel, Anchor::Left, { -5.f, 0.f });
  progressBar->addChildAtPosition(upgStarsLabel, Anchor::Right, { 5.f, 0.f });

  return true;
}

bool PetLayer::maxLevel(int petLevel) {
    if (petLevel > 29) return true;
    return false;
}

float PetLayer::getPetScale(int level) {
    const float minScale = 0.9f;
    const float maxScale = 1.5f;
    const int maxLevel = 30;

    if (level <= 1) return minScale;
    if (level >= maxLevel) return maxScale;

    float t = (float)(level - 1) / (maxLevel - 1);
    return minScale * pow(maxScale / minScale, t);
}

void PetLayer::drawPet(PetLayer::PetStyle style, int petLevel) {
    auto petPanel = getChildByIDRecursive("pet-panel");

    auto petMenu = CCMenu::create();
    petMenu->setID("pet-menu");
    petMenu->setLayout(ColumnLayout::create()->setGap(6));

    auto petShadow = CCSprite::createWithSpriteFrameName("chest_shadow_001.png");
    petShadow->setOpacity(90);
    auto petSpr = getPetSprByStyle(style);

    petMenu->addChild(petShadow);
    petMenu->addChild(petSpr);

    petMenu->updateLayout();

    petPanel->addChildAtPosition(petMenu, Anchor::Center, { 0.f, -20.f });
    petShadow->setScale(0.55f);

    petMenu->setScale(getPetScale(petLevel));
}

std::string PetLayer::getRarityFromInt(int rarity) {
    std::string rarityText = "";
    switch (rarity) {
        case 1: rarityText = "Common"; break;
        case 2: rarityText = "Rare"; break;
        case 3: rarityText = "Legendary"; break;
        case 4: rarityText = "Mythic"; break;
    }

    return rarityText;
}

CCSprite* PetLayer::getPetSprByStyle(PetLayer::PetStyle style) {
    CCSprite* sprite = nullptr;
    if (style == PetStyle::StandardCube) {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(2);
        playerSpr->updatePlayerFrame(2, IconType::Cube);
        playerSpr->setColors({111, 255, 0}, {0, 251, 255});
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    } else if (style == PetStyle::OwnCube) {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(GameManager::sharedState()->m_playerFrame);
        playerSpr->updatePlayerFrame(GameManager::sharedState()->m_playerFrame, IconType::Cube);
        playerSpr->setColors({111, 255, 0}, {0, 251, 255});
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    } else {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(GameManager::sharedState()->m_playerFrame);
        playerSpr->updatePlayerFrame(GameManager::sharedState()->m_playerFrame, IconType::Cube);
        playerSpr->setColors(
            GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerColor),
            GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerColor2)
        );
        if (GameManager::sharedState()->m_playerGlow != 0) {
            playerSpr->setGlowOutline(
                GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerGlowColor)
            );
        }
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    }

    sprite->setID("pet-sprite");
    return sprite;
}

PetLayer::PetStyle PetLayer::getStyleByLevel(int petLevel) {
    PetStyle style;

    if (petLevel < 5) {
        style = PetStyle::StandardCube;
    } else if (petLevel >= 5 && petLevel < 15) {
        style = PetStyle::OwnCube;
    } else {
        style = PetStyle::OwnCubeWithColors;
    }

    return style;
}

void PetLayer::onSettingsBtn(CCObject* sender) {
    auto layer = LGSettingsLayer::create(true);
    auto scene = CCScene::create();

    auto transition = CCTransitionFade::create(0.5f, scene);

    scene->addChild(layer);

    CCDirector::sharedDirector()->pushScene(transition);
}

void PetLayer::onBack(CCObject *sender) {
  auto gm = GameManager::sharedState();
  gm->fadeInMenuMusic();

  CCDirector::sharedDirector()->popSceneWithTransition(
      0.5f, PopTransition::kPopTransitionFade);
}
