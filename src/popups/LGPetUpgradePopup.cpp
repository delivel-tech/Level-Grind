#include "LGPetUpgradePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <fmt/format.h>
#include <matjson.hpp>

#include "../other/LGManager.hpp"
#include "../other/PetManager.hpp"

using namespace geode::prelude;

LGPetUpgradePopup* LGPetUpgradePopup::create(PetLayer::PetData petData) {
    auto ret = new LGPetUpgradePopup;
    if (ret && ret->init(petData)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGPetUpgradePopup::init(PetLayer::PetData petData) {
    if (!Popup::init(260.f, 180.f)) return false;

    this->setTitle("Upgrade Pet");

    m_petData = petData;

    addSideArt(
        m_mainLayer, 
        SideArt::All, 
        SideArtStyle::PopupGold, 
        false
    );

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(ColumnLayout::create()->setGap(10));

    auto upgradeLevelBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Upgrade Level"),
        this,
        menu_selector(LGPetUpgradePopup::onUpgradeLevel)
    );

    auto upgradeRarityBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Upgrade Rarity"),
        this,
        menu_selector(LGPetUpgradePopup::onUpgradeRarity)
    );

    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Center);

    btnMenu->addChild(upgradeLevelBtn);
    btnMenu->addChild(upgradeRarityBtn);

    btnMenu->updateLayout();

    return true;
}

void LGPetUpgradePopup::onClosePopup(::UploadActionPopup* popup) {
    if (popup) {
        popup->m_delegate = nullptr;
        if (popup->getParent()) {
            popup->closePopup();
        }
    }
    this->onClose(nullptr);
}

void LGPetUpgradePopup::onUpgradeRarity(CCObject* sender) {
    if (PetManager::get()->m_isRarityUpgraded) {
        Notification::create("Please re-open pet layer", NotificationIcon::Info)->show();
        return;
    }
    PetManager::get()->m_isRarityUpgraded = true;
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

    web::WebRequest req;

    matjson::Value body;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    body["newRarity"] = m_petData.petRarity + 1;
    body["upgradeCost"] = upgCost;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet rarity...");
    upopup->show();

    auto uPopupRef = Ref(upopup);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/upgrade_pet_rarity"),
        [uPopupRef](web::WebResponse res) {
            if (!uPopupRef) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            }

            auto json = jsonRes.unwrap();

            auto isOK = json["ok"].asBool().unwrapOrDefault();

            if (!isOK) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Pet rarity upgraded.");
                return;
            }
            return;
        }
    );
}

void LGPetUpgradePopup::onUpgradeLevel(CCObject* sender) {
    if (PetManager::get()->m_isLevelUpgraded) {
        Notification::create("Please re-open pet layer", NotificationIcon::Info)->show();
        return;
    }
    PetManager::get()->m_isLevelUpgraded = true;
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

    web::WebRequest req;

    matjson::Value body;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    body["newLevel"] = m_petData.petLevel + 1;
    body["upgradeCost"] = upgCost;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet level...");
    upopup->show();

    auto uPopupRef = Ref(upopup);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/upgrade_pet_level"),
        [uPopupRef](web::WebResponse res) {
            if (!uPopupRef) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            }

            auto json = jsonRes.unwrap();

            auto isOK = json["ok"].asBool().unwrapOrDefault();

            if (!isOK) {
                uPopupRef->showFailMessage("Upgrading failed! Try again later.");
                log::error("req failed, error: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Pet level upgraded.");
                return;
            }
            return;
        }
    );
}

int LGPetUpgradePopup::getUpgradeRarityCostByCurrRarity(int rarity) {
    switch (rarity) {
        case 1: return 500; break;
        case 2: return 2000; break;
        case 3: return 4000; break;
        default: return 999999; break;
    }
}

int LGPetUpgradePopup::getUpgradeLvlCostByLevel(int petLevel) {
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