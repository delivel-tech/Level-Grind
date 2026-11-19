#include "../../managers/Auth.hpp"
#include "PetUpgradePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <fmt/format.h>
#include <matjson.hpp>

#include "../../managers/DataManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

PetUpgradePopup* PetUpgradePopup::create(PetManager::PetData petData, geode::Function<void()> onUpgraded) {
    auto ret = new PetUpgradePopup;
    if (ret && ret->init(petData, std::move(onUpgraded))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PetUpgradePopup::init(PetManager::PetData petData, geode::Function<void()> onUpgraded) {
    if (!Popup::init(260.f, 180.f)) return false;
    m_onUpgraded = std::move(onUpgraded);
    PetManager::getInstance().m_isLevelUpgraded = false;
    PetManager::getInstance().m_isRarityUpgraded = false;

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
    if (PetManager::getInstance().m_isRarityUpgraded) return;
    if (m_petData.petRarity < 1 || m_petData.petRarity >= PetManager::kMaxRarity) {
        Notification::create("Your pet rarity is fully upgraded", NotificationIcon::Success)->show();
        return;
    }
    int upgCost = getUpgradeRarityCostByCurrRarity(m_petData.petRarity);

    if (m_petData.gp < upgCost) {
        Notification::create(
            fmt::format("Not enough Grind Points. Required for upgrade: {}", upgCost),
            NotificationIcon::Error
        )->show();
        return;
    }

    web::WebRequest req;

    matjson::Value body;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;

    body["newRarity"] = m_petData.petRarity + 1;
    body["upgradeCost"] = upgCost;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet rarity...");
    upopup->show();

    auto uPopupRef = Ref(upopup);
    PetManager::getInstance().m_isRarityUpgraded = true;
    Ref<PetUpgradePopup> self = this;

    m_listener.spawn(
        auth::post("https://api.delivel.tech/v2/upgrade_pet_rarity", body),
        [uPopupRef, self, upgCost](web::WebResponse res) {
            bool ok = res.ok() && res.json() && res.json().unwrap()["ok"].asBool().unwrapOr(false);
            if (ok && uPopupRef) {
                uPopupRef->m_delegate = nullptr;
                uPopupRef->closePopup();
            }
            if (self) self->finishUpgrade(ok, false, upgCost);
            if (ok || !uPopupRef) return;
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

void PetUpgradePopup::onUpgradeLevel(CCObject* sender) {
    if (PetManager::getInstance().m_isLevelUpgraded) return;
    if (m_petData.petLevel < 1 || m_petData.petLevel >= PetManager::kMaxLevel) {
        Notification::create("Your pet level is fully upgraded", NotificationIcon::Success)->show();
        return;
    }

    int upgCost = getUpgradeLvlCostByLevel(m_petData.petLevel);

    if (m_petData.gp < upgCost) {
        Notification::create(
            fmt::format("Not enough Grind Points. Required for upgrade: {}", upgCost),
            NotificationIcon::Error
        )->show();
        return;
    }

    web::WebRequest req;

    matjson::Value body;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;

    body["newLevel"] = m_petData.petLevel + 1;
    body["upgradeCost"] = upgCost;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Upgrading pet level...");
    upopup->show();

    auto uPopupRef = Ref(upopup);
    PetManager::getInstance().m_isLevelUpgraded = true;
    Ref<PetUpgradePopup> self = this;

    m_listener.spawn(
        auth::post("https://api.delivel.tech/v2/upgrade_pet_level", body),
        [uPopupRef, self, upgCost](web::WebResponse res) {
            bool ok = res.ok() && res.json() && res.json().unwrap()["ok"].asBool().unwrapOr(false);
            if (ok && uPopupRef) {
                uPopupRef->m_delegate = nullptr;
                uPopupRef->closePopup();
            }
            if (self) self->finishUpgrade(ok, true, upgCost);
            if (ok || !uPopupRef) return;
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

int PetUpgradePopup::getUpgradeRarityCostByCurrRarity(int rarity) {
    return PetManager::getInstance().getUpgradeRarityCostByCurrRarity(rarity);
}

int PetUpgradePopup::getUpgradeLvlCostByLevel(int petLevel) {
    return PetManager::getInstance().getUpgradeLvlCostByLevel(petLevel);
}

void PetUpgradePopup::finishUpgrade(bool ok, bool level, int cost) {
    if (level) PetManager::getInstance().m_isLevelUpgraded = false;
    else PetManager::getInstance().m_isRarityUpgraded = false;
    if (!ok) return;
    if (level) m_petData.petLevel++;
    else m_petData.petRarity++;
    m_petData.gp = std::max<long long>(0, m_petData.gp - cost);
    auto cb = std::move(m_onUpgraded);
    this->onClose(nullptr);
    if (cb) cb();
}

}