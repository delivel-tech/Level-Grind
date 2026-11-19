#include "UserManagePopup.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/Auth.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"
#include "RoleSelectorPopup.hpp"
#include "WeeklyAchievementConfigurePopup.hpp"
#include "../StaffIcons.hpp"

using namespace geode::prelude;

const int kButtonWidth = 250.f;

namespace levelgrind {

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
            this->setTitle("Grind Coordinator: Ban Pet");
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
        this->setTitle("Grind Coordinator: Manage User");
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

    m_optionsMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos(centerX(), centerY() - 23)
        .id("options-menu")
        .scale(0.9f)
        .contentSize(m_mainLayer->getContentWidth() - 50.f, 80)
        .layout(ColumnLayout::create()->setGap(6.f)->setAxisReverse(true)->setAutoScale(false))
        .zOrder(1)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize({ m_optionsMenu->getContentWidth() + 5, m_optionsMenu->getContentHeight() + 10 })
        .pos(centerX(), centerY() - 20)
        .id("menu-bg")
        .scale(0.9f)
        .opacity(80)
        .parent(m_mainLayer);

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
    if (roles.isOwner) return CCSprite::createWithSpriteFrameName("badge_owner.png"_spr);
    else if (roles.isAdmin) return CCSprite::createWithSpriteFrameName("badge_admin.png"_spr);
    else if (roles.isHelper) return CCSprite::createWithSpriteFrameName("badge_helper.png"_spr);
    else if (roles.isArtist) return CCSprite::createWithSpriteFrameName("badge_artist.png"_spr);
    else if (roles.isBooster) return CCSprite::createWithSpriteFrameName("badge_booster.png"_spr);
    else if (roles.isContributor) return CCSprite::createWithSpriteFrameName("badge_contributor.png"_spr);

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

            self->m_petExists = parsed.petExists;
            self->m_petBanned = parsed.isPetBanned;
            self->m_targetLabelMenu->updateLayout();
            self->buildActions();

            self->m_spinner->removeFromParent();
            self->loadModeration();
            return;
        }
    );
}

void UserManagePopup::loadModeration() {
    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["targetAccountId"] = m_targetUser->m_accountID;
    Ref<UserManagePopup> self = this;
    m_modTask.spawn(auth::post(fmt::format("{}/staff_user_status", APIClient::getInstance().baseUrl), std::move(body)), [self](web::WebResponse res) {
        if (!self) return;
        auto json = res.json();
        if (!res.ok() || !json) return;
        auto const& v = json.unwrap();
        self->m_clanBan = v["clanBan"].asBool().unwrapOr(false);
        self->m_lbBan = v["lbBan"].asBool().unwrapOr(false);
        self->buildActions();
    });
}

void UserManagePopup::wipePet() {
    Ref<UserManagePopup> self = this;
    createQuickPopup(
        "Wipe Pet",
        fmt::format("Wipe the pet of <cy>{}</c>?", std::string(m_targetUser->m_userName)),
        "Cancel", "Wipe",
        [self](auto, bool yes) {
            if (!yes || !self) return;
            auto uPopup = UploadActionPopup::create(nullptr, "Wiping pet...");
            uPopup->show();
            Ref<UploadActionPopup> uPopupRef = uPopup;
            self->m_listener.spawn(
                APIClient::getInstance().wipePet(self->m_targetUser->m_accountID),
                [uPopupRef](web::WebResponse res) {
                    if (!uPopupRef) return;
                    auto parsed = APIClient::getInstance().wipePetParse(res);
                    if (parsed.ok) uPopupRef->showSuccessMessage("Success! Pet wiped.");
                    else uPopupRef->showFailMessage("Failed! Try again later");
                }
            );
        }
    );
}

