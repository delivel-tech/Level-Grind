#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <UIBuilder.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../core/DataManager.hpp"
#include "../features/admin/UserManagePopup.hpp"
#include "../core/GuidePopup.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/function.hpp"

#include <Geode/modify/ProfilePage.hpp>

#include "../core/GuidePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class $modify(GrindProfilePage, ProfilePage) {
    void fixBadges() {
        if (auto badge = getChildByIDRecursive("owner-badge"_spr)) badge->removeFromParent();
        if (auto badge = getChildByIDRecursive("admin-badge"_spr)) badge->removeFromParent();
        if (auto badge = getChildByIDRecursive("helper-badge"_spr)) badge->removeFromParent();
        if (auto badge = getChildByIDRecursive("artist-badge"_spr)) badge->removeFromParent();
        if (auto badge = getChildByIDRecursive("contrib-badge"_spr)) badge->removeFromParent();
        if (auto badge = getChildByIDRecursive("booster-badge"_spr)) badge->removeFromParent();
    }
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        this->fixBadges();

        auto mod = Mod::get();

        bool hideBadges = mod->getSavedValue<bool>("disable-badges");
        bool hideBadgesForMods = mod->getSavedValue<bool>("hide-badges-for-mods");

        auto usernameMenu = this->m_mainLayer->getChildByID("username-menu");

        if (!usernameMenu) return;

        bool hasModBadge = usernameMenu->getChildByID("mod-badge");

        bool showBadge = [hideBadges, hideBadgesForMods, hasModBadge] {
            if (hideBadges || (hideBadgesForMods && hasModBadge)) {
                return false;
            } else {
                return true;
            }
        }();

        struct badgeInfo {
            std::string spriteName = "";
            std::string badgeID = "";
            geode::Function<void(CCMenuItemSpriteExtra*)> callback;
        };

        auto buildBadge = [usernameMenu](badgeInfo Info) {
            Build(CCMenuItemExt::createSpriteExtra(
                CCSprite::create(Info.spriteName.c_str()),
                std::move(Info.callback)
            ))
                .parent(usernameMenu)
                .id(Info.badgeID);
        };

        badgeInfo ownerInfo{
            .spriteName = "badge_owner.png"_spr,
            .badgeID = "owner-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) { 
                GuidePopup::create(GuidePage::OwnerRoleGuide, GuidePopupState::FromOutside)->show(); 
            }
        };

        badgeInfo adminInfo{
            .spriteName = "badge_admin.png"_spr,
            .badgeID = "admin-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) {
                GuidePopup::create(GuidePage::AdminRoleGuide, GuidePopupState::FromOutside)->show();
            }
        };

        badgeInfo helperInfo{
            .spriteName = "badge_helper.png"_spr,
            .badgeID = "helper-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) {
                GuidePopup::create(GuidePage::HelperRoleGuide, GuidePopupState::FromOutside)->show();
            }
        };

        badgeInfo artistInfo{
            .spriteName = "badge_artist.png"_spr,
            .badgeID = "artist-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) {
                GuidePopup::create(GuidePage::ArtistRoleGuide, GuidePopupState::FromOutside)->show();
            }
        };

        badgeInfo contribInfo{
            .spriteName = "badge_contributor.png"_spr,
            .badgeID = "contrib-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) {
                GuidePopup::create(GuidePage::ContribRoleGuide, GuidePopupState::FromOutside)->show();
            }
        };

        badgeInfo boosterInfo{
            .spriteName = "badge_booster.png"_spr,
            .badgeID = "booster-badge"_spr,
            .callback = [](CCMenuItemSpriteExtra* btn) {
                GuidePopup::create(GuidePage::BoosterRoleGuide, GuidePopupState::FromOutside)->show();
            }
        };

        auto& dm = DataManager::getInstance();
        auto shared = dm.getSharedData();

        bool ownerFound =
            std::find(shared.owners.begin(), shared.owners.end(), score->m_accountID)
            != shared.owners.end();

        bool adminFound =
            std::find(shared.admins.begin(), shared.admins.end(), score->m_accountID)
            != shared.admins.end();

        bool helperFound =
            std::find(shared.helpers.begin(), shared.helpers.end(), score->m_accountID)
            != shared.helpers.end();

        bool contFound =
            std::find(shared.contributors.begin(), shared.contributors.end(), score->m_accountID)
            != shared.contributors.end();

        bool artistFound =
            std::find(shared.artists.begin(), shared.artists.end(), score->m_accountID)
            != shared.artists.end();

        bool boosterFound =
            std::find(shared.boosters.begin(), shared.boosters.end(), score->m_accountID)
            != shared.boosters.end();

        if (showBadge) {
            if (ownerFound) buildBadge(std::move(ownerInfo));
            if (adminFound) buildBadge(std::move(adminInfo));
            if (helperFound) buildBadge(std::move(helperInfo));
            if (artistFound) buildBadge(std::move(artistInfo));
            if (contFound) buildBadge(std::move(contribInfo));
            if (boosterFound) buildBadge(std::move(boosterInfo));
        }

        usernameMenu->updateLayout();

        auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

        if (auto id = leftMenu->getChildByID("manage-user-btn"_spr)) id->removeFromParent();

        GrindPosition pos = dm.getUserPosition();

        if (pos == GrindPosition::Admin || pos == GrindPosition::Owner) {
            auto manageUserBtn = Build<CCSprite>::create("button_add_2.png"_spr)
                .scale(0.55f)
                .intoMenuItem([score] { UserManagePopup::create(score)->show(); })
                .id("manage-user-btn"_spr)
                .parent(leftMenu)
                .intoParent()
                .updateLayout();
        }
    }
};

}