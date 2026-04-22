#include "UserManagePopup.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"
#include "RoleSelectorPopup.hpp"

using namespace geode::prelude;

const int kButtonWidth = 250.f;

namespace levelgrind {

// Ban Pet Popup class is here because it's only used in this file and it's easier to manage it here

class BanPetPopup : public BasePopup {
public:
    static BanPetPopup* create(int accountID) {
        auto ret = new BanPetPopup;
        if (ret && ret->init(accountID)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(int accountID) {
        if (!BasePopup::init(300.f, 120.f, "GJ_square01.png")) return false;

        DataManager::getInstance().getUserPosition();

        if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin) {
            this->setTitle("Grind Admin: Ban Pet");
        } else if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
            this->setTitle("Grind Owner: Ban Pet");
        } else {
            this->setTitle("Unknown Role: Ban Pet");
        }

        auto textInput = Build(TextInput::create(270.f, "Write down the reason...", "chatFont.fnt"))
            .with([](TextInput* input) {
                input->setMaxCharCount(60);
            })
            .id("input")
            .pos(fromCenter(0, 5))
            .parent(m_mainLayer)
            .collect();

        auto banBtnMenu = Build(ButtonSprite::create("Ban Pet", "bigFont.fnt", "GJ_button_01.png"))
            .id("ban-btn")
            .scale(0.7f)
            .intoMenuItem([accountID, textInput, this] {
                std::string reason = textInput->getString();
                if (reason.empty()) {
                    Notification::create("Please provide a reason for banning the pet.", NotificationIcon::Error)->show();
                    return;
                }

                auto uPopup = UploadActionPopup::create(nullptr, "Banning pet...");
                uPopup->show();

                auto uPopupRef = Ref(uPopup);

                m_listener.spawn(
                    APIClient::getInstance().banPet(accountID, reason),
                    [uPopupRef](web::WebResponse res) {
                        if (!uPopupRef) return;
                        auto parsed = APIClient::getInstance().banPetParse(res);
                        if (parsed.ok) {
                            uPopupRef->showSuccessMessage("Success! Pet banned.");
                        } else {
                            uPopupRef->showFailMessage("Failed! Try again later");
                        }
                    }
                );
            })
            .parent(CCMenu::create())
            .intoParent()
            .id("ban-btn-menu")
            .parent(m_mainLayer)
            .pos(fromBottom({0, 25}))
            .collect();

        return true;
    }

