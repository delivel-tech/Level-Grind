#pragma once

#include <Geode/Geode.hpp>
#include "../BaseLayer.hpp"
#include "../../utils/globals.hpp"

#include <cue/ListNode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

namespace levelgrind {

class SuggestionsLayer : public BaseLayer, public LevelManagerDelegate, public SetIDPopupDelegate {
public:
    enum class PointsFilterOp {
        More,
        Equals,
        Less
    };

    static SuggestionsLayer* create();
    void keyBackClicked() override;

    void setPointsFilter(PointsFilterOp op, int value);
    PointsFilterOp getPointsFilterOp() const;
    int getPointsFilterValue() const;
    void applyPointsFilter();

    ~SuggestionsLayer() {
        m_searchTask.cancel();
        auto glm = GameLevelManager::get();
        if (glm && glm->m_levelManagerDelegate == this) {
            glm->m_levelManagerDelegate = nullptr;
        }
    }

private:
    bool init() override;

    void onInfoButton(CCObject* sender);
    void onPageButton(CCObject* sender);
    void onPrevPage(CCObject* sender);
    void onNextPage(CCObject* sender);
    void onRefresh(CCObject* sender);
    void onFilterButton(CCObject* sender);

    void updatePageButton();
    void refreshLevels();
    void startLoading();
    void stopLoading();
    void hideUIElements();
    void showUIElements();
    void populateFromArray(CCArray* levels);
    void performFetchLevels();
    void loadPageFromStoredIDs();

    void loadLevelsFinished(CCArray* levels, char const* key, int p2) override;
    void loadLevelsFailed(char const* key, int p1) override;
    void setIDPopupClosed(SetIDPopup* popup, int value) override;

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;

    std::string m_title;

    std::vector<int> m_allLevelIDs;
    std::vector<LevelSuggestion> m_allLevelSuggestions;
    std::vector<matjson::Value> m_initialSuggestions;
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
    CCMenuItemSpriteExtra* m_filterBtn = nullptr;

    int m_page = 0;
    int m_suggestionsMode = 0;

    PointsFilterOp m_pointsFilterOp = PointsFilterOp::More;
    int m_pointsFilterValue = 0;

    TaskHolder<web::WebResponse> m_searchTask;
};

}