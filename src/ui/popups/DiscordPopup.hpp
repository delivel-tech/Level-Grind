#pragma once
#include "../BasePopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/misc_nodes/CCClippingNode.h"
#include "Geode/ui/MDTextArea.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"
#include <cue/RepeatingBackground.hpp>

using namespace geode::prelude;

namespace levelgrind {

class DiscordPopup : public BasePopup {
public:
    struct DiscordPageInfo {
        std::string title = "";
        std::string description = "";
    };

    static DiscordPopup* create(std::vector<DiscordPageInfo> pages);

private:
    CCMenu* m_mainMenu = nullptr;
    CCMenu* m_interactiveMenu = nullptr;
    NineSlice* m_interactiveMenuBG = nullptr;
    NineSlice* m_stencil = nullptr;
    CCClippingNode* m_clip = nullptr;
    cue::RepeatingBackground* m_background = nullptr;
    float m_hue = 0.f;

    std::vector<DiscordPageInfo> m_pages;
    int m_currentPage = 0;

    CCMenuItemSpriteExtra* m_pageArrowLeft = nullptr;
    CCMenuItemSpriteExtra* m_pageArrowRight = nullptr;
    CCLabelBMFont* m_pageTitleLabel = nullptr;
    MDTextArea* m_pageDescArea = nullptr;

    bool init(std::vector<DiscordPageInfo> pages);
    void update(float dt) override;
    void logOpened();

    void createClip();

    void wrapNodeIntoClippingNode(CCNode* node);
    CCMenu* createNewMainMenu();

    cue::RepeatingBackground* createBackground();

    static ccColor3B hueToColor(float hue, float saturation = 1.f, float value = 1.f);

    void createPagesUI();
    void turnPage(int dir);
    void updatePage();
    void cleanUpPage();

    TaskHolder<web::WebResponse> m_listener;
};

}