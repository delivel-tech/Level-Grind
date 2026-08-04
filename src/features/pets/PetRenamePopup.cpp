#include "PetRenamePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/async.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>

#include "../../core/BackendManager.hpp"

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
    async::spawn(this->onRename(m_input->getString()));
}

arc::Future<> PetRenamePopup::onRename(std::string petName) {
    auto upopup = UploadActionPopup::create(nullptr, "Updating pet name...");
    upopup->show();

    auto uPopupRef = Ref(upopup);
    Ref<PetRenamePopup> self = this;

    auto parsed = co_await BackendManager::getInstance().renamePet(petName);

    if (!uPopupRef || !self) co_return;

    if (!parsed.ok) {
        uPopupRef->showFailMessage("Updating failed! Try again later.");
        co_return;
    }

    uPopupRef->showSuccessMessage("Success! Pet name updated.");
    co_return;
}

}