#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include "../ui/popups/GuidePopup.hpp"

using namespace geode::prelude;

namespace levelgrind::roles {

struct RoleBadge {
    std::string sprite;
    GuidePage page;
};

std::vector<RoleBadge> forAccount(int accountId);

CCMenuItemSpriteExtra* badgeButton(RoleBadge const& badge, float height);

}
