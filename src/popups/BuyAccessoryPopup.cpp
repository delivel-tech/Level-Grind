#include "BuyAccessoryPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
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

BuyAccessoryPopup* BuyAccessoryPopup::create(PetShopLayer::AccessoryInfo *info, PetLayer::PetData petData) {
    auto ret = new BuyAccessoryPopup;
    if (ret && ret->init(info, petData)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BuyAccessoryPopup::init(PetShopLayer::AccessoryInfo* info, PetLayer::PetData petData) {
    if (!Popup::init({ 260.f, 190.f }, "GJ_square02.png")) return false;

    m_petData = petData;
    m_accessoryInfo = info;

    m_closeBtn->removeFromParent();

    m_price = PetShopLayer::getPriceWithDiscount(m_accessoryInfo);

    this->setTitle(
        fmt::format(
            "Buy {} for {} pet moons",
            m_accessoryInfo->accessoryName,
            m_price
        )
    );

    auto petSpr = PetLayer::getPetSprByStyle(
        PetLayer::getStyleByLevel(m_petData.petLevel)
    );

    float yAccessoryOffset = m_accessoryInfo->aPosition.height - 22.f;
    float xAccessoryOffset = m_accessoryInfo->aPosition.width - 30.f;

    m_mainLayer->addChildAtPosition(
        petSpr, Anchor::Center
    );

    auto accessorySpr = CCSprite::create(m_accessoryInfo->fileName.c_str());
    accessorySpr->setScale(m_accessoryInfo->aScale);
    m_mainLayer->addChildAtPosition(
        accessorySpr,
        Anchor::Center,
        { 0.f + xAccessoryOffset, 22.f + yAccessoryOffset }
    );

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(RowLayout::create()->setGap(15));

    auto cancelBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Cancel", "goldFont.fnt", "GJ_button_06.png"),
        [this](CCObject* sender) {
            this->onClose(sender);
        }
    );

    auto buyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Buy"),
        this,
        menu_selector(BuyAccessoryPopup::onBuyBtn)
    );

    m_mainLayer->addChildAtPosition(
        btnMenu,
        Anchor::Bottom,
        { 0.f, 25.f }
    );

    btnMenu->addChild(cancelBtn);
    btnMenu->addChild(buyBtn);

    btnMenu->updateLayout();

    return true;
}

void BuyAccessoryPopup::onBuyBtn(CCObject* sender) {
    if (PetManager::get()->isItemBought) {
        Notification::create("Please re-open pet layer to sync data.", NotificationIcon::Info)->show();
        return;
    }
    PetManager::get()->isItemBought = true;
    if (m_petData.petMoons < m_price) {
        Notification::create(fmt::format("Not enough moons. Required: {}", m_price), NotificationIcon::Error)->show();
        this->onClose(sender);
        return;
    }

    auto req = web::WebRequest();

    matjson::Value body;

    body["accountId"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();
    body["itemId"] = m_accessoryInfo->aid;
    body["price"] = m_price;

    req.bodyJSON(body);

    auto uPopup = UploadActionPopup::create(nullptr, fmt::format("Buying {}...", m_accessoryInfo->accessoryName));
    uPopup->show();
    
    auto uPopupRef = Ref(uPopup);

    m_listener.spawn(
        req.post("https://api.delivel.tech/buy_item"),
        [uPopupRef](web::WebResponse res) {
            if (!uPopupRef) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Buying item failed! Try again later.");
                log::error("req failed, error code: {}", res.code());
                return;
            }

            auto jsonRes = res.json();

            if (!jsonRes) {
                uPopupRef->showFailMessage("Buying item failed! Try again later.");
                log::error("req failed, error code: {}", res.code());
                return;
            }

            auto json = jsonRes.unwrapOrDefault();

            bool isOK = json["ok"].asBool().unwrapOrDefault();

            if (!isOK) {
                uPopupRef->showFailMessage("Buying item failed! Try again later.");
                log::error("req failed, error code: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Item bought.");
                Notification::create("Please re-open pet layer to sync data.", NotificationIcon::Info)->show();
                return;
            }
        }
    );
}