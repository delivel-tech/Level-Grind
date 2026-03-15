#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../other/LGManager.hpp"
#include "../popups/ManageLevel.hpp"

using namespace geode::prelude;

class $modify(LevelGrinding, LevelInfoLayer) {
	struct Fields {
		bool isRated = false;
	};
	bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) return false;

		auto menu = getChildByIDRecursive("left-side-menu");
		auto spr = CCSprite::create("button_add_1.png"_spr);
		spr->setScale(0.847f);

		auto addLevelBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(LevelGrinding::onAddBtn)
		);

		if (m_level->m_stars.value() > 0) {
			m_fields->isRated = true;
		}

		if (!m_fields->isRated) {
			addLevelBtn->setColor({ 128, 128, 128 });
		}

		if (LGManager::get()->isHelper() || LGManager::get()->isAdmin()) {
			menu->addChild(addLevelBtn);
		    menu->updateLayout();
		}

		return true;
	}

	void onAddBtn(CCObject* sender) {
		if (m_fields->isRated) {
			ManageLevel::create(m_level, m_difficultySprite)->show();
		} else {
			FLAlertLayer::create(
				"Level Not Rated",
				"This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>.",
				"OK"
			)->show();
		}
	}
};