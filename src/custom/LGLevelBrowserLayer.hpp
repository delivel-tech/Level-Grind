#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/misc_nodes/CCProgressTimer.h"
#include "Geode/ui/ProgressBar.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/modify/GameLevelManager.hpp>

using namespace geode::prelude;

class LGLevelBrowserLayer : public CCLayer, public LevelManagerDelegate, public SetIDPopupDelegate {
public:
    static LGLevelBrowserLayer* create(
        std::vector<int> difficulties,
        std::vector<int> lengths,
        std::vector<std::string> grindTypes,
        std::vector<int> demonDifficulties,
        std::vector<int> versions,
        bool newerFirst,
        bool recentlyAdded
    );
    bool init(GJSearchObject* object);
    void keyBackClicked() override;
    float getCompletionPercent() const { return m_completionPercent; }
    
    ~LGLevelBrowserLayer() {
        m_searchTask.cancel();
        auto glm = GameLevelManager::get();
        if (glm && glm->m_levelManagerDelegate == this) {
            glm->m_levelManagerDelegate = nullptr;
        }
    }

    std::vector<int> m_difficulties;
    std::vector<int> m_lengths;
    std::vector<std::string> m_grindTypes;
    std::vector<int> m_demonDifficulties;
    bool m_isNewerFirst;
    bool m_isRecentlyAdded;

    std::vector<int> m_versions;

    GJSearchObject* m_searchObject;
    
    geode::async::TaskHolder<web::WebResponse> m_searchTask;
    
    std::vector<int> m_allLevelIDs; 
    int m_totalLevels = 0;
    int m_totalPages = 1;
    bool m_loading = false;
    bool m_needsLayout = false;

protected:
    void onInfoButton(CCObject* sender);
    void onPageButton(CCObject* sender);
    void onRandomPage(CCObject* sender);
    void onPrevPage(CCObject* sender);
    void onNextPage(CCObject* sender);

    void updatePageButton();
    void onRefresh(CCObject *sender);

    void refreshLevels();
    void startLoading();
    void stopLoading();
    void hideUIElements();
    void showUIElements();
    void recalculateCompletionProgress();
    void populateFromArray(CCArray* levels);
    void performFetchLevels();
    void loadPageFromStoredIDs(); 

    void loadLevelsFinished(CCArray* levels, char const* key, int p2) override;
    void loadLevelsFailed(char const* key, int p1) override;
    
    void setIDPopupClosed(SetIDPopup* popup, int value) override;
    
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void update(float dt) override;

    GJListLayer* m_listLayer;
    ScrollLayer* m_scrollLayer;

    LoadingSpinner* m_circle;
    CCLabelBMFont* m_levelsLabel;
    CCLabelBMFont* m_completionInfoLabel;
    CCLabelBMFont* m_pageButtonLabel;

    CCMenuItemSpriteExtra* m_pageButton;
    CCMenuItemSpriteExtra* m_randomPageButton;
    CCMenuItemSpriteExtra* m_refreshBtn;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;

    ProgressBar* m_progressBar;

    int m_completedLevels = 0;
    bool m_shouldShowProgressBar = false;
    float m_completionPercent = 0.f;

    int m_page = 0;
};