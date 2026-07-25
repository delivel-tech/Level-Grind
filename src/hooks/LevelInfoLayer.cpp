#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../ui/popups/ManageLevelPopup.hpp"
#include "../managers/DataManager.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/Notification.hpp"

#include <UIBuilder.hpp>
#include "../ui/popups/NoteViewerPopup.hpp"
#include "../ui/popups/IndicatorsPopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class $modify(LevelGrind, LevelInfoLayer) {
    struct Fields {
        bool shownAutoNote = false;
    };
    void onPlay(CCObject* sender) {
        bool autoNote = Mod::get()->getSavedValue<bool>("auto-notes");

        if (autoNote) {
        if (!this->m_fields->shownAutoNote) {
            auto range = DataManager::getInstance().getSharedData().notes.equal_range(this->m_level->m_levelID);

            std::vector<NoteInfo> notes_vec;

            for (auto it = range.first; it != range.second; ++it) {
                notes_vec.push_back(it->second);
            }

            if (notes_vec.empty()) {
                LevelInfoLayer::onPlay(sender);
            } else {
                this->m_fields->shownAutoNote = true;
                NoteViewerPopup::create(notes_vec)->show();
            }
        } else {
            LevelInfoLayer::onPlay(sender);
        }
        } else {
            LevelInfoLayer::onPlay(sender);
        }
    }
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto leftSideMenu = this->getChildByIDRecursive("left-side-menu");
        log::warn("left side menu not found");
        if (!leftSideMenu) return true;

        auto range = DataManager::getInstance().getSharedData().notes.equal_range(level->m_levelID);

        std::vector<NoteInfo> notes_vec;

        for (auto it = range.first; it != range.second; ++it) {
            notes_vec.push_back(it->second);
        }

        if (!notes_vec.empty()) {
            Build(CircleButtonSprite::createWithSprite("button_note.png"_spr))
                .intoMenuItem([notes_vec] {
                    NoteViewerPopup::create(notes_vec)->show();
                })
                .parent(leftSideMenu)
                .intoParent()
                .updateLayout();
        }

        auto titleLabel = this->getChildByID("title-label");
        auto dailyLabel = this->getChildByID("daily-label");

        if (titleLabel && Mod::get()->getSavedValue<bool>("enable-indicators")) {
            auto eventGap = dailyLabel ? dailyLabel->getScaledContentWidth() : 0;
            auto infoBtnMenu = Build(CCMenu::create())
                .parent(this)
                .id("indicators-btn-menu"_spr)
                .pos({
                    this->getContentWidth() / 2 + 13 + eventGap + titleLabel->getScaledContentWidth() / 2,
                    titleLabel->getPositionY()
                })
                .collect();

            Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
                .scale(0.6f)
                .intoMenuItem([this] {
                    IndicatorsPopup::create(this->m_level->m_levelID)->show();
                })
                .parent(infoBtnMenu)
                .id("indicators-btn"_spr);
        }
        
        // staff things code below
        GrindPosition userPos = DataManager::getInstance().getUserPosition();

        if (userPos != GrindPosition::Helper
        && userPos != GrindPosition::Admin
        && userPos != GrindPosition::Owner) return true;

        Build<CCSprite>::create("button_add_1.png"_spr)
            .scale(0.847f)
            .intoMenuItem([this, userPos] {
                bool isRated = this->m_level->m_stars.value() > 0;
                if (isRated) {
                    if (userPos != GrindPosition::User) ManageLevelPopup::create(this->m_level)->show();
                    else Notification::create("You cannot open manage level popup.", NotificationIcon::Error)->show();
                } else {
                    if (userPos == GrindPosition::Helper || userPos == GrindPosition::User) {
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
							        ManageLevelPopup::create(this->m_level)->show();
						        }
					        }
				        );
                    }
                }
            })
            .with([this](CCMenuItemSpriteExtra* btn) {
                bool isRated = this->m_level->m_stars.value() > 0;
                if (!isRated) btn->setColor({128, 128, 128});
            })
            .parent(leftSideMenu)
            .intoParent()
            .updateLayout();

        return true;
    }
};

}