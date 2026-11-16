#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

enum class ReportTarget { Clan = 0, Pet = 1, Message = 2 };

class ReportPopup : public BasePopup {
public:
    static ReportPopup* create(ReportTarget type, int targetId, std::string const& targetName);
private:
    bool init(ReportTarget type, int targetId, std::string const& targetName);
    void refreshCategories();
    void onSend();

    ReportTarget m_type = ReportTarget::Clan;
    int m_targetId = 0;
    int m_category = 0;
    std::vector<CCMenuItemSpriteExtra*> m_catButtons;
    CCMenu* m_catMenu = nullptr;
    TextInput* m_reason = nullptr;
};

class PetLeaderboardPopup : public BasePopup {
public:
    static PetLeaderboardPopup* create();
private:
    bool init() override;
    void load();

    std::string m_sort = "level";
    int m_page = 0;
    bool m_hasMore = false;
    cue::ListNode* m_list = nullptr;
    CCMenu* m_tabs = nullptr;
    CCLabelBMFont* m_footer = nullptr;
    CCMenuItemSpriteExtra* m_prev = nullptr;
    CCMenuItemSpriteExtra* m_next = nullptr;
    CCNode* m_status = nullptr;
    TaskHolder<web::WebResponse> m_task;
    ~PetLeaderboardPopup() { m_task.cancel(); }
};

class StaffReportsPopup : public BasePopup {
public:
    static StaffReportsPopup* create();
    void load();
private:
    bool init() override;

    int m_status = 0;
    int m_page = 0;
    bool m_hasMore = false;
    int m_modType = 1;
    cue::ListNode* m_list = nullptr;
    CCMenu* m_tabs = nullptr;
    CCLabelBMFont* m_footer = nullptr;
    CCMenuItemSpriteExtra* m_prev = nullptr;
    CCMenuItemSpriteExtra* m_next = nullptr;
    CCNode* m_status_ = nullptr;
    TaskHolder<web::WebResponse> m_task;
    ~StaffReportsPopup() { m_task.cancel(); }
};

}
