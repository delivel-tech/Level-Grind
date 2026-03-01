#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

using namespace geode::prelude;

#include "../custom/LevelGrindLayer.hpp"

class $modify(LevelGrind, LevelSearchLayer) {
	bool init(int type) {
		if (!LevelSearchLayer::init(type)) return false;
		
		auto menu = getChildByIDRecursive("other-filter-menu");
		
		auto spr = CCSprite::create("search_btn_1.png"_spr);

		spr->setScale(0.65f);

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
		auto searchBar = typeinfo_cast<CCTextInputNode*>(getChildByIDRecursive("search-bar"));
		if (searchBar) {
			searchBar->onClickTrackNode(false);
		}
		auto scene = CCScene::create();
		auto grindingLayer = LevelGrindLayer::create();
		scene->addChild(grindingLayer);

		auto transition = CCTransitionFade::create(0.5f, scene);
		CCDirector::sharedDirector()->pushScene(transition);
	}
};