#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

using namespace geode::prelude;

class EventLevelPopup : public Popup {
public:
    enum GrindEventType {
        Daily = 0,
        Weekly = 1,
    };

    static EventLevelPopup* create(GrindEventType type);

private:
    bool init(GrindEventType type);
    void update(float dt) override;

    void onInfoBtn(CCObject* sender);

    GrindEventType m_type;

    CCSprite* m_title = nullptr;

    CCLayer* m_classicContainer = nullptr;
    LevelCell* m_classicCell = nullptr;
    LoadingSpinner* m_classicSpinner = nullptr;

    CCLayer* m_platContainer = nullptr;
    LevelCell* m_platCell = nullptr;
    LoadingSpinner* m_platSpinner = nullptr;

    CCLabelBMFont* m_classicTimer = nullptr;
    CCLabelBMFont* m_platTimer = nullptr;

    int m_classicLevelId = -1;
    int m_platLevelId = -1;
    double m_classicSecondsLeft = 0.f;
    double m_platSecondsLeft = 0.f;

    std::string m_pendingKey;
    int m_pendingClassicId = -1;
    int m_pendingPlatId = -1;

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;

    ~EventLevelPopup() {
        m_listener.cancel();
    }
};