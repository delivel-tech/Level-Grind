#include "LGAnnPopup.hpp"
#include "Geode/cocos/platform/CCImage.h"
#include "Geode/ui/LazySprite.hpp"
#include "Geode/ui/NineSlice.hpp"

using namespace geode::prelude;

LGAnnPopup* LGAnnPopup::create() {
    auto ret = new LGAnnPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGAnnPopup::init() {
    if (!Popup::init(400.f, 225.f, "GJ_square07.png")) return false;

    auto imageSpr = LazySprite::create({ m_mainLayer->getScaledContentSize() }, true);
    imageSpr->loadFromUrl("https://api.delivel.tech/cdn/achievement.png", CCImage::kFmtPng, true);
    imageSpr->setAutoResize(true);

    auto stencil = NineSlice::create("GJ_square06.png");
    if (stencil) {
        stencil->setAnchorPoint({0.f, 0.f});
        stencil->setPosition({0.f, 0.f});
        stencil->setContentSize({m_mainLayer->getScaledContentSize()});
    }

    auto clip = CCClippingNode::create(stencil);
    clip->setPosition({0.f, 0.f});
    clip->setContentSize({m_mainLayer->getScaledContentSize()});
    clip->setAlphaThreshold(0.01f);

    imageSpr->setPosition({m_mainLayer->getScaledContentSize().width / 2.f,
                         m_mainLayer->getScaledContentSize().height / 2.f});
    clip->addChild(imageSpr);

    m_mainLayer->addChild(clip, -1);

    return true;
}