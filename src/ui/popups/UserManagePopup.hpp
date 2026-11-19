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
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class UserManagePopup : public BasePopup {
public:
    static UserManagePopup* create(GJUserScore* targetUser);

private:
    bool init(GJUserScore* targetUser);

    Ref<GJUserScore> m_targetUser;
    LoadingSpinner* m_spinner;
    CCMenu* m_optionsMenu = nullptr;
    CCLabelBMFont* m_targetLabel;
    CCMenu* m_targetLabelMenu;
    CCSprite* getBadgeByHighestRole(UserRoles roles);

    void buildUI();
    void loadModeration();
    void buildActions();
    void wipePet();
    void unbanPet();
    void moderate(std::string endpoint, matjson::Value body, std::string loading, std::string success);

    bool m_petExists = false;
    bool m_petBanned = false;
    bool m_clanBan = false;
    bool m_lbBan = false;
    TaskHolder<web::WebResponse> m_modTask;

    TaskHolder<web::WebResponse> m_listener;

    ~UserManagePopup() {
        m_modTask.cancel();
        m_listener.cancel();
    }
};

}