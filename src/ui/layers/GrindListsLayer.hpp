#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include "../BaseLayer.hpp"
#include "../../managers/GrindListsData.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GrindListsLayer : public BaseLayer {
public:
    static GrindListsLayer* create();
private:
    bool init() override;
    void load();
    void showPage();
    void setStatus(std::string const& text);

    lists::Filters m_filters;
    std::vector<lists::GrindList> m_lists;
    int m_page = 0;

    cue::ListNode* m_listNode = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    CCLabelBMFont* m_status = nullptr;
    CCNode* m_spinner = nullptr;
    CCMenuItemSpriteExtra* m_prev = nullptr;
    CCMenuItemSpriteExtra* m_next = nullptr;
    TaskHolder<web::WebResponse> m_task;
    ~GrindListsLayer() { m_task.cancel(); }
};

}
