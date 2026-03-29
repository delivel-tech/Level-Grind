#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <matjson.hpp>
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

#include "../popups/UserManagePopup.hpp"
#include "../other/LGManager.hpp"
#include "BanPetPopup.hpp"

using namespace geode::prelude;

UserManagePopup* UserManagePopup::create(int accountID, const char* username, int icon, int color1, int color2, int color3) {
    auto ret = new UserManagePopup;
    if (ret && ret->init(accountID, username, icon, color1, color2, color3)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void UserManagePopup::onWipePetDataBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Wiping pet data...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/wipe_pet_data"),
        [popupRef, self](web::WebResponse res) {
            if (!popupRef || !self) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to wipe pet. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to wipe pet. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! Pet data wiped");
            }
        }
    );
}

void UserManagePopup::onBanPetBtn(CCObject* sender) {
    BanPetPopup::create(m_targetAccountID)->show();
}

void UserManagePopup::onUnbanPetBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountId"] = m_targetAccountID;
    body["accountId"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Unbanning pet...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/unban_pet"),
        [popupRef, self](web::WebResponse res) {
            if (!popupRef || !self) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to unban pet. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to unban pet. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! Pet unbanned");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
            }
        }
    );
}

bool UserManagePopup::init(int targetAccountID, const char* username, int icon, int color1, int color2, int color3) {
    if (!Popup::init(350.f, 210.f, "GJ_square02.png")) return false;

    m_targetAccountID = targetAccountID;
    m_username = username;
    m_targetIcon = icon;
    m_targetColor1 = color1;
    m_targetColor2 = color2;
    m_targetColor3 = color3;

    addSideArt(m_mainLayer, SideArt::All, SideArtStyle::PopupBlue, false);

    auto spinner = LoadingSpinner::create(50.f);

    m_mainLayer->addChildAtPosition(spinner, Anchor::Center);

    if (LGManager::get()->isDelivel()) {
        this->setTitle("Owner: Manage User");
    } else {
        this->setTitle("Admin: Manage User");
    }

    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(ColumnLayout::create()->setGap(7)->setGrowCrossAxis(true)->setCrossAxisReverse(true));
    btnMenu->setScale(0.61f);
    btnMenu->setContentHeight(150);
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

            auto jsonRes = res.json();

            if (!jsonRes) {
                log::error("failed check helper req");
                spinnerRef->removeFromParent();
                Notification::create("Failed to fetch data!", NotificationIcon::Error)->show();
                return;
            }

            auto json = jsonRes.unwrapOrDefault();

            bool isAdmin = !json["isAdmin"].asBool().unwrapOrDefault();
            bool isHelper = !json["isHelper"].asBool().unwrapOrDefault();
            bool isCont = !json["isContributor"].asBool().unwrapOrDefault();
            bool isArtist = !json["isArtist"].asBool().unwrapOrDefault();
            bool isBooster = !json["isBooster"].asBool().unwrapOrDefault();
            bool petExists = json["petExists"].asBool().unwrapOrDefault();
            bool isPetBanned = json["isPetBanned"].asBool().unwrapOrDefault();

            spinnerRef->removeFromParent();

            if (LGManager::get()->isDelivel()) {

            if (isAdmin) {
                auto promoteAdminBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Admin", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onPromoteAdminBtn)
                );
                btnMenuRef->addChild(promoteAdminBtn);
            } else {
                auto demoteAdminBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Admin", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onDemoteAdminBtn)
                );
                btnMenuRef->addChild(demoteAdminBtn);
            }

            }

            if (isHelper) {
                auto promoteHelperBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Helper", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onPromoteBtn)
                );
                btnMenuRef->addChild(promoteHelperBtn);
            } else {
                auto demoteHelperBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Helper", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onDemoteBtn)
                );
                btnMenuRef->addChild(demoteHelperBtn);
            }

            if (isCont) {
                auto promoteContBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Contrib", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onPromoteContributorBtn)
                );
                btnMenuRef->addChild(promoteContBtn);
            } else {
                auto demoteContBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Contrib", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onDemoteContributorBtn)
                );
                btnMenuRef->addChild(demoteContBtn);
            }

            if (isArtist) {
                auto promoteArtistBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Artist", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onPromoteArtistBtn)
                );
                btnMenuRef->addChild(promoteArtistBtn);
            } else {
                auto demoteArtistBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Artist", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onDemoteArtistBtn)
                );
                btnMenuRef->addChild(demoteArtistBtn);
            }

            if (isBooster) {
                auto promoteBoosterBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Promote Booster", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onPromoteBoosterBtn)
                );
                btnMenuRef->addChild(promoteBoosterBtn);
            } else {
                auto demoteBoosterBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Demote Booster", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                    self,
                    menu_selector(UserManagePopup::onDemoteBoosterBtn)
                );
                btnMenuRef->addChild(demoteBoosterBtn);
            }

            if (petExists) {
                auto wipeBtnSpr = ButtonSprite::create("Wipe Pet", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                auto wipeBtn = CCMenuItemSpriteExtra::create(
                    wipeBtnSpr,
                    self,
                    menu_selector(UserManagePopup::onWipePetDataBtn)
                );
                btnMenuRef->addChild(wipeBtn);
            } else {
                auto wipeBtnSpr = ButtonSprite::create("Wipe Pet", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                wipeBtnSpr->setColor({ 128, 128, 128 });
                auto wipeBtn = CCMenuItemSpriteExtra::create(
                    wipeBtnSpr,
                    self,
                    menu_selector(UserManagePopup::onWipePetDataBtn)
                );
                wipeBtn->setEnabled(false);
                btnMenuRef->addChild(wipeBtn);
            }

            if (isPetBanned) {
                if (petExists) {
                    auto unbanBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Unban Pet", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f),
                        self,
                        menu_selector(UserManagePopup::onUnbanPetBtn)
                    );
                    btnMenuRef->addChild(unbanBtn);
                } else {
                    auto unbBtnSpr = ButtonSprite::create("Unban Pet", 250.f, true, "goldFont.fnt", "GJ_button_06.png", 30.f, 1.f);
                    unbBtnSpr->setColor({ 128, 128, 128 });

                    auto unbanBtn = CCMenuItemSpriteExtra::create(
                        unbBtnSpr,
                        self,
                        menu_selector(UserManagePopup::onUnbanPetBtn)
                    );
                    unbanBtn->setEnabled(false);
                    btnMenuRef->addChild(unbanBtn);
                }
            } else {
                if (petExists) {
                    auto banBtn = CCMenuItemSpriteExtra::create(
                        ButtonSprite::create("Ban Pet", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f),
                        self,
                        menu_selector(UserManagePopup::onBanPetBtn)
                    );
                    btnMenuRef->addChild(banBtn);
                } else {
                    auto bBtnSpr = ButtonSprite::create("Ban Pet", 250.f, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                    bBtnSpr->setColor({ 128, 128, 128 });

                    auto banBtn = CCMenuItemSpriteExtra::create(
                        bBtnSpr,
                        self,
                        menu_selector(UserManagePopup::onBanPetBtn)
                    );
                    banBtn->setEnabled(false);
                    btnMenuRef->addChild(banBtn);
                }
            }

            btnMenuRef->updateLayout();
        }
    );

    return true;
}

