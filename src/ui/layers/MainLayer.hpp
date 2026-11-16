#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "../BaseLayer.hpp"
#include "../components/LoadingLayer.hpp"
#include "../../utils/globals.hpp"
#include "../../managers/GrindListsData.hpp"

using namespace geode::prelude;

namespace levelgrind {

class MainLayer : public BaseLayer {
public:
    static MainLayer* create();

    bool isSplitDifficultySelected(int difficulty) const;
    void setSplitDifficultySelected(int difficulty, bool selected);
    void refreshSplitDifficultyFilters();

    static GetLevelsBody savedSearchBody();

private:
    bool init() override;
    bool initSearchPanel();

    void update(float dt) override;

    CCMenu* addSection(CCNode* parent, char const* title, CCPoint center, CCSize size, float titleWidth, float menuScale, float gap);
    CCMenuItemToggler* addToggle(CCMenu* menu, CCNode* off, CCNode* on, std::string const& key, char const* id);

    void onKeyToggle(CCObject* sender);
    void onOpenDifficultySelector(CCObject* sender);
    void onSearch(CCObject* sender);
    void onRandom(CCObject* sender);
    void onReset(CCObject* sender);

    void setKey(std::string const& key, bool value);
    void updateDemonSection();
    void updateDifficultySelectorVisibility();
    void updateResetButton();

    void scheduleCountRefresh();
    void sendCountRequest();
    void setCountText(std::string const& value);

    std::unordered_map<CCMenuItemToggler*, std::string> m_toggleKeys;
    std::unordered_map<std::string, CCMenuItemToggler*> m_togglersByKey;

    CCMenu* m_demonsMenu = nullptr;
    CCNode* m_demonsHint = nullptr;
    CCNode* m_difficultySelector = nullptr;
    CCMenuItemSpriteExtra* m_resetBtn = nullptr;

    bool m_randomPending = false;
    float m_randomTimer = 10.f;
    int m_randomLevelID = -1;
    std::string m_randomKey;

    Ref<levelgrind::LoadingLayer> m_randomLoadingLayer;

    TaskHolder<web::WebResponse> m_listener;

    CCLabelBMFont* m_countLabel = nullptr;
    CCNode* m_starsRow = nullptr;
    void showCountResult(GetLevelsResponse const& parsed);
    std::unordered_map<std::string, GetLevelsResponse> m_countCache;
    float m_countMaxWidth = 140.f;
    TaskHolder<web::WebResponse> m_countListener;
    float m_countDelay = -1.f;
    int m_countSeq = 0;

    void initListsMode();
    void buildListsPanel();
    void applyListsMode();
    bool listFiltersDefault() const;
    lists::Filters m_listFilters;
    CCNode* m_listsPanel = nullptr;
    CCMenuItemToggler* m_listsToggle = nullptr;
    CCNode* m_searchPanel = nullptr;
    CCNode* m_randomBtn = nullptr;
    float m_actionScale = -1.f;

    bool m_splitHard4 = true;
    bool m_splitHard5 = true;
    bool m_splitHarder6 = true;
    bool m_splitHarder7 = true;
    bool m_splitInsane8 = true;
    bool m_splitInsane9 = true;

    ~MainLayer() { m_listener.cancel(); m_countListener.cancel(); }
};

}
