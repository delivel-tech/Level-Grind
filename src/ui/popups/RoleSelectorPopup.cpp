#include <Geode/Geode.hpp>
#include "RoleSelectorPopup.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include "../../utils/utils.hpp"
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

namespace levelgrind {

RoleSelectorPopup* RoleSelectorPopup::create(UserRoles roles, GJUserScore* targetUser) {
    auto ret = new RoleSelectorPopup;
    if (ret && ret->init(roles, targetUser)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;

}

bool RoleSelectorPopup::init(UserRoles roles, GJUserScore* targetUser) {
    if (!BasePopup::init({ 320.f, 120.f } )) return false;

    m_newRoles.isAdmin = roles.isAdmin;
    m_newRoles.isOwner = roles.isOwner;
    m_newRoles.isHelper = roles.isHelper;
    m_newRoles.isArtist = roles.isArtist;
    m_newRoles.isBooster = roles.isBooster;
    m_newRoles.isContributor = roles.isContributor;

    m_targetUser = targetUser;

    auto pos = DataManager::getInstance().getUserPosition();
    if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Owner: Set Roles");
    } else if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Set Roles");
    } else {
        this->setTitle("Unknown Role: Set Roles");
    }

    if (pos != GrindPosition::Owner && pos != GrindPosition::Admin) {
        Notification::create("You don't have permission to set roles!", NotificationIcon::Error)->show();
        return false;
    }

    auto menuBg = Build(NineSlice::create("square02_small.png"))
        .contentSize({ m_mainLayer->getContentWidth() - 15,
        (m_mainLayer->getContentHeight() - 20) / 2 })
        .pos(centerX(), centerY())
        .id("menuBg")
        .scale(0.9f)
        .opacity(120)
        .parent(m_mainLayer)
        .collect();

    auto badgesMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos(centerX(), centerY())
        .id("badges-menu")
        .scale(0.9f)
        .contentSize(menuBg->getContentSize().width - 10.f, menuBg->getContentSize().height - 10.f)
        .layout(
            RowLayout::create()
                ->setGap(5)
                ->setCrossAxisReverse(true)
        )
        .zOrder(1);

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        auto badgeAdmin = CCSprite::create("badge_admin.png"_spr);
        auto badgeAdminOff = CCSprite::create("badge_admin.png"_spr);
        badgeAdminOff->setColor({ 100, 100, 100 });

        auto badgeAdminBtn = Build(CCMenuItemToggler::create(
            badgeAdminOff, badgeAdmin, this, menu_selector(RoleSelectorPopup::onAdminToggle)
        ))
            .parent(badgesMenu)
            .id("admin-toggle-btn")
            .collect();

