#include "IndicatorsPopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"

#include "../../core/BackendManager.hpp"
#include "Geode/ui/Notification.hpp"

#include "../../core/GuidePopup.hpp"

#include <UIBuilder.hpp>

namespace levelgrind {

IndicatorsPopup* IndicatorsPopup::create(int levelID) {
    auto ret = new IndicatorsPopup;
    if (ret && ret->init(levelID)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IndicatorsPopup::init(int levelID) {
    if (!BasePopup::init({285, 160})) return false;

    this->setTitle("Grind Indicators");

    m_statusMenu = Build<CCMenu>::create()
        .parent(m_mainLayer)
        .pos({m_title->getPositionX(), m_title->getPositionY() - 20 })
        .id("status-menu")
        .scale(0.9f)
        .contentSize(m_mainLayer->getContentWidth() - 20.f, 30.f)
        .layout(
            RowLayout::create()
                ->setGap(6)
                ->setAutoScale(false)
        )
        .zOrder(1)
        .collect();

    m_menuBG = Build(NineSlice::create("square02_small.png"))
        .contentSize({m_mainLayer->getContentWidth() - 45, 90})
        .pos(centerX(), centerY() - 20)
        .id("menu-bg")
        .scale(0.9f)
        .opacity(80)
        .parent(m_mainLayer)
        .collect();

    auto statusLabel = Build(CCLabelBMFont::create(
        "Status:", "bigFont.fnt"
    ))
        .id("status-label")
        .scale(0.5f)
        .parent(m_statusMenu)
        .collect();

    m_loadingSpinner = Build(LoadingSpinner::create(50))
        .id("spinner")
        .parent(m_menuBG)
        .zOrder(1)
        .pos(m_menuBG->getContentSize() / 2)
        .collect();

    m_statusSpinner = Build(LoadingSpinner::create(20))
        .id("status-spinner")
        .parent(m_statusMenu)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::LevelIndicatorsGuide, GuidePopupState::FromOutside)->show();
        })
        .parent(m_buttonMenu)
        .pos(
            m_buttonMenu->getContentSize()
        )
        .collect();

    m_statusMenu->updateLayout();

    async::spawn(this->onLoadIndicators(levelID));

    return true;
}

arc::Future<> IndicatorsPopup::onLoadIndicators(int levelID) {
    Ref<IndicatorsPopup> self = this;

    auto parsed = co_await BackendManager::getInstance().getIndicators(levelID);

    if (!self) co_return;

    if (!parsed.ok) {
        self->m_loadingSpinner->removeFromParent();
        self->m_loadingSpinner = nullptr;

        self->m_statusSpinner->removeFromParent();
        self->m_statusSpinner = nullptr;

        Notification::create("Failed to load indicators!", NotificationIcon::Error)->show();

        co_return;
    }

    self->m_statusSpinner->removeFromParent();
    self->m_statusSpinner = nullptr;

    if (parsed.added) {
        Build(CCLabelBMFont::create("Added", "bigFont.fnt"))
            .scale(0.5f)
            .parent(self->m_statusMenu)
            .id("added-label");

        Build(CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))
            .parent(self->m_statusMenu)
            .scale(0.5f)
            .id("added-icon");
    } else {
        Build(CCLabelBMFont::create("Not Added", "bigFont.fnt"))
            .parent(self->m_statusMenu)
            .scale(0.5f)
            .id("not-added-label");

        Build(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"))
            .parent(self->m_statusMenu)
            .scale(0.5f)
            .id("not-added-icon");
    }

    self->m_statusMenu->updateLayout();

    self->m_loadingSpinner->removeFromParent();
    self->m_loadingSpinner = nullptr;

    self->m_indicatorsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .parent(self->m_menuBG)
        .id("indicators-menu")
        .pos(self->m_menuBG->getContentSize() / 2)
        .zOrder(1)
        .scale(0.9f)
        .collect();

    auto coinMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setAxisReverse(true)->setAutoScale(false))
        .parent(self->m_indicatorsMenu)
        .collect();

    auto eventMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setAxisReverse(true)->setAutoScale(false))
        .parent(self->m_indicatorsMenu)
        .collect();

    auto packMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setAxisReverse(true)->setAutoScale(false))
        .parent(self->m_indicatorsMenu)
        .collect();

    Build(CCSprite::create("coin_indicator.png"_spr))
        .scale(0.35f)
        .parent(coinMenu);

    Build(
        parsed.coin ? CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png") : CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png")
    )
        .parent(coinMenu);

    Build(CCSprite::create("event_indicator.png"_spr))
        .scale(0.35f)
        .parent(eventMenu);

    Build(
        parsed.event ? CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png") : CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png")
    )
        .parent(eventMenu);

    Build(CCSprite::create("pack_indicator.png"_spr))
        .scale(0.35f)
        .parent(packMenu);

    Build(
        parsed.pack ? CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png") : CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png")
    )
        .parent(packMenu);

    coinMenu->updateLayout();
    eventMenu->updateLayout();
    packMenu->updateLayout();

    self->m_indicatorsMenu->updateLayout();

    co_return;
}

}