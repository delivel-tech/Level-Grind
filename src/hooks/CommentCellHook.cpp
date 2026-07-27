#include <Geode/Geode.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <UIBuilder.hpp>

#include "../managers/DataManager.hpp"

#include "../ui/popups/GuidePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class $modify(CommentCell) {
    void loadFromComment(GJComment* p0) {
        CommentCell::loadFromComment(p0);
        auto layer = m_mainLayer;

        CCMenu* usernameMenu = static_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));

        auto mod = Mod::get();

        bool hideBadges = mod->getSavedValue<bool>("disable-badges");
        bool hideBadgesForMods = mod->getSavedValue<bool>("hide-badges-for-mods");

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
                Build(CCSprite::create(Info.spriteName.c_str())).scale(0.75f).collect(),
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
            std::find(shared.owners.begin(), shared.owners.end(), p0->m_accountID)
            != shared.owners.end();

        bool adminFound =
            std::find(shared.admins.begin(), shared.admins.end(), p0->m_accountID)
            != shared.admins.end();

        bool helperFound =
            std::find(shared.helpers.begin(), shared.helpers.end(), p0->m_accountID)
            != shared.helpers.end();

        bool contFound =
            std::find(shared.contributors.begin(), shared.contributors.end(), p0->m_accountID)
            != shared.contributors.end();

        bool artistFound =
            std::find(shared.artists.begin(), shared.artists.end(), p0->m_accountID)
            != shared.artists.end();

        bool boosterFound =
            std::find(shared.boosters.begin(), shared.boosters.end(), p0->m_accountID)
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
    }
};

}