#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include "../../utils/globals.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/ProgressBar.hpp"
#include "ccTypes.h"

#include <Geode/binding/LevelBrowserLayer.hpp>
#include <UIBuilder.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "../layers/PackBrowserLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GrindPackCell : public CCNode {
public:
    static GrindPackCell* create(GrindPack packInfo) {
        auto ret = new GrindPackCell;
        if (ret && ret->init(packInfo)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(GrindPack packInfo) {
        if (!CCNode::init()) return false;

        this->setContentSize({ 356.f, 60.f });

        auto infoMenu = Build(CCMenu::create())
            .parent(this)
            .pos({
                15,
                (this->getContentHeight() / 2) + 12
            })
            .layout(RowLayout::create()->setGap(10)->setAutoGrowAxis(true)->setAutoScale(false))
            .anchorPoint({ 0, 0.5f })
            .id("info-menu")
            .scale(0.7f)
            .collect();

        auto otherMenu = Build(CCMenu::create())
            .parent(this)
            .pos({
                15,
                (this->getContentHeight() / 2) - 12
            })
            .layout(RowLayout::create()->setGap(10)->setAutoGrowAxis(true))
            .anchorPoint({ 0, 0.5f })
            .id("other-menu")
            .scale(1.f)
            .collect();

        auto progressBar = Build(ProgressBar::create(ProgressBarStyle::Solid))
            .parent(otherMenu)
            .with([packInfo](ProgressBar* bar) {
                bar->setFillColor(packInfo.color);
            })
            .anchorPoint({ 0, 0.5f })
            .scale(0.7f)
            .collect();

        size_t i = 0;
        
        if (GameStatsManager::get()->hasCompletedOnlineLevel(packInfo.levels.id1)) i++;
        if (GameStatsManager::get()->hasCompletedOnlineLevel(packInfo.levels.id2)) i++;
        if (GameStatsManager::get()->hasCompletedOnlineLevel(packInfo.levels.id3)) i++;

        int finalPercentage = 0;
        if (i == 1) finalPercentage = 33;
        else if (i == 2) finalPercentage = 66;
        else if (i == 3) finalPercentage = 100;

        progressBar->updateProgress(finalPercentage);

        std::string completed = fmt::format("{}/3", i);

        auto completedLabel = Build(CCLabelBMFont::create(completed.c_str(), "bigFont.fnt"))
            .scale(0.6f)
            .parent(progressBar)
            .center()
            .collect();

        if (i == 3) {
            auto completedSpr = Build(CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))
                .parent(progressBar)
                .pos({ progressBar->getContentWidth() - 20.f, progressBar->getContentHeight() / 2 })
                .collect();
        }

        auto difficultySprite = Build(CCSprite::createWithSpriteFrameName([packInfo] -> const char* {
            switch (packInfo.difficulty) {
                case levelgrind::CustomDifficultyEnum::Auto: return "diffIcon_auto_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::Easy: return "diffIcon_01_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::Normal: return "diffIcon_02_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::Hard: return "diffIcon_03_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::Harder: return "diffIcon_04_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::Insane: return "diffIcon_05_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::EasyDemon: return "diffIcon_07_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::MediumDemon: return "diffIcon_08_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::HardDemon: return "diffIcon_06_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::InsaneDemon: return "diffIcon_09_btn_001.png"; break;
                case levelgrind::CustomDifficultyEnum::ExtremeDemon: return "diffIcon_10_btn_001.png"; break;
                default: return "diffIcon_00_btn_001.png";
            }
        }()))
            .parent(infoMenu)
            .collect();

        auto titleLabel = Build(CCLabelBMFont::create(packInfo.title.c_str(), "bigFont.fnt"))
            .limitLabelWidth(270, 1.f, 0.4f)
            .color(packInfo.color)
            .parent(infoMenu)
            .collect();

        auto packTypeSpr = Build(CCSprite::createWithSpriteFrameName(packInfo.isStar ? "GJ_bigStar_noShadow_001.png" : "GJ_bigMoon_noShadow_001.png"))
            .scale(0.6f)
            .parent(infoMenu)
            .collect();

        auto viewBtnMenu = Build(CCMenu::create())
            .parent(this)
            .pos(
                (this->getContentWidth() / 2.f + progressBar->getScaledContentWidth() / 1.5f) - 25,
                this->getContentHeight() / 2.f
            )
            .id("view-btn-menu")
            .collect();
            
        auto viewBtn = Build(ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png"))
            .scale(0.7f)
            .intoMenuItem([packInfo] {
                auto layer = PackBrowserLayer::create(packInfo.title, GJSearchObject::create(SearchType::Type19, fmt::format("{},{},{}", 
                packInfo.levels.id1, packInfo.levels.id2, packInfo.levels.id3)));
                
                auto scene = CCScene::create();
                scene->addChild(layer);

                auto transition = CCTransitionFade::create(0.5f, scene);
                CCDirector::sharedDirector()->pushScene(transition);
            })
            .parent(viewBtnMenu)
            .collect();

        infoMenu->updateLayout();
        otherMenu->updateLayout();

        return true;
    }
};

}