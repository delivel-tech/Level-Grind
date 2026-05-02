#include "EventPopup.hpp"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/NineSlice.hpp"

#include <Geode/binding/LevelCell.hpp>
#include <UIBuilder.hpp>

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

bool EventPopup::init(EventType type) {
    if (!BasePopup::init({ 420.f, 280.f })) return false;

    m_type = type;

    auto cs = m_mainLayer->getContentSize();

    m_noElasticity = true;

    m_title = m_type == EventType::Daily
        ? CCSprite::create("daily_level.png"_spr)
        : CCSprite::create("weekly_level.png"_spr);

    m_title->setPosition({ cs.width / 2.f, cs.height - 25.f });
    m_title->setScale(0.85f);
    m_mainLayer->addChild(m_title);

    addSideArt(
        m_mainLayer,
        SideArt::Bottom,
        SideArtStyle::PopupGold,
        false
    );

    addSideArt(
        m_mainLayer,
        SideArt::TopLeft,
        SideArtStyle::PopupGold,
        false
    );

    m_levelContainer = Build(CCLayer::create())
        .contentSize({ 380.f, 116.f })
        .center()
        .collect();

    auto levelBg = Build(NineSlice::create("GJ_square02.png"))
        .contentSize(m_levelContainer->getContentSize())
        .parent(m_levelContainer)
        .collect();

    m_levelCell = Build(LevelCell::create(380.f, 116.f))
        .parent(m_levelContainer)
        .collect();

    return true;
}

}