    TaskHolder<web::WebResponse> m_listener;
};

UserManagePopup* UserManagePopup::create(GJUserScore *targetUser) {
    auto ret = new UserManagePopup;
    if (ret && ret->init(targetUser)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool UserManagePopup::init(GJUserScore* targetUser) {
    if (!BasePopup::init(270.f, 160.f, "GJ_square01.png")) return false;

    m_targetUser = targetUser;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Manage User");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Owner: Manage User");
    } else {
        this->setTitle("Unknown Role: Manage User");
    }

    if (pos != GrindPosition::Admin && pos != GrindPosition::Owner) {
        Notification::create("You cannot manage profiles.", NotificationIcon::Error)->show();
    }

    auto targetLabelMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos({m_title->getPositionX(), m_title->getPositionY() - 20 })
        .id("target-label-menu")
        .scale(0.9f)
        .contentSize(m_mainLayer->getContentWidth() - 20.f, 30.f)
        .layout(
            RowLayout::create()
                ->setGap(6)
                ->setAutoScale(false)
        )
        .zOrder(1)
        .collect();

    auto targetLabel = Build<CCLabelBMFont>::create(
        fmt::format(
            "Target: {}", targetUser->m_userName
        ).c_str(), "bigFont.fnt", m_mainLayer->getContentSize().width - 40, kCCTextAlignmentCenter
    )
        .id("target-label")
        .scale(0.5f)
        .parent(targetLabelMenu)
        .collect();

    m_targetLabel = targetLabel;
    m_targetLabelMenu = targetLabelMenu;

    m_targetLabelMenu->updateLayout();

    auto optionsMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos(centerX(), centerY() - 20)
        .id("options-menu")
        .scale(0.9f)
        .contentSize(m_mainLayer->getContentWidth() - 50.f, 80)
        .layout(
            RowLayout::create()
                    ->setGap(7)
                    ->setGrowCrossAxis(true)
                    ->setCrossAxisReverse(true)
        )
        .zOrder(1)
        .collect();

    m_optionsMenu = optionsMenu;

    auto menuBg = Build(NineSlice::create("square02_small.png"))
        .contentSize({ optionsMenu->getContentWidth() + 5,
        optionsMenu->getContentHeight() + 10 })
        .pos(centerX(), centerY() - 20)
        .id("menu-bg")
        .scale(0.9f)
        .opacity(80)
        .parent(m_mainLayer)
        .collect();

    auto spinner = Build(LoadingSpinner::create(55))
        .pos(centerX(), centerY() - 20)
        .scale(0.9f)
        .parent(m_mainLayer)
        .collect();

    m_spinner = spinner;

    buildUI();

    return true;
}

CCSprite* UserManagePopup::getBadgeByHighestRole(UserRoles roles) {
    if (roles.isOwner) {
        return CCSprite::create("badge_owner.png"_spr);
    } else if (roles.isAdmin) {
        return CCSprite::create("badge_admin.png"_spr);
    } else if (roles.isHelper) {
        return CCSprite::create("badge_helper.png"_spr);
    } else if (roles.isArtist) {
        return CCSprite::create("badge_artist.png"_spr);
    } else if (roles.isBooster) {
        return CCSprite::create("badge_booster.png"_spr);
    } else if (roles.isContributor) {
        return CCSprite::create("badge_contributor.png"_spr);
    }
    return CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png");
}

void UserManagePopup::buildUI() {
    auto self = Ref(this);
    m_listener.spawn(
        APIClient::getInstance().getUserRoles(m_targetUser->m_accountID),
        [self](web::WebResponse res) {
            if (!self) return;
            auto parsed = APIClient::getInstance().getUserRolesParse(res);
            if (!parsed.ok) {
                Notification::create("Failed to get user roles!", NotificationIcon::Error)->show();
                self->m_spinner->removeFromParent();
                return;
            }

            auto badge = self->getBadgeByHighestRole(parsed.roles);
            if (badge) {
                auto badgeNode = Build(badge)
                    .scale(0.7f)
                    .intoMenuItem([roles = parsed.roles, targetUser = self->m_targetUser] {
                        auto popup = RoleSelectorPopup::create(roles, targetUser);
                        popup->show();
                    })
                    .parent(self->m_targetLabelMenu)
                    .id("role-badge")
                    .collect();
            }

            bool petExists = parsed.petExists;
            bool isPetBanned = parsed.isPetBanned;

            if (petExists) {
                auto wipePetBtn = Build(CCSprite::createWithSpriteFrameName("GJ_deleteServerBtn_001.png"))
                    .parent(self->m_optionsMenu)
                    .id("wipe-pet-btn")
                    .intoMenuItem([self] {
                        
                        createQuickPopup(
                            "Are you sure?",
                            fmt::format("Please confirm your decision to wipe the {}'s pet.", self->m_targetUser->m_userName),
                            "Cancel", "Confirm",
                            [self](auto, bool btn2) {
                                if (btn2) {
                                    auto uPopup = UploadActionPopup::create(nullptr, "Wiping pet...");
                                    uPopup->show();

                                    auto uPopupRef = Ref(uPopup);

                                    self->m_listener.spawn(
                                        APIClient::getInstance().wipePet(self->m_targetUser->m_accountID),
                                        [uPopupRef](web::WebResponse res) {
                                            if (!uPopupRef) return;
                                            auto parsed = APIClient::getInstance().wipePetParse(res);
                                            if (parsed.ok) {
                                                uPopupRef->showSuccessMessage("Success! Pet wiped.");
                                            } else {
                                                uPopupRef->showFailMessage("Failed! Try again later");
                                            }
                                        }
                                    );
                                }
                            }
                        );
                    })
                    .collect();
            } else {
                auto wipePetBtn = Build(CCSprite::createWithSpriteFrameName("GJ_deleteServerBtn_001.png"))
                    .parent(self->m_optionsMenu)
                    .id("wipe-pet-btn")
                    .color({ 100, 100, 100 })
                    .intoMenuItem([] {
                    })
                    .enabled(false)
                    .collect();
            }

            if (isPetBanned) {
                if (petExists) {
                    auto unbanBtn = Build<CCSprite>::createSpriteName("GJ_cancelDownloadBtn_001.png")
                        .parent(self->m_optionsMenu)
                        .id("unban-pet-btn")
                        .scale(1.2f)
                        .intoMenuItem([self] {
                            auto uPopup = UploadActionPopup::create(nullptr, "Unbanning pet...");
                            uPopup->show();

                            auto uPopupRef = Ref(uPopup);

                            self->m_listener.spawn(
                                APIClient::getInstance().unbanPet(self->m_targetUser->m_accountID),
                                [uPopupRef](web::WebResponse res) {
                                    if (!uPopupRef) return;
                                    auto parsed = APIClient::getInstance().unbanPetParse(res);
                                    if (parsed.ok) {
                                        uPopupRef->showSuccessMessage("Success! Pet unbanned.");
                                    } else {
                                        uPopupRef->showFailMessage("Failed! Try again later");
                                    }
                                }
                            );
                        })
                        .collect();
                } else {
                    auto unbanBtn = Build<CCSprite>::createSpriteName("GJ_cancelDownloadBtn_001.png")
                        .parent(self->m_optionsMenu)
                        .id("unban-pet-btn")
                        .scale(1.2f)
                        .color({ 100, 100, 100 })
                        .intoMenuItem([] {
                        })
                        .enabled(false)
                        .collect();
                }
            } else {
                if (petExists) {
                    auto banBtn = Build<CCSprite>::createSpriteName("GJ_deleteBtn_001.png")
                        .parent(self->m_optionsMenu)
                        .id("ban-pet-btn")
                        .intoMenuItem([self] {
                            BanPetPopup::create(self->m_targetUser->m_accountID)->show();
                        })
                        .collect();
                } else {
                    auto banBtn = Build<CCSprite>::createSpriteName("GJ_deleteBtn_001.png")
                        .parent(self->m_optionsMenu)
                        .id("ban-pet-btn")
                        .color({ 100, 100, 100 })
                        .intoMenuItem([] {
                        })
                        .enabled(false)
                        .collect();
                }
            }

            self->m_optionsMenu->updateLayout();
            self->m_targetLabelMenu->updateLayout();

            self->m_spinner->removeFromParent();
            return;
        }
    );
}

}