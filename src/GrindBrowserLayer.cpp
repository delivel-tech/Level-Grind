#include "GrindBrowserLayer.hpp"
#include "Geode/c++stl/string.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/particle_nodes/CCParticleExamples.h"
#include "Geode/cocos/particle_nodes/CCParticleSystemQuad.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/cocos/sprite_nodes/CCSpriteFrameCache.h"
#include "Geode/cocos/textures/CCTexture2D.h"
#include "Geode/cocos/textures/CCTextureCache.h"
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

gd::string browserTitle;

GrindBrowserLayer* GrindBrowserLayer::create(const char *title, GJSearchObject *search) {
    auto ret = new GrindBrowserLayer;
    if (ret && ret->init(search)) {
        ret->autorelease();
        browserTitle = title;
        return ret;
    }
    delete ret;
    browserTitle = title;
    return nullptr;
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

            if (!Mod::get()->getSettingValue<bool>("disable-star-particles")) {
                auto grindParticles = CCParticleSnow::create();
                auto texture = CCTextureCache::sharedTextureCache()->addImage("GJ_bigStar_noShadow.png"_spr, true);
                grindParticles->m_fStartSpin = 0.f;
                grindParticles->m_fEndSpin = 180.f;
                grindParticles->m_fStartSize = 6.f;
                grindParticles->m_fEndSize = 3.f;
                grindParticles->setTexture(texture);

            this->addChild(grindParticles);
            }
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
            return browserTitle;
        }
        return LevelBrowserLayer::getSearchTitle();
    }
};