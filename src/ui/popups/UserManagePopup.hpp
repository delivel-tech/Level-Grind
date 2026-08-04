#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../BasePopup.hpp"
#include "../../features/admin/AdminTypes.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>

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
    arc::Future<> onLoadUserRoles();
    arc::Future<> onWipePetClicked();
    arc::Future<> onUnbanPetClicked();
};

}