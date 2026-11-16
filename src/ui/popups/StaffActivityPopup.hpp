#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class StaffActivityPopup : public BasePopup {
public:
    static StaffActivityPopup* create();

private:
    bool init() override;
    void load(int mode);

    int m_mode = 1;
    bool m_notes = false;
    cue::ListNode* m_list = nullptr;
    CCNode* m_status = nullptr;
    std::vector<CCMenuItemSpriteExtra*> m_tabs;
    CCMenu* m_tabMenu = nullptr;

    TaskHolder<web::WebResponse> m_listener;

    ~StaffActivityPopup() {
        m_listener.cancel();
    }
};

}
