#include "BanPetPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/TextInput.hpp"

#include "../other/LGManager.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>

using namespace geode::prelude;

BanPetPopup* BanPetPopup::create(int accountId) {
    auto ret = new BanPetPopup;
    if (ret && ret->init(accountId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BanPetPopup::init(int accountId) {
    if (!Popup::init(440.f, 150.f)) return false;

    this->setTitle("Ban Pet");

    m_targetAccountId = accountId;

    addSideArt(
        m_mainLayer, 
        SideArt::All, 
        SideArtStyle::PopupGold, 
        false
    );

    m_input = TextInput::create(400.f, "Write down the reason..", "bigFont.fnt");
    m_input->setMaxCharCount(100);
    m_input->setID("input");

    m_mainLayer->addChildAtPosition(m_input, Anchor::Center, { 0.f, 5.f });

    auto banBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ban Pet"),
        this,
        menu_selector(BanPetPopup::onBanBtn)
    );
    banBtn->setID("ban-btn");

    auto banBtnMenu = CCMenu::create();
    banBtnMenu->setID("ban-btn-menu");
    banBtnMenu->addChild(banBtn);

    m_mainLayer->addChildAtPosition(banBtnMenu, Anchor::Bottom, { 0.f, 30.f });

    return true;
}

void BanPetPopup::onBanBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountId"] = m_targetAccountId;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();
    body["banReason"] = m_input->getString().c_str();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Banning pet...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<BanPetPopup> self = this;

    m_listener.spawn(
        req.post("https://api.delivel.tech/ban_pet"),
        [popupRef, self](web::WebResponse res) {
            if (!popupRef || !self) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to ban pet. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to ban pet. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! Pet banned");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
            }
        }
    );
}