#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

class BaseLayer : public CCLayer {
public:
    static BaseLayer* create();
    bool init() override;
    CCMenuItemSpriteExtra* m_backBtn = nullptr;

private:
    void keyBackClicked() override;
    virtual void onBack(CCObject* sender);
};