        if (roles.isAdmin) badgeAdminBtn->toggle(true);
    }

    auto badgeHelper = CCSprite::create("badge_helper.png"_spr);
    auto badgeHelperOff = CCSprite::create("badge_helper.png"_spr);
    badgeHelperOff->setColor({ 100, 100, 100 });

    auto badgeHelperBtn = Build(CCMenuItemToggler::create(
        badgeHelperOff, badgeHelper, this, menu_selector(RoleSelectorPopup::onHelperToggle)
    ))
        .parent(badgesMenu)
        .id("helper-toggle-btn")
        .collect();

    auto badgeArtist = CCSprite::create("badge_artist.png"_spr);
    auto badgeArtistOff = CCSprite::create("badge_artist.png"_spr);
    badgeArtistOff->setColor({ 100, 100, 100 });

    auto badgeArtistBtn = Build(CCMenuItemToggler::create(
        badgeArtistOff, badgeArtist, this, menu_selector(RoleSelectorPopup::onArtistToggle)
    ))
        .parent(badgesMenu)
        .id("artist-toggle-btn")
        .collect();

    auto badgeContributor = CCSprite::create("badge_contributor.png"_spr);
    auto badgeContributorOff = CCSprite::create("badge_contributor.png"_spr);
    badgeContributorOff->setColor({ 100, 100, 100 });

    auto badgeContributorBtn = Build(CCMenuItemToggler::create(
        badgeContributorOff, badgeContributor, this, menu_selector(RoleSelectorPopup::onContributorToggle)
    ))
        .parent(badgesMenu)
        .id("contributor-toggle-btn")
        .collect();

    auto badgeBooster = CCSprite::create("badge_booster.png"_spr);
    auto badgeBoosterOff = CCSprite::create("badge_booster.png"_spr);
    badgeBoosterOff->setColor({ 100, 100, 100 });

    auto badgeBoosterBtn = Build(CCMenuItemToggler::create(
        badgeBoosterOff, badgeBooster, this, menu_selector(RoleSelectorPopup::onBoosterToggle)
    ))
        .parent(badgesMenu)
        .id("booster-toggle-btn")
        .collect();

    if (roles.isHelper) badgeHelperBtn->toggle(true);
    if (roles.isArtist) badgeArtistBtn->toggle(true);
    if (roles.isContributor) badgeContributorBtn->toggle(true);
    if (roles.isBooster) badgeBoosterBtn->toggle(true);

    badgesMenu.updateLayout();

    auto buttonsMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos(fromCenter(0, -40))
        .id("buttons-menu")
        .scale(0.65f)
        .contentSize(m_mainLayer->getContentWidth() - 15.f, 40.f)
        .layout(
            RowLayout::create()
                ->setGap(5)
        )
        .zOrder(1)
        .collect();

    auto applyBtn = Build(ButtonSprite::create("Apply", "bigFont.fnt", "GJ_button_01.png"))
        .pos(fromBottom(10.f))
        .parent(buttonsMenu)
        .id("apply-btn")
        .intoMenuItem([this]() {
            auto uPopup = UploadActionPopup::create(nullptr, "Updating roles...");
            uPopup->show();

            auto uPopupRef = Ref(uPopup);

            m_listener.spawn(
                APIClient::getInstance().setRoles(
                    APIClient::getInstance().makeSetRolesBody(
                        GJAccountManager::sharedState()->m_accountID,
                        DataManager::getInstance().getUserToken(),
                        m_targetUser->m_accountID,
                        std::string(m_targetUser->m_userName),
                        m_targetUser->m_playerCube,
                        m_targetUser->m_color1,
                        m_targetUser->m_color2,
                        m_targetUser->m_color3,
                        m_newRoles.isAdmin,
                        m_newRoles.isHelper,
                        m_newRoles.isArtist,
                        m_newRoles.isContributor,
                        m_newRoles.isBooster
                    )
                ),
                [uPopupRef](web::WebResponse res) {
                    if (!uPopupRef) return;
                    auto parsed = APIClient::getInstance().setRolesParse(res);
                    if (parsed.ok) {
                        uPopupRef->showSuccessMessage("Success! Roles updated.");
                    } else {
                        uPopupRef->showFailMessage("Failed! Try again later.");
                    }
                }
            );
        })
        .collect();

    buttonsMenu->updateLayout();

    return true;
}

void RoleSelectorPopup::onAdminToggle(CCObject* sender) {
	bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        m_newRoles.isAdmin = true;
    } else {
        m_newRoles.isAdmin = false;
    }
}

void RoleSelectorPopup::onHelperToggle(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        m_newRoles.isHelper = true;
    } else {
        m_newRoles.isHelper = false;
    }
}

void RoleSelectorPopup::onArtistToggle(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        m_newRoles.isArtist = true;
    } else {
        m_newRoles.isArtist = false;
    }
}

void RoleSelectorPopup::onBoosterToggle(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        m_newRoles.isBooster = true;
    } else {
        m_newRoles.isBooster = false;
    }

}

void RoleSelectorPopup::onContributorToggle(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        m_newRoles.isContributor = true;
    } else {
        m_newRoles.isContributor = false;
    }

}

}