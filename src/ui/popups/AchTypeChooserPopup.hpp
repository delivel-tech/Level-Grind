#pragma once

#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "../components/WeeklyAchCandidateCell.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include <cue/RadioLogic.hpp>

#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

class AchTypeChooserPopup : public BasePopup {
public:
    static AchTypeChooserPopup* create(WeeklyAchCandidateCell* owner) {
        auto ret = new AchTypeChooserPopup;
        if (ret && ret->init(owner)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(WeeklyAchCandidateCell* owner) {
        if (!BasePopup::init({260, 170})) return false;

        setTitle("Choose Type");

        enum class TypeStates {
            Star = 0, Moon = 1, Coin = 2, Demon = 3
        };

        cue::RadioLogic<TypeStates> radio;

        auto rowMenu = Build(CCMenu::create())
            .layout(RowLayout::create()->setGap(10))
            .id("row-menu")
            .parent(m_buttonMenu)
            .pos({centerX(), centerY() - 10})
            .collect();

        auto starToggler = Build(radio.createToggler(
            TypeStates::Star, 
            CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"),
            Build(CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png")).color({100, 100, 100}).collect()
        ))
            .id("star-toggler")
            .parent(rowMenu)
            .collect();

        auto moonToggler = Build(
            radio.createToggler(
                TypeStates::Moon,
                CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png"),
                Build(CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png")).color({100, 100, 100}).collect()
            )
        )
            .id("moon-toggler")
            .parent(rowMenu)
            .collect();

        auto coinToggler = Build(
            radio.createToggler(
                TypeStates::Coin,
                CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png"),
                Build(CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png")).color({100, 100, 100}).collect()
            )
        )
            .id("coin-toggler")
            .parent(rowMenu)
            .collect();

        auto demonToggler = Build(
            radio.createToggler(
                TypeStates::Demon,
                CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png"),
                Build(CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png")).color({100, 100, 100}).collect()
            )
        )
            .id("demon-toggler")
            .parent(rowMenu)
            .collect();

        radio.setCallback([owner](TypeStates which) {
            owner->m_type = static_cast<int>(which);
        });
        radio.select(static_cast<TypeStates>(owner->m_type));

        rowMenu->updateLayout();

        return true;
    }
};

}