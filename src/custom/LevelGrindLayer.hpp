#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include <Geode/Geode.hpp>
#include <vector>
#pragma once

using namespace geode::prelude;

class LevelGrindLayer : public cocos2d::CCLayer {
public:
    static LevelGrindLayer* create();
    bool init() override;

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

    std::vector<int> difficulties;
    std::vector<int> lengths;
    std::vector<std::string> grindTypes;
    std::vector<int> demonDifficulties;

    std::vector<int> versions;

    cocos2d::CCMenu* demonsMenu;
    CCScale9Sprite* demonsPanel;
};