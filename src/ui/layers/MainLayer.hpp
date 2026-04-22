#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include "../BaseLayer.hpp"
#include "../components/LoadingLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/NineSlice.hpp"

using namespace geode::prelude;

namespace levelgrind {

class MainLayer : public BaseLayer {
public:
    static MainLayer* create();

    bool isSplitDifficultySelected(int difficulty) const;
    void setSplitDifficultySelected(int difficulty, bool selected);
    void refreshSplitDifficultyFilters();

    std::vector<int> m_difficulties;

private:
    bool init() override;
    bool initFarMenus();
    bool initMainPanel();
    bool initMD();

    void update(float dt) override;

    void onShortToggler(CCObject* sender);
    void onMediumToggler(CCObject* sender);
    void onLongToggler(CCObject* sender);
    void onXLToggler(CCObject* sender);

    void onAutoToggler(CCObject* sender);
    void onEasyToggler(CCObject* sender);
    void onNormalToggler(CCObject* sender);
    void onHardToggler(CCObject* sender);
    void onHarderToggler(CCObject* sender);
    void onInsaneToggler(CCObject* sender);
    void onOpenDifficultySelector(CCObject* sender);
    void updateDifficultySelectorVisibility();

    void onVer22Toggler(CCObject* sender);
    void onVer21Toggler(CCObject* sender);
    void onVer20Toggler(CCObject* sender);
    void onVer19Toggler(CCObject* sender);
    void onVerLower19Toggler(CCObject* sender);

    void onStarToggler(CCObject* sender);
    void onMoonToggler(CCObject* sender);
    void onCoinToggler(CCObject* sender);
    void onDemonToggler(CCObject* sender);

    void onEasyDemonToggler(CCObject* sender);
    void onMediumDemonToggler(CCObject* sender);
    void onHardDemonToggler(CCObject* sender);
    void onInsaneDemonToggler(CCObject* sender);
    void onExtremeDemonToggler(CCObject* sender);

    std::vector<int> m_lengths;
    std::vector<int> m_demonDifficulties;
    std::vector<std::string> m_grindTypes;
    std::vector<int> m_versions;

    bool m_randomPending = false;
    float m_randomTimer = 10.f;
    int m_randomLevelID = -1;
    std::string m_randomKey;

    Ref<levelgrind::LoadingLayer> m_randomLoadingLayer;

    std::pair<NineSlice*, CCMenu*> m_demonsFilters;

    TaskHolder<web::WebResponse> m_listener;

    bool m_splitHard4 = true;
    bool m_splitHard5 = true;
    bool m_splitHarder6 = true;
    bool m_splitHarder7 = true;
    bool m_splitInsane8 = true;
    bool m_splitInsane9 = true;

    ~MainLayer() { m_listener.cancel(); }
};

}