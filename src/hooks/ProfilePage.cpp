#include <UIBuilder.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../managers/DataManager.hpp"
#include "../ui/popups/UserManagePopup.hpp"
#include "../ui/popups/GuidePopup.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"

#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(GrindProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

        if (auto id = leftMenu->getChildByID("manage-user-btn"_spr)) id->removeFromParent();

        GrindPosition pos = DataManager::getInstance().getUserPosition();

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