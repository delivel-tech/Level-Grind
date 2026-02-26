#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCTextInputNode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <argon/argon.hpp>
#include <fmt/format.h>
#include "LevelGrindLayer.hpp"
#include <Geode/modify/LevelInfoLayer.hpp>
#include "HelperPopup.hpp"
#include "UserManagePopup.hpp"
#include "ReqButtonSetting.hpp"
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
	registerReqButtonSettingType();

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

class $modify(LevelGrinding, LevelInfoLayer) {
	struct Fields {
		GJGameLevel* m_currentLevel;
		bool isRated = false;
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

		if (m_fields->m_currentLevel->m_stars.value() > 0) {
			m_fields->isRated = true;
		}

		if (!m_fields->isRated) {
			addLevelBtn->setColor({ 128, 128, 128 });
		}

		if (Mod::get()->getSavedValue<bool>("isHelper") || Mod::get()->getSavedValue<bool>("isAdmin")) {
			menu->addChild(addLevelBtn);
		    menu->updateLayout();
		}

		return true;
	}

	void onAddBtn(CCObject* sender) {
		if (m_fields->isRated) {
			HelperPopup::create(m_fields->m_currentLevel)->show();
		} else {
			FLAlertLayer::create(
				"Level Not Rated",
				"This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>.",
				"OK"
			)->show();
		}
	}
};

class $modify(UserManage, ProfilePage) {
	struct Fields {
		int m_targetAccountID;
		const char* m_username;
	};
	void loadPageFromUserInfo(GJUserScore* score) {
		ProfilePage::loadPageFromUserInfo(score);

		if (auto mngBtn = getChildByIDRecursive("lg-manage-btn")) mngBtn->removeFromParent();

		m_fields->m_targetAccountID = score->m_accountID;
		m_fields->m_username = score->m_userName.c_str();

		auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

		auto spr = CCSprite::create("button_add_2.png"_spr);
		spr->setScale(0.55f);

		auto manageBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(UserManage::onManageBtn)
		);
		manageBtn->setID("lg-manage-btn");

		if (!Mod::get()->getSavedValue<bool>("isAdmin")) return;

		leftMenu->addChild(manageBtn);
		leftMenu->updateLayout();
	}

	void onManageBtn(CCObject* sender) {
		UserManagePopup::create(m_fields->m_targetAccountID, m_fields->m_username)->show();
	}
};