void UserManagePopup::onDemoteAdminBtn(CCObject* sender) {
    matjson::Value body;

    auto demoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

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

void UserManagePopup::onPromoteArtistBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountID"] = m_targetAccountID;
    body["targetUsername"] = m_username;
    body["targetIcon"] = m_targetIcon;
    body["targetColor1"] = m_targetColor1;
    body["targetColor2"] = m_targetColor2;
    body["targetColor3"] = m_targetColor3;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Promoting to artist...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/promote_artist"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to promote. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! User promoted to artist");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
            }
        }
    );
}

void UserManagePopup::onDemoteArtistBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Demoting artist...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/demote_artist"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to demote. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! User demoted from artist");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
            }
        }
    );
}

void UserManagePopup::onPromoteBoosterBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountID"] = m_targetAccountID;
    body["targetUsername"] = m_username;
    body["targetIcon"] = m_targetIcon;
    body["targetColor1"] = m_targetColor1;
    body["targetColor2"] = m_targetColor2;
    body["targetColor3"] = m_targetColor3;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Promoting to booster...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/promote_booster"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to promote. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! User promoted to booster");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
                
            }
        }
    );
}

void UserManagePopup::onDemoteBoosterBtn(CCObject* sender) {
    matjson::Value body;

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Demoting booster...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/demote_booster"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isOk = json["ok"].asBool().unwrapOrDefault();

            if (!isOk) {
                popupRef->showFailMessage("Failed to demote. Try again later");
            } else {
                popupRef->showSuccessMessage("Success! User demoted from booster");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
            }
        }
    );
}

