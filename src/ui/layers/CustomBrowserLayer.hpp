#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include "../BaseLayer.hpp"
#include "../../features/levels/LevelTypes.hpp"
#include "../../features/events/EventTypes.hpp"
#include "../../core/BrowserTypes.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/utils/async.hpp"

#include <arc/future/Future.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class CustomBrowserLayer : public BaseLayer, public LevelManagerDelegate, public SetIDPopupDelegate {
public:
    static CustomBrowserLayer* create(GetLevelsBody body, std::string title, CustomBrowserType type, EventType eventType);
    void keyBackClicked() override;

    ~CustomBrowserLayer() {
        auto glm = GameLevelManager::get();
        if (glm && glm->m_levelManagerDelegate == this) {
            glm->m_levelManagerDelegate = nullptr;
        }
    }

private:
    bool init(GetLevelsBody body, std::string title, CustomBrowserType type, EventType eventType);

    void onInfoButton(CCObject* sender);
    void onPageButton(CCObject* sender);
    void onPrevPage(CCObject* sender);
    void onNextPage(CCObject* sender);
    void onRefresh(CCObject* sender);

    void updatePageButton();
    void refreshLevels();
    void startLoading();
    void stopLoading();
    void hideUIElements();
    void showUIElements();
    void populateFromArray(CCArray* levels);
    void performFetchLevels();
    arc::Future<> onFetchLevels();
    void loadPageFromStoredIDs();

    void loadLevelsFinished(CCArray* levels, char const* key, int p2) override;
    void loadLevelsFailed(char const* key, int p1) override;
    void setIDPopupClosed(SetIDPopup* popup, int value) override;

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;

    GetLevelsBody m_body;
    std::string m_title;
    CustomBrowserType m_type;
    EventType m_eventType;

    std::vector<int> m_allLevelIDs;
    int m_totalLevels = 0;
    int m_totalPages = 1;
    bool m_loading = false;
    bool m_needsLayout = false;

    GJSearchObject* m_searchObject = nullptr;

    cue::ListNode* m_listNode = nullptr;
    ScrollLayer* m_scrollLayer = nullptr;

    LoadingSpinner* m_circle = nullptr;
    CCLabelBMFont* m_levelsLabel = nullptr;
    CCLabelBMFont* m_pageButtonLabel = nullptr;

    CCMenuItemSpriteExtra* m_pageButton = nullptr;
    CCMenuItemSpriteExtra* m_refreshBtn = nullptr;
    CCMenuItemSpriteExtra* m_prevButton = nullptr;
    CCMenuItemSpriteExtra* m_nextButton = nullptr;

    ProgressBar* m_progressBar = nullptr;
    CCLabelBMFont* m_completionInfoLabel = nullptr;

    CCMenuItemSpriteExtra* m_randomPageBtn = nullptr;

    int m_page = 0;
};

}