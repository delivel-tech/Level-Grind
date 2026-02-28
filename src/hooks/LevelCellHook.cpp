#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/modify/LevelCell.hpp>

#include "../custom/LGLevelBrowserLayer.hpp"
#include "../custom/LGLevelInfoLayer.hpp"

using namespace geode::prelude;

class $modify(LCHook, LevelCell) {
    void onClick(CCObject* sender) {
        auto mainLayer = getChildByIDRecursive("main-layer");

        if (!mainLayer) {
            LevelCell::onClick(sender);
            log::error("failed to find main layer in LevelCell");
            return;
        }

        auto thisLevelCell = mainLayer->getParent();

        if (!thisLevelCell) {
            LevelCell::onClick(sender);
            log::error("failed to find LevelCell in LevelCell");
            return;
        }

        auto thisContentLayer = thisLevelCell->getParent();

        if (!thisContentLayer) {
            LevelCell::onClick(sender);
            log::error("failed to find content layer in LevelCell");
            return;
        }

        auto scrollLayer = thisContentLayer->getParent();

        if (!scrollLayer) {
            LevelCell::onClick(sender);
            log::error("failed to find scroll layer in LevelCell");
            return;
        }

        auto thisGJListLayer = scrollLayer->getParent();

        if (!thisGJListLayer) {
            LevelCell::onClick(sender);
            log::error("failed to find GJListLayer in LevelCell");
            return;
        }

        auto browserLayer = thisGJListLayer->getParent();

        if (!browserLayer) {
            LevelCell::onClick(sender);
            log::error("failed to find browser layer in LevelCell");
            return;
        }

        if (auto lgBrowserLayer = typeinfo_cast<LGLevelBrowserLayer*>(browserLayer)) {
            auto scene = CCScene::create();

            auto layer = LGLevelInfoLayer::create(this->m_level, false);

            scene->addChild(layer);

            auto transition = CCTransitionFade::create(0.5f, scene);

            CCDirector::sharedDirector()->pushScene(transition);
        } else {
            LevelCell::onClick(sender);
        }

        return;
    }
};
