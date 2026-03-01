#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../popups/HelperPopup.hpp"

using namespace geode::prelude;

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