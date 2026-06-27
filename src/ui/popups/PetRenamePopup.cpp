#include "PetRenamePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <matjson.hpp>

#include "../../managers/DataManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

PetRenamePopup* PetRenamePopup::create(std::string currPetName) {
    auto ret = new PetRenamePopup;
    if (ret && ret->init(currPetName)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PetRenamePopup::init(std::string currPetName) {
    if (!Popup::init(260.f, 180.f)) return false;

    this->setTitle("Rename Pet");

    m_input = TextInput::create(180.f, currPetName, "bigFont.fnt");
    m_input->setMaxCharCount(20);
    m_input->setID("input");
    
    m_mainLayer->addChildAtPosition(m_input, Anchor::Center);

    auto okBtnSpr = ButtonSprite::create("Rename", "bigFont.fnt", "GJ_button_01.png");
    okBtnSpr->setScale(0.8f);

    auto okBtn = CCMenuItemSpriteExtra::create(
        okBtnSpr,
        this,
        menu_selector(PetRenamePopup::onOKBtn)
    );
    okBtn->setID("ok-btn");

    auto okBtnMenu = CCMenu::create();
    okBtnMenu->setID("ok-btn-menu");
    okBtnMenu->addChild(okBtn);

    m_mainLayer->addChildAtPosition(okBtnMenu, Anchor::Bottom, { 0.f, 30.f });

    return true;
}

void PetRenamePopup::onOKBtn(CCObject* sender) {
    web::WebRequest req;

    matjson::Value body;

    auto upopup = UploadActionPopup::create(nullptr, "Updating pet name...");
    upopup->show();

    body["token"] = DataManager::getInstance().getUserToken();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["petName"] = m_input->getString().c_str();

    req.bodyJSON(body);

    auto uPopupRef = Ref(upopup);
    auto self = Ref(this);

    m_listener.spawn(
        req.post("https://api.delivel.tech/update_pet_name"),
        [uPopupRef, self](web::WebResponse res) {
            if (!uPopupRef || !self) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Updating failed! Try again later.");
                log::error("req failed, code: {}", res.code());
                return;
            }

            auto jsonRes = res.json();

            if (!jsonRes) {
                uPopupRef->showFailMessage("Updating failed! Try again later.");
                log::error("req failed, code: {}", res.code());
                return;
            }

            auto json = jsonRes.unwrap();

            auto isOK = json["ok"].asBool().unwrapOrDefault();

            if (!isOK) {
                uPopupRef->showFailMessage("Updating failed! Try again later.");
                log::error("req failed, code: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Pet name updated.");
            }
            return;
        }
    );
}

}