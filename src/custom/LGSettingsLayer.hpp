#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cue/ListNode.hpp>
#include <unordered_map>

#include "BaseLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/BasedButton.hpp"

using namespace geode::prelude;
class ColorChannelSprite;

class LGSettingsLayer : public BaseLayer {
public:
    static LGSettingsLayer* create(bool isFromPet);

private:
    bool init(bool isFromPet);
    void scrollWheel(float y, float x) override;

    void selectTab(cue::ListNode* tab);
    void createGrindTab();
    void createAppearanceTab();
    void createPetTab();
    void createStaffTab();
    void refreshBackgroundColorUI();
    bool getIncomingToggleValue(CCObject* sender) const;
    void setBoolSetting(char const* key, bool value, bool syncUI = true);

    CCMenu* makeToggleCell(
        const char* title,
        const char* desc,
        const char* settingKey,
        cocos2d::SEL_MenuHandler cb,
        cocos2d::CCNode* target
    );

    CCMenu* makeFloatCell(
        const char* title,
        const char* desc,
        const char* settingKey,
        float min,
        float max
    );

    CCMenu* makeColorCell(
        const char* title,
        const char* desc,
        cocos2d::SEL_MenuHandler cb,
        cocos2d::CCNode* target
    );

    CCMenu* makeHeader(const char* title);

    void makeTabs();

    void onUncompletedToggled(CCObject* sender);
    void onCompletedToggled(CCObject* sender);
    void onNewerFirst(CCObject* sender);
    void onRecentlyAdded(CCObject* sender);
    void onHideProgressBar(CCObject* sender);
    void onHideCompletionInfo(CCObject* sender);
    void onDisableStarParticles(CCObject* sender);
    void onDisableCustomBackground(CCObject* sender);
    void onNoBadgeForMods(CCObject* sender);
    void onDisableBadges(CCObject* sender);
    void onDisablePet(CCObject* sender);
    void onDisableAutoNotes(CCObject* sender);

    void onBackgroundSpeedArrow(CCObject* sender);
    void onBackgroundColorPick(CCObject* sender);
    void updateBackgroundSpeedUI(float speed, cocos2d::CCNode* source = nullptr);

    void onTabBtn(CCObject* sender);
    void onResetSettings(CCObject* sender);
    void applyDefaultSettings();

    std::vector<cue::ListNode*> m_tabs;
    std::vector<TabButton*> m_tabButtons;
    TabButton* m_grindTabBtn = nullptr;
    TabButton* m_appearanceTabBtn = nullptr;
    TabButton* m_petTabBtn = nullptr;
    TabButton* m_staffTabBtn = nullptr;
    cue::ListNode* m_currentTab = nullptr;
    cue::ListNode* m_grindTab = nullptr;
    cue::ListNode* m_appearanceTab = nullptr;
    cue::ListNode* m_petTab = nullptr;
    cue::ListNode* m_staffTab = nullptr;
    std::unordered_map<std::string, CCMenuItemToggler*> m_boolTogglers;
    std::unordered_map<CCMenuItemSpriteExtra*, float> m_speedArrowSteps;

    TextInput* m_backgroundSpeedInput = nullptr;
    CCMenuItemSpriteExtra* m_bigSpeedLeftBtn = nullptr;
    CCMenuItemSpriteExtra* m_speedLeftBtn = nullptr;
    CCMenuItemSpriteExtra* m_speedRightBtn = nullptr;
    CCMenuItemSpriteExtra* m_bigSpeedRightBtn = nullptr;
    ColorChannelSprite* m_bgPreview = nullptr;

    static constexpr CCSize CELL_SIZE = {356.f, 28.f};
    static constexpr float BG_SPEED_MIN = 0.f;
    static constexpr float BG_SPEED_MAX = 5.f;
};