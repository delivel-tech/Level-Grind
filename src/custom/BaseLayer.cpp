#include "BaseLayer.hpp"
#include "Geode/ui/General.hpp"
#include <cue/RepeatingBackground.hpp>

using namespace geode::prelude;

BaseLayer* BaseLayer::create() {
    auto ret = new BaseLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BaseLayer::init() {
    if (!CCLayer::init()) return false;

    this->setKeypadEnabled(true);

    if (Mod::get()->getSavedValue<bool>("disable-custom-background")) {
		auto bg = createLayerBG();
		bg->setColor({ 0, 102, 255 });
        addChild(bg, -1);
	} else {
		auto customBg = cue::RepeatingBackground::create("game_bg_01_001.png", 1.0f, cue::RepeatMode::X);
        customBg->setColor(Mod::get()->getSavedValue<cocos2d::ccColor3B>("rgbBackground"));
        customBg->setSpeed(Mod::get()->getSavedValue<float>("background-speed"));
		addChild(customBg, -1);
	}

    auto backBtn = addBackButton(this, BackButtonStyle::Green);
    m_backBtn = backBtn;

    addSideArt(this, SideArt::All, SideArtStyle::Layer, false);

    return true;
}

void BaseLayer::keyBackClicked() {
    this->onBack(nullptr);
}

void BaseLayer::onBack(CCObject* sender) {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}