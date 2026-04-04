#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../other/LGManager.hpp"
#include "../popups/ManageLevel.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/Popup.hpp"

using namespace geode::prelude;

class $modify(LevelGrinding, LevelInfoLayer) {
	struct Fields {
		bool isRated = false;
		std::string m_levelNote;
		bool m_noteShown = false;
	};
	void onPlay(CCObject* sender) {
		if (Mod::get()->getSavedValue<bool>("disable-auto-notes")) {
			m_fields->m_noteShown = true;
			LevelInfoLayer::onPlay(sender);
			return;
		}
    	if (m_fields->m_levelNote.empty() || m_fields->m_noteShown) {
        	LevelInfoLayer::onPlay(sender);
        	return;
    	}
   	 	m_fields->m_noteShown = true;
    	createQuickPopup(
        	"Level Note",
        	m_fields->m_levelNote.c_str(),
        	"Cancel", "Play",
        	[sender, this](auto, bool btn2) {
            	if (btn2) {
                	LevelInfoLayer::onPlay(sender);
            	}
            	m_fields->m_noteShown = false;
        	}	
    	);
    }
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

		auto it = LGManager::get()->getNotes().find(level->m_levelID);
		if (it != LGManager::get()->getNotes().end()) {
			std::string levelNote = it->second;
			m_fields->m_levelNote = levelNote;

			auto noteBtn = CCMenuItemSpriteExtra::create(
				CircleButtonSprite::createWithSprite("button_note.png"_spr),
				this,
				menu_selector(LevelGrinding::onNoteBtn)
			);

			menu->addChild(noteBtn);
			menu->updateLayout();
		}

		return true;
	}

	void onNoteBtn(CCObject* sender) {
		FLAlertLayer::create(
			"Level Note",
			m_fields->m_levelNote.c_str(),
			"OK"
		)->show();
	}

	void onAddBtn(CCObject* sender) {
		if (m_fields->isRated) {
			ManageLevel::create(m_level, m_difficultySprite)->show();
		} else {
			if (!LGManager::get()->isAdmin()) {
				FLAlertLayer::create(
					"Level Not Rated!",
					"This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>.",
					"OK"
				)->show();
			} else {
				createQuickPopup(
					"Level Not Rated!",
					"This level <cr>has not been rated</c>. You cannot add unrated levels to the <cy>Level Grind</c>. Do you <cp>still want to open</c>?",
					"Cancel", "Open",
					[&](auto, bool btn2) {
						if (btn2) {
							ManageLevel::create(m_level, m_difficultySprite)->show();
						}
					}
				);
			}
		}
	}
};