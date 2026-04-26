#include <Geode/Geode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../ui/popups/ManageLevelPopup.hpp"
#include "../managers/DataManager.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Notification.hpp"

#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(LevelGrind, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        
        // staff things code, everything else goes below
        GrindPosition userPos = DataManager::getInstance().getUserPosition();

        if (userPos != GrindPosition::Helper
        && userPos != GrindPosition::Admin
        && userPos != GrindPosition::Owner) return true;

        auto leftSideMenu = this->getChildByIDRecursive("left-side-menu");
        log::warn("left side menu not found");
        if (!leftSideMenu) return true;

        Build<CCSprite>::create("button_add_1.png"_spr)
            .scale(0.847f)
            .intoMenuItem([this, userPos] {
                bool isRated = this->m_level->m_stars.value() > 0;
                if (isRated) {
                    if (userPos != GrindPosition::User) ManageLevelPopup::create(this->m_level)->show();
                    else Notification::create("You cannot open manage level popup.", NotificationIcon::Error)->show();
                } else {
                    if (userPos == GrindPosition::Helper || userPos == GrindPosition::User) {
                        FLAlertLayer::create(
					        "Level Not Rated!",
					        "This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>.",
					        "OK"
				        )->show();
                    } else {
                        createQuickPopup(
					        "Level Not Rated!",
					        "This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>. Do you <cp>still want to open</c>?",
					        "Cancel", "Open",
					        [&](auto, bool btn2) {
						        if (btn2) {
							        ManageLevelPopup::create(this->m_level)->show();
						        }
					        }
				        );
                    }
                }
            })
            .parent(leftSideMenu)
            .intoParent()
            .updateLayout();

        return true;
    }
};

}