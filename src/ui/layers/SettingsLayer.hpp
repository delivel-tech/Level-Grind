#include "../BaseLayer.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/BasedButton.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class SettingsLayer : public BaseLayer {
public:
    static SettingsLayer* create();
    static void initSettings();

private:
    bool init() override;

    void makeTabs();
    void onTab(CCObject* sender);

    void onBackgroundColorPicker(CCObject* sender);
    void refreshBackgroundColorUI();

    static void applyDefaultSettings();

    void createGrindList();
    void createAppearanceList();
    void createOtherList();

    void setBoolSetting(const char* key, bool value);

    CCMenu* makeToggleCell(
        const char* title,
        const char* desc,
        const char* settingKey,
        CCNode* target
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
        SEL_MenuHandler cb,
        CCNode* target
    );

    CCMenu* makeHeaderCell(const char* title);

    std::vector<cue::ListNode*> m_listNodes;
    std::vector<TabButton*> m_tabs;

    cue::ListNode* m_currentList = nullptr;
    cue::ListNode* m_grindList = nullptr;
    cue::ListNode* m_appearanceList = nullptr;
    cue::ListNode* m_otherList = nullptr;

    TabButton* m_grindTab = nullptr;
    TabButton* m_appearanceTab = nullptr;
    TabButton* m_otherTab = nullptr;

    CCMenuItemSpriteExtra* m_resetBtn = nullptr;
    CCMenu* m_resetBtnMenu = nullptr;

    ColorChannelSprite* m_bgPreview = nullptr;

    CCMenuItemToggler* m_onlyUncompletedToggler = nullptr;
    CCMenuItemToggler* m_onlyCompletedToggler = nullptr;
    CCMenuItemToggler* m_newerFirstToggler = nullptr;
    CCMenuItemToggler* m_recentlyAddedToggler = nullptr;

    static constexpr CCSize CELL_SIZE = {356.f, 28.f};

    TaskHolder<web::WebResponse> m_listener;
};

}