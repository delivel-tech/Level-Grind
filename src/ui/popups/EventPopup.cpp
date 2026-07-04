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
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>
#include "../../managers/APIClient.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

#include "../layers/CustomBrowserLayer.hpp"

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

static void fixCell(LevelCell* cell) {
    if (!cell || !cell->m_mainMenu) return;
    cell->m_mainMenu->setPosition({0, 0});
    auto viewButton    = cell->m_mainMenu->getChildByID("view-button");
    auto creatorButton = cell->m_mainMenu->getChildByID("creator-name");
    if (creatorButton) {
        creatorButton->setPosition({50, 65});
        creatorButton->setAnchorPoint({0.f, 0.5f});
    }
    if (viewButton) {
        viewButton->setPosition({
            cell->getContentSize().width - 40,
            cell->getContentSize().height / 2.f
        });
    }

    std::vector<std::string> ids {
        "length-icon", "downloads-icon", "likes-icon", "length-label", "downloads-label", "likes-label",
        "orbs-icon", "orbs-label"
    };

    for (const auto& id : ids) {
        if (auto element = cell->m_mainLayer->getChildByID(id)) {
            element->setPositionY(
                element->getPositionY() + 12.f
            );
        }
    }
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

    m_classicCell = new LevelCell("EventLevelCell", 380, 116);
    m_classicCell->setPosition({20, 65});
    m_classicCell->setScale(0.95f);
    m_classicCell->setContentSize({380, 116});
    m_classicCell->autorelease();
    m_classicCell->setID("classic-cell");
    m_mainLayer->addChild(m_classicCell);

    m_platCell = new LevelCell("EventLevelCell", 380, 116);
    m_platCell->setPosition({20, 65});
    m_platCell->setScale(0.95f);
    m_platCell->setContentSize({380, 116});
    m_platCell->autorelease();
    m_platCell->setID("plat-cell");
    m_mainLayer->addChild(m_platCell);

    m_platCell->setVisible(false);

    m_timer = CCLabelBMFont::create("Resets in: --:--:--:--", "goldFont.fnt");
    m_timer->setPosition(centerX(), 20);
    m_timer->setScale(0.6f);
    m_mainLayer->addChild(m_timer);

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
            CustomBrowserLayer::create(body, getName(type), CustomBrowserType::EventsHistory, type)->open();
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

    auto self = Ref(this);

    m_listener.spawn(
        APIClient::getInstance().getEvents(static_cast<int>(type)),
        [self](web::WebResponse res) {
            if (!self) return;

            auto parsed = APIClient::getInstance().getEventsParse(res);

            if (!parsed.ok) {
                Notification::create("Failed to load events.", NotificationIcon::Error)->show();
                return;
            }

            if (!parsed.classicEvent.exists || !parsed.platEvent.exists) {
                Notification::create("Failed to load events.", NotificationIcon::Error)->show();
                return;
            }

            self->m_secondsLeft = parsed.classicEvent.secondsLeft;

            auto glm = GameLevelManager::sharedState();
            if (!glm) return;

            if (parsed.classicEvent.levelId > 0 && parsed.platEvent.levelId > 0) {
                auto searchObj = GJSearchObject::create(
                    SearchType::Type19,
                    fmt::format("{},{}", parsed.classicEvent.levelId, parsed.platEvent.levelId)
                );
                auto key = std::string(searchObj->getKey());
                auto stored = glm->getStoredOnlineLevels(key.c_str());

                if (stored && stored->count() > 1) {
                    for (unsigned int i = 0; i < stored->count(); i++) {
                        auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
                        if (!lvl) continue;
                        
                        if (lvl->m_levelID == parsed.classicEvent.levelId && self->m_classicCell) {
                            self->m_classicCell->loadFromLevel(lvl);
                            fixCell(self->m_classicCell);
                            if (self->m_spinner) self->m_spinner->setVisible(false);
                        }
                        if (lvl->m_levelID == parsed.platEvent.levelId && self->m_platCell) {
                            self->m_platCell->loadFromLevel(lvl);
                            fixCell(self->m_platCell);
                            if (self->m_spinner) self->m_spinner->setVisible(false);
                        }
                    }
                } else {
                    self->m_pendingKey = key;
                    self->m_pendingClassicId = parsed.classicEvent.levelId;
                    self->m_pendingPlatId = parsed.platEvent.levelId;
                    glm->getOnlineLevels(searchObj);
                }
            }
        }
    );

    return true;
}

void EventPopup::update(float dt) {
    if (auto id = getChildByIDRecursive("cvolton.betterinfo/level-id-label")) id->removeFromParent();
    if (auto id = getChildByIDRecursive("cdc.level_thumbnails/separator")) id->removeFromParent();
    if (auto id = getChildByIDRecursive("cdc.level_thumbnails/clipping-node")) id->removeFromParent();
    if (m_secondsLeft > 0) m_secondsLeft -= dt;

    std::string prefix = "Resets in: ";
    if (m_timer) m_timer->setString((prefix + formatTime(m_secondsLeft)).c_str());

    if (m_pendingKey.empty()) return;

    auto glm = GameLevelManager::sharedState();
    if (!glm) return;

    auto stored = glm->getStoredOnlineLevels(m_pendingKey.c_str());
    if (!stored || stored->count() == 0) return;

    for (unsigned int i = 0; i < stored->count(); i++) {
        auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
        if (!lvl) continue;

        if (m_pendingClassicId != -1 && lvl->m_levelID == m_pendingClassicId && m_classicCell) {
            m_classicCell->loadFromLevel(lvl);
            fixCell(m_classicCell);
            if (m_spinner) m_spinner->setVisible(false);
            m_pendingClassicId = -1;
        }

        if (m_pendingPlatId != -1 && lvl->m_levelID == m_pendingPlatId && m_platCell) {
            m_platCell->loadFromLevel(lvl);
            fixCell(m_platCell);
            if (m_spinner) m_spinner->setVisible(false);
            m_pendingPlatId = -1;
        }
    }

    if (m_pendingClassicId == -1 && m_pendingPlatId == -1)
        m_pendingKey.clear();
}

}