#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <cue/ListNode.hpp>
#include <map>
#include "../BaseLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GrindLeaderboardsLayer : public BaseLayer {
public:
    static GrindLeaderboardsLayer* create();

    struct Entry {
        int rank = 0;
        int accountId = 0;
        std::string username;
        long long value = 0;
        int icon = 1, color1 = 0, color2 = 3, color3 = 0;
    };
    struct Board {
        std::vector<Entry> entries;
        std::optional<Entry> me;
        int players = 0;
        long long resetsAt = 0;
        long long fetchedAt = 0;
        long long serverOffset = 0;
    };

private:
    bool init() override;
    void onTab(CCObject* sender);
    void buildTypeButtons();
    void load();
    void show(Board const& board);
    void tick(float);
    CCNode* makeRow(Entry const& e, bool mine);
    std::string key() const;

    int m_period = 0;
    int m_type = 0;
    std::vector<TabButton*> m_tabs;
    CCMenu* m_typeMenu = nullptr;
    cue::ListNode* m_list = nullptr;
    CCNode* m_spinner = nullptr;
    CCLabelBMFont* m_status = nullptr;
    CCLabelBMFont* m_timer = nullptr;
    std::map<std::string, Board> m_cache;
    long long m_resetsAt = 0;
    long long m_serverOffset = 0;
    int m_requestSeq = 0;
    TaskHolder<web::WebResponse> m_task;

    ~GrindLeaderboardsLayer() { m_task.cancel(); }
};

}
