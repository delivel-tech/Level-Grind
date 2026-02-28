#include "../custom/LGLevelInfoLayer.hpp"
#include <cue/RepeatingBackground.hpp>

using namespace geode::prelude;

LGLevelInfoLayer* LGLevelInfoLayer::create(GJGameLevel *level, bool challenge) {
    auto ret = new LGLevelInfoLayer;
    if (ret && ret->init(level, challenge)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGLevelInfoLayer::init(GJGameLevel* level, bool challenge) {
    if (!LevelInfoLayer::init(level, challenge)) return false;

    auto bg = getChildByIDRecursive("background");

    if (!bg) {
        log::error("failed to find background in LevelInfoLayer");
        return false;
    } else {
        auto parent = bg->getParent();
        if (!parent) {
            log::error("failed to find background parent in LevelInfoLayer");
            return false;
        }
        bg->removeFromParent();

        auto newBg = cue::RepeatingBackground::create("game_bg_01_001.png", 1.0f, cue::RepeatMode::X);
        newBg->setSpeed(0.5f);
        newBg->setColor({ 34, 60, 110 });

        parent->addChild(newBg);
        parent->updateLayout();

        if (!Mod::get()->getSettingValue<bool>("disable-star-particles")) {
            auto grindParticles = CCParticleSnow::create();
            auto texture = CCTextureCache::sharedTextureCache()->addImage("GJ_bigStar_noShadow.png"_spr, true);
            grindParticles->m_fStartSpin = 0.f;
            grindParticles->m_fEndSpin = 180.f;
            grindParticles->m_fStartSize = 6.f;
            grindParticles->m_fEndSize = 3.f;
            grindParticles->setTexture(texture);

            parent->addChild(grindParticles, -1);
        }
    }

    return true;
}