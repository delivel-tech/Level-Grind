#include "EventLevelPopup.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDPopup.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/ui/NineSlice.hpp>

using namespace geode::prelude;

EventLevelPopup* EventLevelPopup::create(GrindEventType type) {
    auto ret = new EventLevelPopup;
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
        creatorButton->setPosition({50, 54});
        creatorButton->setAnchorPoint({0.f, 0.5f});
    }
    if (viewButton) {
        viewButton->setPosition({
            cell->getContentSize().width,
            cell->getContentSize().height / 2.f
        });
    }
}

bool EventLevelPopup::init(GrindEventType type) {
    if (!Popup::init(420.f, 280.f)) return false;

    m_type = type;
    auto cs = m_mainLayer->getContentSize();

    m_noElasticity = true;

    m_title = m_type == GrindEventType::Daily
        ? CCSprite::create("daily_level.png"_spr)
        : CCSprite::create("weekly_level.png"_spr);
    m_title->setPosition({ cs.width / 2.f, cs.height - 25.f });
    m_title->setScale(0.85f);
    m_mainLayer->addChild(m_title);

    addSideArt(
        m_mainLayer,
        SideArt::All,
        SideArtStyle::PopupGold,
        false
    );

    auto infoSpr = CCSprite::create("info_btn.png"_spr);
    infoSpr->setScale(0.7f);

    auto infoBtn = CCMenuItemSpriteExtra::create(
        infoSpr,
        this,
        menu_selector(EventLevelPopup::onInfoBtn)
    );
    infoBtn->setID("info-btn");

    auto infoBtnMenu = CCMenu::create();
    infoBtnMenu->setID("info-btn-menu");
    infoBtnMenu->addChild(infoBtn);

    m_mainLayer->addChildAtPosition(
        infoBtnMenu, Anchor::TopRight, { -25.f, -25.f }
    );

    m_classicContainer = CCLayer::create();
    m_classicContainer->setContentSize({ 380.f, 84.f });
    m_classicContainer->setPosition({ 22.f, 145.f });

    auto classicBG = NineSlice::create("GJ_square03.png");
    classicBG->setContentSize({ 380.f, 84.f });
    classicBG->setPosition({ 190.f, 42.f });
    m_classicContainer->addChild(classicBG, -1);

    auto cellSize = m_classicContainer->getContentSize();

    m_classicCell = new LevelCell("EventLevelCell", 420, 94);
    m_classicCell->setScale(0.85f);
    m_classicCell->setPosition({ -18.f, -4.f });
    m_classicCell->autorelease();
    m_classicCell->setContentSize(cellSize);
    m_classicCell->setAnchorPoint({ 0.5f, 0.5f });
    m_classicContainer->addChild(m_classicCell, 1);

    m_classicSpinner = LoadingSpinner::create(25.f);
    m_classicSpinner->setPosition({ cellSize.width / 2.f, cellSize.height / 2.f });
    m_classicSpinner->setVisible(true);
    m_classicContainer->addChild(m_classicSpinner, 2);

    m_mainLayer->addChild(m_classicContainer);

    m_platContainer = CCLayer::create();
    m_platContainer->setContentSize({ 380.f, 84.f });
    m_platContainer->setPosition({ 22.f, 55.f });

    auto platBG = NineSlice::create("GJ_square03.png");
    platBG->setContentSize({ 380.f, 84.f });
    platBG->setPosition({ 190.f, 42.f });
    m_platContainer->addChild(platBG, -1);

    m_platCell = new LevelCell("EventLevelCell", 420, 94);
    m_platCell->setScale(0.85f);
    m_platCell->setPosition({ -18.f, -4.f });
    m_platCell->autorelease();
    m_platCell->setContentSize(cellSize);
    m_platCell->setAnchorPoint({ 0.5f, 0.5f });
    m_platContainer->addChild(m_platCell, 1);

    m_platSpinner = LoadingSpinner::create(25.f);
    m_platSpinner->setPosition({ cellSize.width / 2.f, cellSize.height / 2.f });
    m_platSpinner->setVisible(true);
    m_platContainer->addChild(m_platSpinner, 2);

    m_mainLayer->addChild(m_platContainer);

    m_classicTimer = CCLabelBMFont::create("Classic: --:--:--:--", "bigFont.fnt");
    m_classicTimer->setPosition({ cs.width / 2.f, 30.f });
    m_classicTimer->setScale(0.4f);
    m_classicTimer->setColor({ 0, 200, 255 });
    m_mainLayer->addChild(m_classicTimer);

    m_platTimer = CCLabelBMFont::create("Platformer: --:--:--:--", "bigFont.fnt");
    m_platTimer->setPosition({ cs.width / 2.f, 15.f });
    m_platTimer->setScale(0.4f);
    m_platTimer->setColor({ 255, 200, 0 });
    m_mainLayer->addChild(m_platTimer);

    auto winSize = CCDirector::get()->getWinSize();

    m_mainLayer->setPositionX(winSize.width * -0.5f);

    auto sequence = CCSequence::create(
        CCEaseElasticOut::create(CCMoveTo::create(0.5f, {winSize.width * 0.5f, winSize.height / 2}), 0.85),
        nullptr
    );

    m_mainLayer->runAction(sequence);

    this->scheduleUpdate();

    auto self = Ref(this);
    m_listener.spawn(
        web::WebRequest().get("https://api.delivel.tech/get_events"),
        [self](web::WebResponse res) {
            if (!self || !self->m_mainLayer) return;
            if (!res.ok()) return;

            auto jsonRes = res.json();
            if (!jsonRes) return;
            auto json = jsonRes.unwrap();

            std::string classicKey = (self->m_type == GrindEventType::Daily) ? "daily" : "weekly";
            std::string platKey = (self->m_type == GrindEventType::Daily) ? "dailyPlat" : "weeklyPlat";

            if (!json.contains(classicKey)) return;

            auto classicObj   = json[classicKey];
            auto classicIdVal = classicObj["levelId"].as<int>();
            if (!classicIdVal) return;

            int classicId = classicIdVal.unwrap();
            self->m_classicLevelId     = classicId;
            self->m_classicSecondsLeft = (float)classicObj["secondsLeft"].asInt().unwrapOrDefault();

            int platId = -1;
            if (json.contains(platKey)) {
                auto platObj   = json[platKey];
                auto platIdVal = platObj["levelId"].asInt();
                if (platIdVal) {
                    platId = platIdVal.unwrap();
                    self->m_platLevelId     = platId;
                    self->m_platSecondsLeft = (float)platObj["secondsLeft"].asInt().unwrapOrDefault();
                }
            }

            auto glm = GameLevelManager::sharedState();
            if (!glm) return;

            if (platId > 0) {
                auto searchObj = GJSearchObject::create(
                    SearchType::Type19,
                    fmt::format("{},{}", classicId, platId)
                );
                auto key    = std::string(searchObj->getKey());
                auto stored = glm->getStoredOnlineLevels(key.c_str());

                if (stored && stored->count() > 0) {
                    for (unsigned int i = 0; i < stored->count(); i++) {
                        auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
                        if (!lvl) continue;
                        if (lvl->m_levelID == classicId && self->m_classicCell) {
                            self->m_classicCell->loadFromLevel(lvl);
                            fixCell(self->m_classicCell);
                            if (self->m_classicSpinner) self->m_classicSpinner->setVisible(false);
                        }
                        if (lvl->m_levelID == platId && self->m_platCell) {
                            self->m_platCell->loadFromLevel(lvl);
                            fixCell(self->m_platCell);
                            if (self->m_platSpinner) self->m_platSpinner->setVisible(false);
                        }
                    }
                } else {
                    self->m_pendingKey       = key;
                    self->m_pendingClassicId = classicId;
                    self->m_pendingPlatId    = platId;
                    glm->getOnlineLevels(searchObj);
                }
            } else {
                auto searchObj = GJSearchObject::create(
                    SearchType::Search, fmt::format("{}", classicId)
                );
                auto key    = std::string(searchObj->getKey());
                auto stored = glm->getStoredOnlineLevels(key.c_str());

                if (stored && stored->count() > 0) {
                    for (unsigned int i = 0; i < stored->count(); i++) {
                        auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
                        if (!lvl) continue;
                        if (lvl->m_levelID == classicId && self->m_classicCell) {
                            self->m_classicCell->loadFromLevel(lvl);
                            fixCell(self->m_classicCell);
                            if (self->m_classicSpinner) self->m_classicSpinner->setVisible(false);
                        }
                    }
                } else {
                    self->m_pendingKey       = key;
                    self->m_pendingClassicId = classicId;
                    self->m_pendingPlatId    = -1;
                    glm->getOnlineLevels(searchObj);
                }
            }
        }
    );

    return true;
}

