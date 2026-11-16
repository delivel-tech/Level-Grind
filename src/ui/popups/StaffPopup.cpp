#include "ClanPopups.hpp"
#include "StaffPopup.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include <UIBuilder.hpp>

#include "Geode/ui/BasedButtonSprite.hpp"
#include "SyncLevelsPopup.hpp"
#include "EventsManagePopup.hpp"
#include "StaffActivityPopup.hpp"
#include "ReportPopups.hpp"

namespace levelgrind {

StaffPopup* StaffPopup::create() {
    auto ret = new StaffPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StaffPopup::init() {
    if (!BasePopup::init(270.f, 160.f, "GJ_square01.png")) return false;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Helper) {
        this->setTitle("Grind Helper: Staff Panel");
    } else if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Staff Panel");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Coordinator: Staff Panel");
    } else {
        this->setTitle("Unknown Role: Staff Panel");
    }

    if (pos != GrindPosition::Helper && pos != GrindPosition::Admin && pos != GrindPosition::Owner) {
        Notification::create("You cannot use staff panel!", NotificationIcon::Error)->show();
        return false;
    }

    auto importantToolsLabel = Build(CCLabelBMFont::create("Important Tools", "bigFont.fnt"))
        .parent(m_mainLayer)
        .pos({m_title->getPositionX(), m_title->getPositionY() - 20 })
        .scale(0.5f)
        .id("important-tools-label")
        .zOrder(1)
        .collect();

    auto menuBg = Build(NineSlice::create("square02_small.png"))
        .contentSize(m_mainLayer->getContentWidth() - 45.f, 90)
        .pos(centerX(), centerY() - 20)
        .id("menu-bg")
        .scale(0.9f)
        .opacity(80)
        .parent(m_mainLayer)
        .collect();

    auto buttonsMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos(centerX(), centerY() - 20)
        .id("buttons-menu")
        .scale(0.75f)
        .contentSize(m_mainLayer->getContentWidth() - 50.f, 80)
        .layout(
            RowLayout::create()
                    ->setGap(7)
                    ->setGrowCrossAxis(true)
        )
        .zOrder(1)
        .collect();

    Build<CCSprite>::createSpriteName("GJ_levelLeaderboardBtn_001.png")
        .intoMenuItem([] {
            StaffActivityPopup::create()->show();
        })
        .id("activity-btn")
        .parent(buttonsMenu);

    Build<CCSprite>::createSpriteName("GJ_reportBtn_001.png")
        .intoMenuItem([] {
            StaffReportsPopup::create()->show();
        })
        .id("reports-btn")
        .parent(buttonsMenu);

    Build(CircleButtonSprite::createWithSpriteFrameName("GJ_sFriendsIcon_001.png", 0.8f))
        .intoMenuItem([] {
            StaffLogosPopup::create()->show();
        })
        .id("clan-logos-btn")
        .parent(buttonsMenu);

    if (pos == GrindPosition::Owner) {
        auto syncBtn = Build(CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png"))
            .intoMenuItem([] {
                SyncLevelsPopup::create()->show();
            })
            .id("sync-btn")
            .parent(buttonsMenu)
            .collect();
    }

    if (pos == GrindPosition::Admin || pos == GrindPosition::Owner) {
        Build(CircleButtonSprite::createWithSpriteFrameName("gj_dailyCrown_001.png"))
            .intoMenuItem([] {
                EventsManagePopup::create()->show();
            })
            .id("events-btn")
            .parent(buttonsMenu);
    }

    buttonsMenu->updateLayout();

    return true;
}

}