#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>

#pragma once

using namespace geode::prelude;

class UserManagePopup : public Popup {
protected:
    bool init(int targetAccountID, const char* username, int icon, int color1, int color2, int color3);
    async::TaskHolder<geode::utils::web::WebResponse> m_listener;

public:
    static UserManagePopup* create(int accountID, const char* username, int icon, int color1, int color2, int color3);

    void onDemoteBtn(CCObject* sender);
    void onPromoteBtn(CCObject* sender);

    void onDemoteAdminBtn(CCObject* sender);
    void onPromoteAdminBtn(CCObject* sender);

    int m_targetAccountID;
    const char* m_username;

    int m_targetIcon;
    int m_targetColor1;
    int m_targetColor2;
    int m_targetColor3;
};