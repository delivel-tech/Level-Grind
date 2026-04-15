#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../BasePopup.hpp"
#include "../../utils/globals.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class UserManagePopup : public BasePopup {
public:
    static UserManagePopup* create(GJUserScore* targetUser);

private:
    bool init(GJUserScore* targetUser);

    GJUserScore* m_targetUser;
    LoadingSpinner* m_spinner;
    CCMenu* m_optionsMenu;
    CCLabelBMFont* m_targetLabel;
    CCMenu* m_targetLabelMenu;
    CCSprite* getBadgeByHighestRole(UserRoles roles);

    void buildUI();

    TaskHolder<web::WebResponse> m_listener;

    ~UserManagePopup() {
        m_listener.cancel();
    }
};

}