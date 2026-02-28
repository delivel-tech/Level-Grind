#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJGameLevel.hpp>

class HelperPopup : public geode::Popup {
protected:
    bool init(GJGameLevel* level);

public:
    static HelperPopup* create(GJGameLevel* level);
    void onAddButton(cocos2d::CCObject* sender);

    bool star = false;
    bool moon = false;
    bool coin = false;
    bool demon = false;
    int demonDifficulty = 0;

    int levelID = 0;
    std::string levelName = "";
    int levelDifficulty = 0;
    int levelLength = 0;

    void onStarSwitcher(cocos2d::CCObject* sender);
    void onMoonSwitcher(cocos2d::CCObject* sender);
    void onCoinSwitcher(cocos2d::CCObject* sender);
    void onDemonSwitcher(cocos2d::CCObject* sender);
    void onDeleteBtn(cocos2d::CCObject* sender);

protected:
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener2;
};