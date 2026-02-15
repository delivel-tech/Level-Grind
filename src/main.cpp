#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <argon/argon.hpp>
#include <fmt/format.h>
#include "LevelGrindLayer.hpp"
#include <Geode/modify/LevelInfoLayer.hpp>
#include "HelperPopup.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	async::spawn(
        argon::startAuth(),
        [](Result<std::string> res) {
            if (!res.ok()) return;
			auto token = std::move(res).unwrap();
			Mod::get()->setSavedValue("argon_token", token);
        }
    );
}

class $modify(LevelGrind, LevelSearchLayer) {
	bool init(int type) {
		if (!LevelSearchLayer::init(type)) return false;
		
		auto menu = getChildByIDRecursive("other-filter-menu");
		
		auto spr  = CircleButtonSprite::create(
			CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png")
		);

		spr->setScale(0.8f);

		auto grindingBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(LevelGrind::onGrindingBtn)
		);

		grindingBtn->setID("level-grind-btn");
		
		menu->addChild(grindingBtn);
		menu->updateLayout();

		return true;
	}

	void onGrindingBtn(CCObject*) {
		auto scene = CCScene::create();
		auto grindingLayer = LevelGrindLayer::create();
		scene->addChild(grindingLayer);

		auto transition = CCTransitionFade::create(0.5f, scene);
		CCDirector::sharedDirector()->pushScene(transition);
	}
};

class $modify(LevelGrinding, LevelInfoLayer) {
	struct Fields {
		GJGameLevel* m_currentLevel;
	};
	bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) return false;

		m_fields->m_currentLevel = level;

		auto menu = getChildByIDRecursive("left-side-menu");
		auto spr = CCSprite::create("button_add_1.png"_spr);
		spr->setScale(0.847f);

		auto addLevelBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(LevelGrinding::onAddBtn)
		);

		if (Mod::get()->getSavedValue<bool>("isHelper")) {
			menu->addChild(addLevelBtn);
		    menu->updateLayout();
		}

		return true;
	}

	void onAddBtn(CCObject* sender) {
		HelperPopup::create(m_fields->m_currentLevel)->show();
	}
};