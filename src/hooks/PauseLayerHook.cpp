#include <Geode/Geode.hpp>

#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../other/LGManager.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/utils/cocos.hpp"

using namespace geode::prelude;

class $modify(GrindPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto pl = PlayLayer::get();
        if (!pl || !pl->m_level) return;

        int levelId = pl->m_level->m_levelID;
        
        auto& notes = LGManager::get()->getNotes();

        auto it = notes.find(levelId);

        bool hasNote = it != notes.end();

        if (!hasNote) return;

        std::string levelNote = it->second;

        auto noteSpr = CircleButtonSprite::createWithSprite("button_note.png"_spr);
        noteSpr->setScale(0.625f);

        auto noteBtn = CCMenuItemExt::createSpriteExtra(
            noteSpr,
            [levelNote](CCObject* sender) {
                FLAlertLayer::create(
                    "Level Note",
                    levelNote.c_str(),
                    "OK"
                )->show();
            }
        );

        auto rightButtonMenu = getChildByIDRecursive("right-button-menu");
        if (!rightButtonMenu) return;

        rightButtonMenu->addChild(noteBtn);
        rightButtonMenu->updateLayout();

        return;
    }
};