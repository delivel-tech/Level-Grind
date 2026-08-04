#include "PetUpgradePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/utils/async.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <fmt/format.h>
#include <matjson.hpp>

#include "../../core/BackendManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

PetUpgradePopup* PetUpgradePopup::create(SyncPetResponse petData) {
    auto ret = new PetUpgradePopup;
    if (ret && ret->init(petData)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PetUpgradePopup::init(SyncPetResponse petData) {
    if (!Popup::init(260.f, 180.f)) return false;

    this->setTitle("Upgrade Pet");

    m_petData = petData;

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(ColumnLayout::create()->setGap(10));

    auto upgradeLevelBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Upgrade Level"),
        this,
        menu_selector(PetUpgradePopup::onUpgradeLevel)
    );

    auto upgradeRarityBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Upgrade Rarity"),
        this,
        menu_selector(PetUpgradePopup::onUpgradeRarity)
    );

    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Center);

    btnMenu->addChild(upgradeLevelBtn);
    btnMenu->addChild(upgradeRarityBtn);

    btnMenu->updateLayout();

    return true;
}

void PetUpgradePopup::onClosePopup(::UploadActionPopup* popup) {
    if (popup) {
        popup->m_delegate = nullptr;
        if (popup->getParent()) {
            popup->closePopup();
        }
    }
    this->onClose(nullptr);
}

void PetUpgradePopup::onUpgradeRarity(CCObject* sender) {
    if (PetManager::getInstance().m_isRarityUpgraded) {
        Notification::create("Please re-open pet layer", NotificationIcon::Info)->show();
        return;
    }
    PetManager::getInstance().m_isRarityUpgraded = true;
    if (m_petData.petRarity < 1 || m_petData.petRarity > 3) {
        Notification::create("Your pet rarity is fully upgraded", NotificationIcon::Success)->show();
        return;
    }
    int upgCost = getUpgradeRarityCostByCurrRarity(m_petData.petRarity);

    if (m_petData.petMoons < upgCost) {
        Notification::create(
            fmt::format("Not enough moons. Required for upgrade: {}", upgCost),
            NotificationIcon::Error
        )->show();
        return;
    }

    async::spawn(this->onUpgradeRarityClicked(upgCost));
}

arc::Future<> PetUpgradePopup::onUpgradeRarityClicked(int upgradeCost) {
    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet rarity...");
    upopup->show();

    auto uPopupRef = Ref(upopup);

    auto parsed = co_await BackendManager::getInstance().upgradePetRarity(m_petData.petRarity + 1, upgradeCost);

    if (!uPopupRef) co_return;

    if (!parsed.ok) {
        uPopupRef->showFailMessage("Upgrading failed! Try again later.");
        co_return;
    }

    uPopupRef->showSuccessMessage("Success! Pet rarity upgraded.");
    co_return;
}

void PetUpgradePopup::onUpgradeLevel(CCObject* sender) {
    if (PetManager::getInstance().m_isLevelUpgraded) {
        Notification::create("Please re-open pet layer", NotificationIcon::Info)->show();
        return;
    }
    PetManager::getInstance().m_isLevelUpgraded = true;
    if (m_petData.petLevel < 1 || m_petData.petLevel > 29) {
        Notification::create("Your pet level is fully upgraded", NotificationIcon::Success)->show();
        return;
    }

    int upgCost = getUpgradeLvlCostByLevel(m_petData.petLevel);

    if (m_petData.petStars < upgCost) {
        Notification::create(
            fmt::format("Not enough stars. Required for upgrade: {}", upgCost),
            NotificationIcon::Error
        )->show();
        return;
    }

    async::spawn(this->onUpgradeLevelClicked(upgCost));
}

arc::Future<> PetUpgradePopup::onUpgradeLevelClicked(int upgradeCost) {
    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet level...");
    upopup->show();

    auto uPopupRef = Ref(upopup);

    auto parsed = co_await BackendManager::getInstance().upgradePetLevel(m_petData.petLevel + 1, upgradeCost);

    if (!uPopupRef) co_return;

    if (!parsed.ok) {
        uPopupRef->showFailMessage("Upgrading failed! Try again later.");
        co_return;
    }

    uPopupRef->showSuccessMessage("Success! Pet level upgraded.");
    co_return;
}

int PetUpgradePopup::getUpgradeRarityCostByCurrRarity(int rarity) {
    switch (rarity) {
        case 1: return 500; break;
        case 2: return 2000; break;
        case 3: return 4000; break;
        default: return 999999; break;
    }
}

int PetUpgradePopup::getUpgradeLvlCostByLevel(int petLevel) {
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

}