#include "BaseLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/ui/General.hpp"
#include <cue/RepeatingBackground.hpp>

#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

BaseLayer* BaseLayer::create() {
    auto ret = new BaseLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void BaseLayer::open() {
    auto scene = CCScene::create();
    scene->addChild(this);

    auto transition = CCTransitionFade::create(0.5f, scene);

    CCDirector::sharedDirector()->pushScene(transition);
}

bool BaseLayer::init() {
    if (!CCLayer::init()) return false;

    this->setKeyboardEnabled(true);
    this->setKeypadEnabled(true);

    m_winSize = CCDirector::sharedDirector()->getWinSize();

    auto backBtn = addBackButton(this, BackButtonStyle::Green);
    m_backBtn = backBtn;

    auto customBg = Build(cue::RepeatingBackground::create("game_bg_01_001.png", 1.0f, cue::RepeatMode::X))
        .color({ 37, 50, 167 })
        .parent(this)
        .collect();

    m_bg = customBg;

    return true;
}

void BaseLayer::keyBackClicked() {
    this->onBack(nullptr);
}

void BaseLayer::onBack(CCObject* sender) {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

}