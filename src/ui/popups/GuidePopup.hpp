#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/CCScrollLayerExt.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"
#include "../../features/levels/LevelTypes.hpp"
#include "../../core/CoreTypes.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GuidePopup : public BasePopup {
public:
    static GuidePopup* create(GuidePage page, GuidePopupState state);

private:
    bool init(GuidePage page, GuidePopupState state);
    bool initPage(GuidePage page);

    void keyBackClicked() override;
    void addReturnButton();
    void addGrindLogo();
    void addTextArea(std::string text);
    void addScrollbar();
    void addScrollbar(CCScrollLayerExt* scrollLayer);

    cue::ListNode* m_listNode = nullptr;
    GuidePopupState m_state;

    bool initMainPage();
    bool initMainGuidePage();
    bool initPetGuidePage();
    bool initWeeklyAchievementsGuidePage();
    bool initNotesGuidePage();
    bool initOwnerRoleGuidePage();
    bool initAdminRoleGuidePage();
    bool initHelperRoleGuidePage();
    bool initArtistRoleGuidePage();
    bool initContribRoleGuidePage();
    bool initBoosterRoleGuidePage();
    bool initEventsGuidePage();
    bool initGrindPacksGuide();
    bool initHelperSuggestionsGuide();
    bool initLevelIndicatorsGuide();
    bool initCreditsInfoGuide();

    void removePrevious();
};

}