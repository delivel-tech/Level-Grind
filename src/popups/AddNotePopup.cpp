#include "AddNotePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/TextInput.hpp"

#include "../other/LGManager.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/CCTextInputNode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>

using namespace geode::prelude;

AddNotePopup* AddNotePopup::create(int levelId, std::string levelName) {
    auto ret = new AddNotePopup;
    if (ret && ret->init(levelId, levelName)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AddNotePopup::init(int levelId, std::string levelName) {
    if (!Popup::init(440.f, 150.f)) return false;

    this->setTitle("Add Note");

    m_levelId = levelId;
    m_levelName = levelName;

    addSideArt(
        m_mainLayer, 
        SideArt::All, 
        SideArtStyle::PopupGold, 
        false
    );

    m_input = TextInput::create(400.f, "Write down your note..", "bigFont.fnt");
    m_input->setMaxCharCount(255);
    m_input->setID("input");
    m_input->setCommonFilter(CommonFilter::Any);

    m_mainLayer->addChildAtPosition(m_input, Anchor::Center, { 0.f, 5.f });

    auto addBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add Note"),
        this,
        menu_selector(AddNotePopup::onAddBtn)
    );
    addBtn->setID("add-btn");

    auto addBtnMenu = CCMenu::create();
    addBtnMenu->setID("add-btn-menu");
    addBtnMenu->addChild(addBtn);

    m_mainLayer->addChildAtPosition(addBtnMenu, Anchor::Bottom, { 0.f, 30.f });

    return true;
}

void AddNotePopup::onAddBtn(CCObject* sender) {
    web::WebRequest req;

    matjson::Value body;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();
    body["levelID"] = m_levelId;
    body["levelName"] = m_levelName;
    body["note"] = m_input->getString().c_str();
    body["addedBy"] = GJAccountManager::sharedState()->m_username.c_str();

    req.bodyJSON(body);

    auto uPopup = UploadActionPopup::create(nullptr, "Adding note...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/new_note"),
        [uPopupRef](web::WebResponse res) {
            if (!uPopupRef) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Failed! Try again later.");
                log::error("req failed, code: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Note added.");
                return;
            }
        }
    );
}