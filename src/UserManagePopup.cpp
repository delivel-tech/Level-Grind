#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <matjson.hpp>
#include "UserManagePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

UserManagePopup* UserManagePopup::create(int accountID, const char* username) {
    auto ret = new UserManagePopup;
    if (ret && ret->init(accountID, username)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool UserManagePopup::init(int targetAccountID, const char* username) {
    if (!Popup::init(240.f, 180.f)) return false;

    m_targetAccountID = targetAccountID;
    m_username = username;

    addSideArt(m_mainLayer, SideArt::All, SideArtStyle::PopupGold, false);

    auto spinner = LoadingSpinner::create(50.f);

    m_mainLayer->addChildAtPosition(spinner, Anchor::Center);

    this->setTitle("Admin: Manage User");

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(ColumnLayout::create());
    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Center);

    matjson::Value body;
	body["account_id"] = targetAccountID;

	web::WebRequest req;
	req.bodyJSON(body);

    Ref<LoadingSpinner> spinnerRef = spinner;
    Ref<UserManagePopup> self = this;
    Ref<CCMenu> btnMenuRef = btnMenu;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/check_helper_manage"),
        [spinnerRef, self, btnMenuRef](web::WebResponse res) {
            if (!self || !spinnerRef) return;
            if (!res.ok()) {
                log::error("failed check helper req");
                spinnerRef->removeFromParent();
                Notification::create("Failed to fetch data!", NotificationIcon::Error)->show();
                return;
            }

            auto json = res.json().unwrapOrDefault();
            auto position = json["pos"].asInt().unwrapOrDefault();

            if (position == 1) {
                spinnerRef->removeFromParent();
                
                auto demoteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Helper", "goldFont.fnt", "GJ_button_02.png"),
                    self,
                    menu_selector(UserManagePopup::onDemoteBtn)
                );
                btnMenuRef->addChild(demoteBtn);
                btnMenuRef->updateLayout();
            } else {
                spinnerRef->removeFromParent();

                if (GJAccountManager::sharedState()->m_accountID != 13678537) {
                    if (position == 2) {
                        Notification::create("User is admin, you cannot manage other admins", NotificationIcon::Info)->show();
                    } else {
                        auto promoteBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Promote Helper"),
                        self,
                        menu_selector(UserManagePopup::onPromoteBtn)
                        );
                        btnMenuRef->addChild(promoteBtn);
                        btnMenuRef->updateLayout();
                    }
                } else {
                    if (position == 0) {
                        auto promoteBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Promote Helper"),
                        self,
                        menu_selector(UserManagePopup::onPromoteBtn)
                        );
                        btnMenuRef->addChild(promoteBtn);
                        btnMenuRef->updateLayout();
                    }
                }
            }

            if (GJAccountManager::sharedState()->m_accountID == 13678537) {
                if (position == 2) {
                    auto demoteAdminBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Demote Admin", "goldFont.fnt", "GJ_button_02.png"),
                        self,
                        menu_selector(UserManagePopup::onDemoteAdminBtn)
                    );
                    btnMenuRef->addChild(demoteAdminBtn);
                    btnMenuRef->updateLayout();
                } else {
                    auto promoteAdminBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Promote Admin"),
                        self,
                        menu_selector(UserManagePopup::onPromoteAdminBtn)
                    );
                    btnMenuRef->addChild(promoteAdminBtn);
                    btnMenuRef->updateLayout();
                }
            }
        }
    );

    return true;
}

void UserManagePopup::onDemoteAdminBtn(CCObject* sender) {
    matjson::Value body;

    auto demoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Demoting admin...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> demoteRef = demoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/demote_admin"),
        [popupRef, demoteRef, self](web::WebResponse res) {
            if (!popupRef || !demoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad demote req");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isDemoted = json["ok"].asBool().unwrapOrDefault();

            if (!isDemoted) {
                log::error("failed to demote");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User demoted");

                auto btnMenu = demoteRef->getParent();

                demoteRef->removeFromParent();

                auto promoteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Admin"),
                    self,
                    menu_selector(UserManagePopup::onPromoteAdminBtn)
                );
                btnMenu->addChild(promoteBtn);
                btnMenu->updateLayout();

                return;
            }
        }
    );
}

void UserManagePopup::onDemoteBtn(CCObject* sender) {
    matjson::Value body;

    auto demoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Demoting helper...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> demoteRef = demoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/demote_helper"),
        [popupRef, demoteRef, self](web::WebResponse res) {
            if (!popupRef || !demoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad demote req");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isDemoted = json["ok"].asBool().unwrapOrDefault();

            if (!isDemoted) {
                log::error("failed to demote");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User demoted");

                auto btnMenu = demoteRef->getParent();

                demoteRef->removeFromParent();

                auto promoteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Helper"),
                    self,
                    menu_selector(UserManagePopup::onPromoteBtn)
                );
                btnMenu->addChild(promoteBtn);
                btnMenu->updateLayout();

                return;
            }
        }
    );
}

void UserManagePopup::onPromoteBtn(CCObject* sender) {
    matjson::Value body;

    auto promoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");
    body["targetUsername"] = m_username;

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Promoting to helper...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> promoteRef = promoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/promote_helper"),
        [popupRef, promoteRef, self](web::WebResponse res) {
            if (!popupRef || !promoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad promote req");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isPromoted = json["ok"].asBool().unwrapOrDefault();

            if (!isPromoted) {
                log::error("failed to promote");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User promoted");
                
                auto btnMenu = promoteRef->getParent();

                promoteRef->removeFromParent();

                auto demoteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Helper", "goldFont.fnt", "GJ_button_02.png"),
                    self,
                    menu_selector(UserManagePopup::onDemoteBtn)
                );
                btnMenu->addChild(demoteBtn);
                btnMenu->updateLayout();

                return;
            }
        }
    );
}

void UserManagePopup::onPromoteAdminBtn(CCObject* sender) {
    matjson::Value body;

    auto promoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");
    body["targetUsername"] = m_username;

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Promoting to admin...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> promoteRef = promoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/promote_admin"),
        [popupRef, promoteRef, self](web::WebResponse res) {
            if (!popupRef || !promoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad promote req");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isPromoted = json["ok"].asBool().unwrapOrDefault();

            if (!isPromoted) {
                log::error("failed to promote");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User promoted");
                
                auto btnMenu = promoteRef->getParent();

                promoteRef->removeFromParent();

                auto demoteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Admin", "goldFont.fnt", "GJ_button_02.png"),
                    self,
                    menu_selector(UserManagePopup::onDemoteAdminBtn)
                );
                btnMenu->addChild(demoteBtn);
                btnMenu->updateLayout();

                return;
            }
        }
    );
}
