#include "LGDiscordPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/misc_nodes/CCClippingNode.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LazySprite.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

LGDiscordPopup* LGDiscordPopup::create() {
    auto ret = new LGDiscordPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGDiscordPopup::init() {
    if (!Popup::init(400.f, 225.f, "GJ_square07.png")) return false;

    auto imageSpr = CCSprite::create("discord_popup.png"_spr);
    imageSpr->setScale(0.825f);

    auto sStencil = NineSlice::create("GJ_square06.png");
    if (sStencil) {
        sStencil->setAnchorPoint({ 0.f, 0.f });
        sStencil->setPosition({ 0.f, 0.f });
        sStencil->setContentSize({ m_mainLayer->getScaledContentSize() });
    }

    auto clip = CCClippingNode::create(sStencil);
    clip->setPosition({ 0.f, 0.f });
    clip->setContentSize({ m_mainLayer->getScaledContentSize() });
    clip->setAlphaThreshold(0.01f);

    imageSpr->setPosition({
        m_mainLayer->getScaledContentWidth() / 2,
        m_mainLayer->getScaledContentHeight() / 2
    });
    clip->addChild(imageSpr);

    auto cancelBtnSpr = ButtonSprite::create("Cancel");
    auto joinBtnSpr = ButtonSprite::create("Join");

    auto cancelBtn = CCMenuItemSpriteExtra::create(
        cancelBtnSpr,
        this,
        menu_selector(LGDiscordPopup::closePopup)
    );

    auto joinBtn = CCMenuItemSpriteExtra::create(
        joinBtnSpr,
        this,
        menu_selector(LGDiscordPopup::onJoin)
    );

    m_mainLayer->addChild(clip, -1);
    
    auto btnMenu = CCMenu::create();
    btnMenu->setLayout(RowLayout::create()->setGap(15.f));

    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Center, { 0.f, -80.f });

    btnMenu->addChild(cancelBtn);
    btnMenu->addChild(joinBtn);

    btnMenu->updateLayout();

    return true;
}

void LGDiscordPopup::closePopup(CCObject* sender) {
    this->onClose(sender);
}

void LGDiscordPopup::onJoin(CCObject* sender) {
    web::openLinkInBrowser("https://discord.gg/tmf5xtCX5y");
    this->onClose(sender);
}