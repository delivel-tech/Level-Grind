#include "LoadingLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/actions/CCActionInstant.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/CCBlockLayer.hpp>

#include <UIBuilder.hpp>

namespace levelgrind {

LoadingLayer* LoadingLayer::create(std::string loadingLabel) {
    auto ret = new LoadingLayer;
    if (ret && ret->init(loadingLabel)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LoadingLayer::init(std::string loadingLabel) {
    if (!CCLayer::init()) return false;

    this->setKeyboardEnabled(true);
    this->setKeypadEnabled(true);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto blockLayer = CCBlockLayer::create();
    this->addChild(blockLayer);
    m_blockLayer = blockLayer;

    auto loadingSpinner = LoadingSpinner::create(30);
    this->addChild(loadingSpinner);
    loadingSpinner->setPosition(winSize.width / 2, winSize.height / 2);
    m_spinner = loadingSpinner;

    auto label = Build(CCLabelBMFont::create(loadingLabel.c_str(), "goldFont.fnt"))
        .scale(0.5f)
        .limitLabelWidth(110.f, 0.5f, 0.2f)
        .parent(this)
        .pos(winSize.width / 2.f, winSize.height / 2.f - 23.f)
        .collect();
    m_label = label;

    return true;
}

void LoadingLayer::show() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    if (!this->getParent()) {
        scene->addChild(this, 9999);
    }

    this->stopAllActions();
    if (m_blockLayer) m_blockLayer->setOpacity(0);
    if (m_spinner) m_spinner->setOpacity(0);
    if (m_label) m_label->setOpacity(0);

    auto fadeIn = [](CCNode* node) {
        if (!node) return;

        node->stopAllActions();
        node->runAction(CCSequence::create(
            CCFadeTo::create(0.f, 0),
            CCFadeTo::create(0.3f, 255),
            nullptr
        ));
    };

    auto fadeInHalf = [](CCNode* node) {
        if (!node) return;

        node->stopAllActions();
        node->runAction(CCSequence::create(
            CCFadeTo::create(0.f, 0),
            CCFadeTo::create(0.3f, 128),
            nullptr
        ));
    };

    fadeInHalf(m_blockLayer);
    fadeIn(m_spinner);
    fadeIn(m_label);
}

void LoadingLayer::hide() {
    if (!this->getParent()) {
        return;
    }

    this->stopAllActions();

    auto fadeOut = [](CCNode* node) {
        if (!node) return;

        node->stopAllActions();
        node->runAction(CCFadeTo::create(0.3f, 0));
    };

    fadeOut(m_blockLayer);
    fadeOut(m_spinner);
    fadeOut(m_label);

    this->runAction(CCSequence::create(
        CCDelayTime::create(0.3f),
        CCCallFunc::create(this, callfunc_selector(LoadingLayer::onHideFinished)),
        nullptr
    ));
}

void LoadingLayer::keyBackClicked() {
    hide();
}

void LoadingLayer::onHideFinished() {
    m_blockLayer = nullptr;
    m_spinner = nullptr;
    m_label = nullptr;
    this->removeFromParentAndCleanup(true);
}

}