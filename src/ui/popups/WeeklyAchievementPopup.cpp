#include "WeeklyAchievementPopup.hpp"
#include "../components/WeeklyAchievementCell.hpp"

#include <UIBuilder.hpp>
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/cocos.hpp"
#include "GuidePopup.hpp"

#include "../../core/BackendManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

WeeklyAchievementPopup* WeeklyAchievementPopup::create() {
    auto ret = new WeeklyAchievementPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchievementPopup::init() {
    if (!BasePopup::init(400.f, 270.f, "GJ_square01.png")) return false;

    auto title = Build<CCSprite>::create("weekly_ach_title.png"_spr)
        .parent(m_mainLayer)
        .scale(0.8f)
        .pos({
            centerX(),
            centerY() + 93
        })
        .id("title-sprite")
        .collect();

    auto darkBg = Build(NineSlice::create("square02_small.png"))
        .contentSize({
            m_mainLayer->getContentWidth() - 50.f, 170
        })
        .pos(centerX(), centerY() - 30)
        .opacity(80)
        .id("dark-bg")
        .zOrder(1)
        .parent(m_mainLayer)
        .collect();

    auto mainMenu = Build<CCMenu>::create()
        .pos({25, 20})
        .scale(1.05f)
        .contentSize(darkBg->getContentSize())
        .id("main-menu")
        .zOrder(2)
        .parent(m_buttonMenu)
        .collect();

    auto loading = Build(LoadingSpinner::create(50))
        .id("loading")
        .parent(mainMenu)
        .pos(mainMenu->getContentSize() / 2)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::WeeklyAchievementsGuide, GuidePopupState::FromOutside)->show();
        })
        .parent(m_buttonMenu)
        .pos(
            m_buttonMenu->getContentSize()
        )
        .collect();

    async::spawn(this->onLoadWeeklyAch());

    return true;
}

arc::Future<> WeeklyAchievementPopup::onLoadWeeklyAch() {
    LoadingSpinner* loading = typeinfo_cast<LoadingSpinner*>(m_buttonMenu->getChildByID("main-menu")->getChildByID("loading"));
    CCMenu* mainMenu = typeinfo_cast<CCMenu*>(m_buttonMenu->getChildByID("main-menu"));

    auto loadingRef = Ref(loading);
    Ref<WeeklyAchievementPopup> self = this;
    auto mainMenuRef = Ref(mainMenu);

    auto parsed = co_await BackendManager::getInstance().getWeeklyAch();

    if (!loadingRef || !self || !mainMenuRef) co_return;

    if (!parsed.ok) {
        loadingRef->removeFromParent();
        Notification::create("Failed to load weekly achievements!", NotificationIcon::Error)->show();
        co_return;
    }

    auto sequence = CCSequence::create(
        CallFuncExt::create([mainMenuRef, parsed]{
            auto cell = WeeklyAchievementCell::create(AchievementCellType::Third, parsed.cell3);
            cell->setScale(0.8f);
            mainMenuRef->addChild(cell);
            cell->setPosition({
                20, -10
            });
        }),
        CCDelayTime::create(0.2f),
        CallFuncExt::create([mainMenuRef, parsed] {
            auto cell = WeeklyAchievementCell::create(AchievementCellType::Second, parsed.cell2);
            cell->setScale(0.8f);
            mainMenuRef->addChild(cell);
            cell->setPosition({
                214, -10
            });
        }),
        CCDelayTime::create(0.2f),
        CallFuncExt::create([mainMenuRef, parsed] {
            auto cell = WeeklyAchievementCell::create(AchievementCellType::First, parsed.cell1);
            cell->setScale(0.8f);
            mainMenuRef->addChild(cell);
            cell->setPosition({
                117, -10
            });
        }),
        nullptr
    );

    loadingRef->runAction(
        CCSequence::create(
            CCScaleTo::create(0.2f, 0),
            CallFuncExt::create([loadingRef]{
                if (loadingRef) loadingRef->removeFromParent();
            }), nullptr
        )
    );

    self->runAction(sequence);

    co_return;
}

}