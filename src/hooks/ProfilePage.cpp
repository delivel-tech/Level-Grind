#include <UIBuilder.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../managers/DataManager.hpp"
#include "../ui/popups/UserManagePopup.hpp"
#include "../ui/popups/GuidePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/utils/cocos.hpp"

#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(GrindProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

        GrindPosition pos = DataManager::getInstance().getUserPosition();
        
        // holy wall of checks lmao
        if (auto child = getChildByIDRecursive("manage-user-btn")) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-owner-badge"_spr)) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-admin-badge"_spr)) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-helper-badge"_spr)) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-contrib-badge"_spr)) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-artist-badge"_spr)) {
            child->removeFromParent();
        }
        if (auto child = getChildByIDRecursive("grind-booster-badge"_spr)) {
            child->removeFromParent();
        }

        if (pos == GrindPosition::Admin || pos == GrindPosition::Owner) {
            auto manageUserBtn = Build<CCSprite>::create("button_add_2.png"_spr)
                .scale(0.55f)
                .intoMenuItem([score] { UserManagePopup::create(score)->show(); })
                .id("manage-user-btn")
                .parent(leftMenu)
                .intoParent()
                .updateLayout();
        }

        auto usernameMenu = this->getChildByIDRecursive("username-menu");
        if (!usernameMenu) return;

        bool hasModBadge = getChildByIDRecursive("mod-badge") != nullptr;

        bool showBadgeForMods = Mod::get()->getSavedValue<bool>("show-badge-for-mods");
        bool shouldSkipStaffBadge = !showBadgeForMods && hasModBadge;
        bool dontShowBadges = Mod::get()->getSavedValue<bool>("disable-badges");

        if (!dontShowBadges && usernameMenu && !shouldSkipStaffBadge) {
            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            if (!score)
                return;

            auto accID = score->m_accountID;

            bool ownerFound =
                std::find(shared.owners.begin(), shared.owners.end(), accID)
                != shared.owners.end();

            bool adminFound =
                std::find(shared.admins.begin(), shared.admins.end(), accID)
                != shared.admins.end();

            bool helperFound =
                std::find(shared.helpers.begin(), shared.helpers.end(), accID)
                != shared.helpers.end();

            bool contFound =
                std::find(shared.contributors.begin(), shared.contributors.end(), accID)
                != shared.contributors.end();

            bool artistFound =
                std::find(shared.artists.begin(), shared.artists.end(), accID)
                != shared.artists.end();

            bool boosterFound =
                std::find(shared.boosters.begin(), shared.boosters.end(), accID)
                != shared.boosters.end();

            if (ownerFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_owner.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::OwnerRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-owner-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }

            if (adminFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_admin.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::AdminRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-admin-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }

            if (helperFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_helper.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::HelperRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-helper-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }

            if (contFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_contributor.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::ContribRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-contrib-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }

            if (artistFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_artist.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::ArtistRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-artist-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }

            if (boosterFound) {
                auto badge = CCMenuItemExt::createSpriteExtra(
                    CCSprite::create("badge_booster.png"_spr),
                    [](CCObject* sender) {
                        GuidePopup::create(GuidePage::BoosterRoleGuide, GuidePopupState::FromOutside)->show();
                    }
                );
                badge->setID("grind-booster-badge"_spr);
                usernameMenu->addChild(badge);
                usernameMenu->updateLayout();
            }
        }
    }
};

}