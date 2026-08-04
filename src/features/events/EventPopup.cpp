#include "EventPopup.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"

#include <Geode/Enums.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>
#include "../../core/BackendManager.hpp"
#include "Geode/ui/Notification.hpp"

#include "../../core/GuidePopup.hpp"

#include "../../core/CustomBrowserLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

EventPopup* EventPopup::create(EventType type) {
    auto ret = new EventPopup;
    if (ret && ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

static std::string formatTime(float seconds) {
    if (seconds < 0) seconds = 0;
    long s = (long)seconds;
    long d = s / 86400; s %= 86400;
    long h = s / 3600;  s %= 3600;
    long m = s / 60;    s %= 60;
    char buf[32];
    sprintf(buf, "%02ld:%02ld:%02ld:%02ld", d, h, m, s);
    return buf;
}

bool EventPopup::init(EventType type) {
    auto bg = [](EventType type) {
        if (type == EventType::Daily) return "GJ_square01.png";
        else if (type == EventType::Weekly) return "GJ_square04.png";
        else return "GJ_square05.png";
    };
    if (!BasePopup::init({ 420.f, 280.f }, bg(type))) return false;

    m_type = type;

    auto cs = m_mainLayer->getContentSize();

    m_noElasticity = true;

    auto titleType = [](EventType type) {
        if (type == EventType::Daily) return CCSprite::create("daily_title.png"_spr);
        else if (type == EventType::Weekly) return CCSprite::create("weekly_title.png"_spr);
        else return CCSprite::create("monthly_title.png"_spr);
    };

    m_title = titleType(type);

    m_title->setPosition({ cs.width / 2.f, cs.height - 30.f });
    m_mainLayer->addChild(m_title);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_mainLayer->setPositionX(winSize.width * -0.5f);

    auto sequence = CCSequence::create(
        CCEaseElasticOut::create(CCMoveTo::create(0.5f, {winSize.width * 0.5f, winSize.height / 2}), 0.85),
        nullptr
    );

    auto artStyle = [](EventType type) {
        if (type == EventType::Daily) return SideArtStyle::PopupGold;
        else if (type == EventType::Weekly) return SideArtStyle::PopupBlue;
        else return SideArtStyle::PopupGold;
    };

    addSideArt(
        m_mainLayer,
        SideArt::Bottom,
        artStyle(type),
        false
    );

    addSideArt(
        m_mainLayer,
        SideArt::TopLeft,
        artStyle(type),
        false
    );

    m_mainLayer->runAction(sequence);

    auto containerStyle = [](EventType type) {
        if (type == EventType::Daily) return "GJ_square02.png";
        else if (type == EventType::Weekly) return "GJ_square01.png";
        else return "GJ_square01.png";
    };

    CCSize containerSize = {380, 116};

    m_eventContainer = Build(NineSlice::create(containerStyle(type)))
        .contentSize(containerSize)
        .parent(m_mainLayer)
        .pos(centerX(), centerY() - 17)
        .collect();

    auto crownStyle = [](EventType type) {
        if (type == EventType::Daily) return "crown_d.png"_spr;
        else if (type == EventType::Weekly) return "crown_w.png"_spr;
        else return "crown_m.png"_spr;
    };

    auto crown = Build(CCSprite::create(crownStyle(type)))
        .parent(m_mainLayer)
        .pos({centerX(), centerY() + 49})
        .scale(0.656f)
        .zOrder(1)
        .collect();

    m_classicCell = EventLevelCell::create();
    m_classicCell->setPosition({30, 68});
    m_classicCell->setScale(0.95f);
    m_classicCell->setID("classic-cell");
    m_mainLayer->addChild(m_classicCell);

    m_platCell = EventLevelCell::create();
    m_platCell->setPosition({30, 68});
    m_platCell->setScale(0.95f);
    m_platCell->setID("plat-cell");
    m_mainLayer->addChild(m_platCell);

    m_platCell->setVisible(false);

    m_timer = CCLabelBMFont::create("Resets in: --:--:--:--", "goldFont.fnt");
    m_timer->setPosition(centerX(), 20);
    m_timer->setScale(0.6f);
    m_mainLayer->addChild(m_timer);

    auto infoBtn = Build(CCSprite::create("info_btn.png"_spr))
        .scale(0.65f)
        .intoMenuItem([type]{
            GuidePopup::create(GuidePage::EventsGuide, GuidePopupState::FromOutside)->show();
        })
        .parent(m_buttonMenu)
        .pos({
            m_buttonMenu->getContentWidth() - 25,
            25
        })
        .collect();

    m_spinner = Build(LoadingSpinner::create(50))
        .id("spinner")
        .parent(m_eventContainer)
        .center()
        .collect();

    auto safeBtn = Build(CCSprite::createWithSpriteFrameName("GJ_safeBtn_001.png"))
        .intoMenuItem([type]{
            GetLevelsBody body; // placeholder
            auto getName = [](EventType type) {
                if (type == EventType::Daily) return "Dailies Safe";
                else if (type == EventType::Weekly) return "Weeklies Safe";
                else return "Monthlies Safe";
            };

            #ifdef GEODE_IS_WINDOWS
                CustomBrowserLayer::create(body, getName(type), CustomBrowserType::EventsHistory, type)->open();
            #else
                geode::createQuickPopup(
                    "Warning",
                    "Events Safe is currently only tested on Windows.\n"
                    "Opening it on mobile may cause crashes or other unexpected behavior.\n"
                    "Continue anyway?",
                    "Cancel", "Open",
                    [type, getName](auto, bool yes) {
                        GetLevelsBody body;
                        if (yes) CustomBrowserLayer::create(body, getName(type), CustomBrowserType::EventsHistory, type)->open();
                    }
                );
            #endif
        })
        .id("safe-btn")
        .pos(fromTopRight({30, 30}))
        .parent(m_buttonMenu)
        .collect();

    m_arrowLeft = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"))
        .intoMenuItem([this] {
            if (m_arrowRight) {
                m_arrowRight->setEnabled(true);
                m_arrowRight->setColor({255, 255, 255});
            }

            if (m_platCell) m_platCell->setVisible(false);
            if (m_classicCell) m_classicCell->setVisible(true);

            if (m_arrowLeft) {
                m_arrowLeft->setEnabled(false);
                m_arrowLeft->setColor({100, 100, 100});
            }
        })
        .id("arrow-left")
        .parent(m_buttonMenu)
        .pos({-20, centerY()})
        .collect();

    m_arrowLeft->setColor({100, 100, 100});
    m_arrowLeft->setEnabled(false);

    m_arrowRight = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"))
        .flipX(true)
        .intoMenuItem([this] {
            if (m_arrowLeft) {
                m_arrowLeft->setEnabled(true);
                m_arrowLeft->setColor({255, 255, 255});
            }

            if (m_platCell) m_platCell->setVisible(true);
            if (m_classicCell) m_classicCell->setVisible(false);

            if (m_arrowRight) {
                m_arrowRight->setEnabled(false);
                m_arrowRight->setColor({100, 100, 100});
            }
        })
        .id("arrow-right")
        .parent(m_buttonMenu)
        .pos({m_buttonMenu->getContentWidth() + 20, centerY()})
        .collect();

    this->scheduleUpdate();

    async::spawn(this->onLoadEvents(type));

    return true;
}

arc::Future<> EventPopup::onLoadEvents(EventType type) {
    Ref<EventPopup> self;
    co_await async::waitForMainThread([&] {
        self = this;
    });

    auto parsed = co_await BackendManager::getInstance().getEvents(static_cast<int>(type));

    co_await async::waitForMainThread([&] {
        if (!self) return;

        if (!parsed.ok) {
            Notification::create("Failed to load events.", NotificationIcon::Error)->show();
            return;
        }

        if (!parsed.classicEvent.exists || !parsed.platEvent.exists) {
            Notification::create("Failed to load events.", NotificationIcon::Error)->show();
            return;
        }

        self->m_secondsLeft = parsed.classicEvent.secondsLeft;

        if (parsed.classicEvent.levelId > 0 && self->m_classicCell) {
            self->m_classicCell->loadLevel(parsed.classicEvent.levelId);
        }
        if (parsed.platEvent.levelId > 0 && self->m_platCell) {
            self->m_platCell->loadLevel(parsed.platEvent.levelId);
        }
        if (self->m_spinner) self->m_spinner->setVisible(false);
    });

    co_return;
}

void EventPopup::update(float dt) {
    if (m_secondsLeft > 0) m_secondsLeft -= dt;

    std::string prefix = "Resets in: ";
    if (m_timer) m_timer->setString((prefix + formatTime(m_secondsLeft)).c_str());
}

}