void UserManagePopup::unbanPet() {
    auto uPopup = UploadActionPopup::create(nullptr, "Unbanning pet...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<UserManagePopup> self = this;
    m_listener.spawn(
        APIClient::getInstance().unbanPet(m_targetUser->m_accountID),
        [uPopupRef, self](web::WebResponse res) {
            if (!uPopupRef) return;
            auto parsed = APIClient::getInstance().unbanPetParse(res);
            if (parsed.ok) {
                uPopupRef->showSuccessMessage("Success! Pet unbanned.");
                if (self) {
                    self->m_petBanned = false;
                    self->buildActions();
                }
            } else {
                uPopupRef->showFailMessage("Failed! Try again later");
            }
        }
    );
}

void UserManagePopup::buildActions() {
    if (!m_optionsMenu) return;
    m_optionsMenu->removeAllChildren();
    Ref<UserManagePopup> self = this;
    int target = m_targetUser->m_accountID;
    std::string name = m_targetUser->m_userName;

    std::vector<CCMenuItemSpriteExtra*> items;
    auto row = [&](const char* icon, bool active, geode::Function<void()> action) {
        auto spr = staffIconButton(icon, active);
        items.push_back(CCMenuItemExt::createSpriteExtra(spr, [action = std::move(action)](CCObject*) mutable { action(); }));
    };

    auto ban = [self, target, name](std::string kind, bool on, std::string label) {
        createQuickPopup(
            on ? "Ban" : "Unban",
            fmt::format("{} <cy>{}</c> {} {}?", on ? "Ban" : "Unban", name, on ? "from" : "on", label),
            "Cancel", on ? "Ban" : "Unban",
            [self, target, kind, on](auto, bool yes) {
                if (!yes || !self) return;
                matjson::Value body;
                body["accountId"] = GJAccountManager::get()->m_accountID;
                body["targetAccountId"] = target;
                body["kind"] = kind;
                body["ban"] = on;
                self->moderate("staff_ban", std::move(body), on ? "Banning..." : "Unbanning...", on ? "Banned." : "Unbanned.");
            }
        );
    };

    row("weekly_ach_staff_icon.png"_spr, false, [self] {
        if (self) WeeklyAchievementConfigurePopup::create(self->m_targetUser)->show();
    });
    if (m_petExists) {
        if (m_petBanned) {
            row("ban_pet_staff_icon.png"_spr, true, [self] { if (self) self->unbanPet(); });
        } else {
            row("ban_pet_staff_icon.png"_spr, false, [self, target] { if (self) BanPetPopup::create(target)->show(); });
        }
        row("reset_pet_name_staff_icon.png"_spr, false, [self, target, name] {
            createQuickPopup("Reset Pet Name", fmt::format("Reset the pet name of <cy>{}</c>?", name), "Cancel", "Reset",
                [self, target](auto, bool yes) {
                    if (!yes || !self) return;
                    matjson::Value body;
                    body["accountId"] = GJAccountManager::get()->m_accountID;
                    body["targetAccountId"] = target;
                    self->moderate("staff_reset_pet_name", std::move(body), "Resetting...", "Pet name reset.");
                });
        });
        row("clear_pet_staff_icon.png"_spr, false, [self] { if (self) self->wipePet(); });
    }
    row("ban_clan_staff_icon.png"_spr, m_clanBan, [ban, on = !m_clanBan] { ban("clans", on, "Grind Clans"); });
    row("lb_ban_staff_icon.png"_spr, m_lbBan, [ban, on = !m_lbBan] { ban("leaderboards", on, "leaderboards"); });

    for (size_t i = 0; i < items.size(); i += 4) {
        auto line = CCMenu::create();
        line->setLayout(RowLayout::create()->setGap(7.f)->setAutoScale(false));
        line->setContentSize({ m_optionsMenu->getContentWidth(), 36.f });
        for (size_t j = i; j < std::min(items.size(), i + 4); j++) line->addChild(items[j]);
        line->updateLayout();
        m_optionsMenu->addChild(line);
    }
    m_optionsMenu->updateLayout();
    handleTouchPriority(this);
}

void UserManagePopup::moderate(std::string endpoint, matjson::Value body, std::string loading, std::string success) {
    auto popup = UploadActionPopup::create(nullptr, loading);
    popup->show();
    Ref<UploadActionPopup> popupRef = popup;
    Ref<UserManagePopup> self = this;
    m_modTask.spawn(auth::post(fmt::format("{}/{}", APIClient::getInstance().baseUrl, endpoint), std::move(body)),
        [self, popupRef, success](web::WebResponse res) mutable {
            auto json = res.json();
            bool ok = res.ok() && json && json.unwrap()["ok"].asBool().unwrapOr(false);
            if (popupRef) {
                if (ok) popupRef->showSuccessMessage(success);
                else popupRef->showFailMessage(APIClient::errorMessage(res, "Failed! Try again later."));
            }
            if (ok && self) self->loadModeration();
        });
}

}