void EventLevelPopup::onInfoBtn(CCObject* sender) {
    MDPopup::create(
        "Event Grind Levels Info",
        "# <cp>Event Grind Levels</c>\n\n" \
        "There are <cg>4 types of Event Grind Levels:</c>\n" \
        "- Daily Classic Level\n" \
        "- Daily Platformer Level\n" \
        "- Weekly Classic Level\n" \
        "- Weekly Platformer Level\n\n" \
        "Event Levels are <cy>chosen by Grind Admins</c>.",
        "OK"
    )->show();
}

void EventLevelPopup::update(float dt) {
    if (auto id = getChildByIDRecursive("cvolton.betterinfo/level-id-label")) id->removeFromParent();
    if (m_classicSecondsLeft > 0) m_classicSecondsLeft -= dt;
    if (m_platSecondsLeft > 0) m_platSecondsLeft -= dt;

    std::string classicPrefix = (m_type == GrindEventType::Daily) ? "Next Daily Classic in " : "Next Weekly Classic in ";
    std::string platPrefix = (m_type == GrindEventType::Daily) ? "Next Daily Plat in "    : "Next Weekly Plat in ";

    if (m_classicTimer)
        m_classicTimer->setString((classicPrefix + formatTime(m_classicSecondsLeft)).c_str());
    if (m_platTimer)
        m_platTimer->setString((platPrefix + formatTime(m_platSecondsLeft)).c_str());

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
            if (m_classicSpinner) m_classicSpinner->setVisible(false);
            m_pendingClassicId = -1;
        }

        if (m_pendingPlatId != -1 && lvl->m_levelID == m_pendingPlatId && m_platCell) {
            m_platCell->loadFromLevel(lvl);
            fixCell(m_platCell);
            if (m_platSpinner) m_platSpinner->setVisible(false);
            m_pendingPlatId = -1;
        }
    }

    if (m_pendingClassicId == -1 && m_pendingPlatId == -1)
        m_pendingKey.clear();
}