#pragma once
#include <Geode/Geode.hpp>
#include "../../utils/globals.hpp"
#include "../BasePopup.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class RoleSelectorPopup : public BasePopup {
public:
    static RoleSelectorPopup* create(UserRoles roles, GJUserScore* targetUser);

private:
    bool init(UserRoles roles, GJUserScore* targetUser);
    UserRoles m_newRoles;
    GJUserScore* m_targetUser;

    void onAdminToggle(CCObject* sender);
    void onHelperToggle(CCObject* sender);
    void onArtistToggle(CCObject* sender);
    void onBoosterToggle(CCObject* sender);
    void onContributorToggle(CCObject* sender);

    TaskHolder<web::WebResponse> m_listener;

    ~RoleSelectorPopup() {
        m_listener.cancel();
    }
};

}