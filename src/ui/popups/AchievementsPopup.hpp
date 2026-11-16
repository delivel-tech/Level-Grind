#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"
#include "../../managers/AchievementsManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

class AchievementsPopup : public BasePopup {
public:
    static AchievementsPopup* create();
private:
    bool init() override;
    void buildTabs();
    void buildList();
    CCNode* makeCell(achievements::Achievement const& a, float width);

    std::vector<achievements::Achievement> m_all;
    std::string m_category = "levels";
    CCMenu* m_tabs = nullptr;
    cue::ListNode* m_list = nullptr;
    CCLabelBMFont* m_summary = nullptr;
    CCNode* m_spinner = nullptr;
};

class NotificationsPopup : public BasePopup {
public:
    static NotificationsPopup* create();
private:
    bool init() override;
    void build();
    cue::ListNode* m_list = nullptr;
};

}
