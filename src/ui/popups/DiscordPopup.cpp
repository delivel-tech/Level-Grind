#include "DiscordPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/misc_nodes/CCClippingNode.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

DiscordPopup* DiscordPopup::create() {
    auto ret = new DiscordPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DiscordPopup::init() {
    if (!BasePopup::init(400.f, 225.f, "GJ_square07.png")) return false;

    auto imageSpr = Build<CCSprite>::create("discord_popup.png"_spr)
        .scale(0.825f)
        .collect();

    auto sStencil = Build(NineSlice::create("GJ_square06.png"))
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .contentSize({ m_mainLayer->getScaledContentSize() })
        .collect();

    auto clip = Build(CCClippingNode::create(sStencil))
        .pos({ 0.f, 0.f })
        .contentSize({ m_mainLayer->getScaledContentSize() })
        .with([](CCClippingNode* node) {
            node->setAlphaThreshold(0.01f);
        })
        .collect();

    imageSpr->setPosition({
        m_mainLayer->getScaledContentWidth() / 2,
        m_mainLayer->getScaledContentHeight() / 2
    });
    clip->addChild(imageSpr);

    auto btnMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(15))
        .parent(m_mainLayer)
        .pos(fromCenter({ 0.f, -80.f }))
        .collect();

    auto cancelBtn = Build(ButtonSprite::create("Cancel", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this](CCObject* sender) { this->onClose(sender); })
        .parent(btnMenu)
        .collect();

    auto joinBtn = Build(ButtonSprite::create("Join", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this](CCObject* sender) {
            web::openLinkInBrowser("https://discord.gg/tmf5xtCX5y");
            this->onClose(sender);
        })
        .parent(btnMenu)
        .intoParent()
        .updateLayout();

    m_mainLayer->addChild(clip, -1);

    return true;
}

}