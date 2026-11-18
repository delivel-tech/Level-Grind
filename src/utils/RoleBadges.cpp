#include "RoleBadges.hpp"
#include "../managers/DataManager.hpp"

#include <algorithm>

namespace levelgrind::roles {

namespace {

bool contains(std::vector<int> const& list, int id) {
    return std::find(list.begin(), list.end(), id) != list.end();
}

}

std::vector<RoleBadge> forAccount(int accountId) {
    std::vector<RoleBadge> out;
    if (accountId <= 0) return out;
    auto const& shared = DataManager::getInstance().getSharedData();

    if (contains(shared.owners, accountId)) out.push_back({ "badge_owner.png"_spr, GuidePage::OwnerRoleGuide });
    if (contains(shared.developers, accountId)) out.push_back({ "badge_developer.png"_spr, GuidePage::DeveloperRoleGuide });
    if (contains(shared.admins, accountId)) out.push_back({ "badge_admin.png"_spr, GuidePage::AdminRoleGuide });
    if (contains(shared.helpers, accountId)) out.push_back({ "badge_helper.png"_spr, GuidePage::HelperRoleGuide });
    if (contains(shared.contributors, accountId)) out.push_back({ "badge_contributor.png"_spr, GuidePage::ContribRoleGuide });
    if (contains(shared.artists, accountId)) out.push_back({ "badge_artist.png"_spr, GuidePage::ArtistRoleGuide });
    if (contains(shared.boosters, accountId)) out.push_back({ "badge_booster.png"_spr, GuidePage::BoosterRoleGuide });
    return out;
}

CCMenuItemSpriteExtra* badgeButton(RoleBadge const& badge, float height) {
    auto spr = CCSprite::createWithSpriteFrameName(badge.sprite.c_str());
    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    spr->setScale(height / std::max(1.f, spr->getContentHeight()));
    auto page = badge.page;
    return CCMenuItemExt::createSpriteExtra(spr, [page](CCObject*) {
        GuidePopup::create(page, GuidePopupState::FromOutside)->show();
    });
}

}
