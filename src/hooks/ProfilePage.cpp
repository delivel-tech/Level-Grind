#include <UIBuilder.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../managers/DataManager.hpp"
#include "../ui/popups/UserManagePopup.hpp"

#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(GrindProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

        GrindPosition pos = DataManager::getInstance().getUserPosition();

        if (pos == GrindPosition::Admin || pos == GrindPosition::Owner) {
            auto manageUserBtn = Build<CCSprite>::create("button_add_2.png"_spr)
                .scale(0.55f)
                .intoMenuItem([score] { UserManagePopup::create(score)->show(); })
                .id("manage-user-btn")
                .parent(leftMenu)
                .intoParent()
                .updateLayout();
        }
    }
};

}