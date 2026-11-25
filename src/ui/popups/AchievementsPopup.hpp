#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Label.hpp>
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
    cue::ListNode* m_tabs = nullptr;
    cue::ListNode* m_list = nullptr;
    geode::Label* m_summary = nullptr;
    CCNode* m_spinner = nullptr;
};

}
