#include "WeeklyAchievementPopup.hpp"
#include "Geode/cocos/misc_nodes/CCClippingNode.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/LazySprite.hpp"
#include "Geode/ui/NineSlice.hpp"

#include <UIBuilder.hpp>
#include "GuidePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

WeeklyAchievementPopup* WeeklyAchievementPopup::create() {
    auto ret = new WeeklyAchievementPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchievementPopup::init() {
    if (!Popup::init(400.f, 225.f, "GJ_square07.png")) return false;

    auto imageSpr = Build(LazySprite::create({ m_mainLayer->getScaledContentSize() }, true))
        .with([](LazySprite* spr) {
            spr->loadFromUrl("https://api.delivel.tech/cdn/achievement.png", CCImage::kFmtPng, true);
            spr->setAutoResize(true);
        })
        .collect();

    auto stencil = Build(NineSlice::create("GJ_square06.png"))
        .anchorPoint({ 0, 0 })
        .pos({ 0, 0 })
        .contentSize({m_mainLayer->getScaledContentSize()})
        .collect();

    auto clip = Build(CCClippingNode::create(stencil))
        .pos({ 0, 0 })
        .contentSize({m_mainLayer->getScaledContentSize()})
        .with([](CCClippingNode* clippingNode) {
            clippingNode->setAlphaThreshold(0.01f);
        })
        .collect();

    imageSpr->setPosition({
        m_mainLayer->getScaledContentSize().width / 2.f,
        m_mainLayer->getScaledContentSize().height / 2.f
    });
    clip->addChild(imageSpr);

    m_mainLayer->addChild(clip, -1);

    auto infoBtnMenu = Build(CCMenu::create())
        .pos(
            m_mainLayer->getContentWidth(),
            0
        )
        .parent(m_mainLayer)
        .id("info-btn-menu")
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::WeeklyAchievementsGuide, GuidePopupState::FromOutside)->show();
        })
        .parent(infoBtnMenu)
        .id("info-btn")
        .collect();

    return true;
}

}