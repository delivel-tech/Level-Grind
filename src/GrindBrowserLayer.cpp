#include "GrindBrowserLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

GrindBrowserLayer* GrindBrowserLayer::create(const char *title, GJSearchObject *search) {
    auto ret = new GrindBrowserLayer;
    if (ret && ret->init(search)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
    browserTitle = title;
}

class $modify(GrindBrowserLayerHook, LevelBrowserLayer) {
    struct Fields {
        cocos2d::CCSprite* m_bg1 = nullptr;
        cocos2d::CCSprite* m_bg2 = nullptr;
        float m_bgWidth = 0.f;
        float m_speed = 30.f;
    };
    bool init(GJSearchObject* object) {
        if (!LevelBrowserLayer::init(object)) return false;
        
        if (auto grind = typeinfo_cast<GrindBrowserLayer*>(this)) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            auto background = getChildByIDRecursive("background");
            background->removeFromParent();

            const char* bgFile = "game_bg_01_001.png";

            m_fields->m_bg1 = CCSprite::create(bgFile);
	        m_fields->m_bg2 = CCSprite::create(bgFile);

            m_fields->m_bg1->setID("bg-1");
            m_fields->m_bg2->setID("bg-2");

	        m_fields->m_bg1->setColor({ 34, 60, 110 });
	        m_fields->m_bg2->setColor({34, 60, 110});

            if (!m_fields->m_bg1 || !m_fields->m_bg2) {
		        return false;
	        }

            m_fields->m_bg1->setAnchorPoint({ 0.f, 0.f });
	        m_fields->m_bg2->setAnchorPoint({ 0.f, 0.f });

	        float scaleY = winSize.height / m_fields->m_bg1->getContentSize().height;
	        float scaleX = winSize.width / m_fields->m_bg1->getContentSize().width;
	        float scale = std::max(scaleY, scaleX);
	        m_fields->m_bg1->setScale(scale);
	        m_fields->m_bg2->setScale(scale);

            m_fields->m_bgWidth = m_fields->m_bg1->boundingBox().size.width;

	        m_fields->m_bg1->setPosition({ 0.f, 0.f });
	        m_fields->m_bg2->setPosition({ m_fields->m_bgWidth - 1.f, 0.f });

	        this->addChild(m_fields->m_bg1, -100);
	        this->addChild(m_fields->m_bg2, -100);
            this->schedule(schedule_selector(GrindBrowserLayerHook::updateBg));
        }

        return true;
    }

    void updateBg(float dt) {
        if (auto grind = typeinfo_cast<GrindBrowserLayer*>(this)) {
            float dx = m_fields->m_speed * dt;
            m_fields->m_bg1->setPositionX(m_fields->m_bg1->getPositionX() - dx);
	        m_fields->m_bg2->setPositionX(m_fields->m_bg2->getPositionX() - dx);
	        if (m_fields->m_bg1->getPositionX() <= -m_fields->m_bgWidth) {
		        m_fields->m_bg1->setPositionX(m_fields->m_bg2->getPositionX() + m_fields->m_bgWidth - 1.f);
	        }
	        if (m_fields->m_bg2->getPositionX() <= -m_fields->m_bgWidth) {
		        m_fields->m_bg2->setPositionX(m_fields->m_bg1->getPositionX() + m_fields->m_bgWidth - 1.f);
	        }
        }
    }

    gd::string getSearchTitle() {
        if (auto grind = typeinfo_cast<GrindBrowserLayer*>(this)) {
            return "Grinding Levels";
        }
        return LevelBrowserLayer::getSearchTitle();
    }
};