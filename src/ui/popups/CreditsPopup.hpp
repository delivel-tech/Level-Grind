#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class CreditsPopup : public Popup {
public:
    static CreditsPopup* create();

protected:
    bool init() override;

    void loadCredits();
    void onUpdateProfile(CCObject*);
    void applyProfileUpdate(int accountId, const std::string& username, int icon, int color1, int color2, int color3);

    cue::ListNode* m_list = nullptr;

    CCMenuItemSpriteExtra* m_updateBtn = nullptr;
    bool m_updating = false;

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_updateListener;

    ~CreditsPopup() {
        m_listener.cancel();
        m_updateListener.cancel();
    }
};

}