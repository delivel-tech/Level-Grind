#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCBlockLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <string>
#include <vector>
#pragma once

using namespace geode::prelude;

class LevelGrindLayer : public cocos2d::CCLayer {
public:
    static LevelGrindLayer* create();
    bool init() override;
    void update(float dt) override;

    void onBack(cocos2d::CCObject*);
    void keyBackClicked() override;

    void onShort(cocos2d::CCObject* sender);
    void onMedium(cocos2d::CCObject* sender);
    void onLong(cocos2d::CCObject* sender);
    void onXL(cocos2d::CCObject* sender);

    void onEasy(cocos2d::CCObject* sender);
    void onNormal(cocos2d::CCObject* sender);
    void onHard(cocos2d::CCObject* sender);
    void onHarder(cocos2d::CCObject* sender);
    void onInsane(cocos2d::CCObject* sender);

    void onSearchBtn(cocos2d::CCObject* sender);
    void onSettingsBtn(cocos2d::CCObject*);

    void onStarSwitcher(cocos2d::CCObject* sender);
    void onMoonSwitcher(cocos2d::CCObject* sender);
    void onCoinSwitcher(cocos2d::CCObject* sender);
    void onDemonSwitcher(cocos2d::CCObject* sender);

    void on22(cocos2d::CCObject* sender);
    void on21(cocos2d::CCObject* sender);
    void on20(cocos2d::CCObject* sender);
    void on19(cocos2d::CCObject* sender);
    void onLowerThan19(cocos2d::CCObject* sender);

    void onEasyD(cocos2d::CCObject* sender);
    void onMediumD(cocos2d::CCObject* sender);
    void onHardD(cocos2d::CCObject* sender);
    void onInsaneD(cocos2d::CCObject* sender);
    void onExtremeD(cocos2d::CCObject* sender);

    void onDiscordBtn(cocos2d::CCObject* sender);
    void onCreditsBtn(cocos2d::CCObject* sender);

    void onInfoBtn(cocos2d::CCObject* sender);

    void onRandomBtn(cocos2d::CCObject* sender);
    void onAnnouncementsBtn(cocos2d::CCObject* sender);

    void onAchievementsBtn(cocos2d::CCObject* sender);
    void onPetBtn(cocos2d::CCObject* sender);

    void onDiffSelectorBtn(cocos2d::CCObject* sender);
    void updateDiffSelectorButtonVisibility();

    bool isSplitDifficultySelected(int difficulty) const;
    void setSplitDifficultySelected(int difficulty, bool selected);
    void refreshSplitDifficultyFilters();

    std::vector<int> difficulties;
    std::vector<int> lengths;
    std::vector<std::string> grindTypes;
    std::vector<int> demonDifficulties;

    std::vector<int> versions;

    bool m_splitHard4 = false;
    bool m_splitHard5 = false;
    bool m_splitHarder6 = false;
    bool m_splitHarder7 = false;
    bool m_splitInsane8 = false;
    bool m_splitInsane9 = false;

    cocos2d::CCMenu* demonsMenu;
    CCScale9Sprite* demonsPanel;

    CCSprite* m_top = nullptr;

    async::TaskHolder<geode::utils::web::WebResponse> m_listener;

    CCMenuItemSpriteExtra* m_randBtn;
    CCMenuItemSpriteExtra* m_annBtn;

    bool m_randomPending = false;
    float m_randomTimer = 10.f;
    int m_randomLevelID = -1;
    std::string m_randomKey;
    LoadingCircle* m_randomLoadingCircle = nullptr;
    CCBlockLayer* m_randomBlockLayer = nullptr;

    ~LevelGrindLayer() { m_listener.cancel(); }
};