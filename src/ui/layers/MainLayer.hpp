#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include "../BaseLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"

using namespace geode::prelude;

namespace levelgrind {

class MainLayer : public BaseLayer {
public:
    static MainLayer* create();

    bool isSplitDifficultySelected(int difficulty) const;
    void setSplitDifficultySelected(int difficulty, bool selected);
    void refreshSplitDifficultyFilters();

private:
    bool init() override;
    bool initFarMenus();
    bool initMainPanel();
    bool initMD();

    void onShortToggler(CCObject* sender);
    void onMediumToggler(CCObject* sender);
    void onLongToggler(CCObject* sender);
    void onXLToggler(CCObject* sender);

    void onEasyToggler(CCObject* sender);
    void onNormalToggler(CCObject* sender);
    void onHardToggler(CCObject* sender);
    void onHarderToggler(CCObject* sender);
    void onInsaneToggler(CCObject* sender);
    void onOpenDifficultySelector(CCObject* sender);
    void updateDifficultySelectorVisibility();

    std::vector<int> m_difficulties;
    std::vector<int> m_lengths;
    std::vector<int> m_demonDifficulties;
    std::vector<std::string> m_grindTypes;
    std::vector<int> m_versions;

    bool m_splitHard4 = true;
    bool m_splitHard5 = true;
    bool m_splitHarder6 = true;
    bool m_splitHarder7 = true;
    bool m_splitInsane8 = true;
    bool m_splitInsane9 = true;
};

}