void UserManagePopup::onDemoteBtn(CCObject* sender) {
    matjson::Value body;

    auto demoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

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
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();
                return;
            }
        }
    );
}

void UserManagePopup::onPromoteBtn(CCObject* sender) {
    matjson::Value body;

    auto promoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["targetUsername"] = m_username;
    body["targetIcon"] = m_targetIcon;
    body["targetColor1"] = m_targetColor1;
    body["targetColor2"] = m_targetColor2;
    body["targetColor3"] = m_targetColor3;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

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
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();

                return;
            }
        }
    );
}

void UserManagePopup::onPromoteAdminBtn(CCObject* sender) {
    matjson::Value body;

    auto promoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["targetUsername"] = m_username;
    body["targetIcon"] = m_targetIcon;
    body["targetColor1"] = m_targetColor1;
    body["targetColor2"] = m_targetColor2;
    body["targetColor3"] = m_targetColor3;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

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
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();

                return;
            }
        }
    );
}

void UserManagePopup::onPromoteContributorBtn(CCObject* sender) {
    matjson::Value body;

    auto promoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["targetUsername"] = m_username;
    body["targetIcon"] = m_targetIcon;
    body["targetColor1"] = m_targetColor1;
    body["targetColor2"] = m_targetColor2;
    body["targetColor3"] = m_targetColor3;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Promoting to contributor...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> promoteRef = promoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/promote_contributor"),
        [popupRef, promoteRef, self](web::WebResponse res) {
            if (!popupRef || !promoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad promote contributor req");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isPromoted = json["ok"].asBool().unwrapOrDefault();

            if (!isPromoted) {
                log::error("failed to promote contributor");
                popupRef->showFailMessage("Failed to promote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User promoted");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();

                return;
            }
        }
    );
}

void UserManagePopup::onDemoteContributorBtn(CCObject* sender) {
    matjson::Value body;

    auto demoteBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);

    body["targetAccountID"] = m_targetAccountID;
    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Demoting contributor...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<CCMenuItemSpriteExtra> demoteRef = demoteBtn;
    Ref<UserManagePopup> self = this;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/demote_contributor"),
        [popupRef, demoteRef, self](web::WebResponse res) {
            if (!popupRef || !demoteRef || !self) return;
            if (!res.ok()) {
                log::error("bad demote contributor req");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            }

            auto json = res.json().unwrap();
            auto isDemoted = json["ok"].asBool().unwrapOrDefault();

            if (!isDemoted) {
                log::error("failed to demote contributor");
                popupRef->showFailMessage("Failed to demote. Try again later");
                return;
            } else {
                popupRef->showSuccessMessage("Success! User demoted");
                Notification::create("Please restart User Manage Panel to fetch data.", NotificationIcon::Info)->show();

                return;
            }
        }
    );
}
