#include "DiscordPopup.hpp"
#include "Geode/cocos/actions/CCActionEase.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/misc_nodes/CCClippingNode.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>
#include <cue/RepeatingBackground.hpp>
#include <cmath>

using namespace geode::prelude;

namespace levelgrind {

DiscordPopup* DiscordPopup::create(std::vector<DiscordPageInfo> pages) {
    auto ret = new DiscordPopup;
    if (ret && ret->init(pages)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

cue::RepeatingBackground* DiscordPopup::createBackground() {
    this->m_background = Build(cue::RepeatingBackground::create("game_bg_01_001.png", 1.0f, cue::RepeatMode::X))
        .color({ 37, 50, 167 })
        .scale(0.75f)
        .collect();

    return m_background;
}

void DiscordPopup::logOpened() {
    Mod::get()->setSavedValue(fmt::format("discord-popup-opened-{}", Mod::get()->getVersion()), true);
}

void DiscordPopup::createClip() {
    m_stencil = Build(NineSlice::create("GJ_square06.png"))
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .contentSize({ m_mainLayer->getScaledContentSize() })
        .collect();

    m_clip = Build(CCClippingNode::create(this->m_stencil))
        .pos({ 0.f, 0.f })
        .contentSize({ m_mainLayer->getScaledContentSize() })
        .with([](CCClippingNode* node) {
            node->setAlphaThreshold(0.01f);
        })
        .parent(m_mainLayer)
        .zOrder(-1)
        .collect();
}

void DiscordPopup::wrapNodeIntoClippingNode(CCNode* node) {
    m_clip->addChild(node);
}

CCMenu* DiscordPopup::createNewMainMenu() {
    this->m_mainMenu = Build(CCMenu::create())
        .contentSize(this->m_mainLayer->getContentSize())
        .pos({0, 0})
        .id("main-menu")
        .collect();

    return m_mainMenu;
}

ccColor3B DiscordPopup::hueToColor(float hue, float saturation, float value) {
    float h = hue / 60.f;
    int i = static_cast<int>(h) % 6;
    float f = h - std::floor(h);

    float p = value * (1.f - saturation);
    float q = value * (1.f - saturation * f);
    float t = value * (1.f - saturation * (1.f - f));

    float r = 0.f, g = 0.f, b = 0.f;

    switch (i) {
        case 0: r = value; g = t; b = p; break;
        case 1: r = q; g = value; b = p; break;
        case 2: r = p; g = value; b = t; break;
        case 3: r = p; g = q; b = value; break;
        case 4: r = t; g = p; b = value; break;
        case 5: r = value; g = p; b = q; break;
    }

    return ccc3(static_cast<GLubyte>(r * 255), static_cast<GLubyte>(g * 255), static_cast<GLubyte>(b * 255));
}

void DiscordPopup::update(float dt) {
    m_hue += dt * 60.f;
    if (m_hue >= 360.f) m_hue -= 360.f;

    if (m_background) {
        m_background->setColor(hueToColor(m_hue, 0.4f, 0.85f));
    }
}

bool DiscordPopup::init(std::vector<DiscordPageInfo> pages) {
    if (!BasePopup::init(400.f, 260.f, "GJ_square07.png")) return false;

    m_buttonMenu->removeAllChildren();

    m_pages = pages;

    createClip();
    createNewMainMenu();

    this->createBackground();

    m_mainMenu->addChild(m_background);

    auto grindLogo = Build(CCSprite::create("lg-logo.png"_spr))
        .pos(
            m_mainMenu->getContentWidth() / 2.f,
            m_mainMenu->getContentHeight() / 1.2f + 17.f
        )
        .parent(m_mainMenu)
        .scale(0.8f)
        .id("grind-logo")
        .collect();

    auto discordLabel = Build(CCLabelBMFont::create("Join our Discord Server!", "bigFont.fnt"))
        .pos(
            m_mainMenu->getContentWidth() / 2.f,
            m_mainMenu->getContentHeight() / 1.2f - 13.f
        )
        .parent(m_mainMenu)
        .scale(0.5f)
        .id("discord-label")
        .collect();

    #ifndef GEODE_IS_IOS

    auto particles = Build(CCParticleGalaxy::create())
        .with([](CCParticleGalaxy* galaxy) {
            galaxy->setLife(0.8f);
        })
        .id("join-btn-particles")
        .scale(0.85f)
        .pos(fromCenter({ 52.f, -107.f }))
        .parent(m_mainMenu)
        .collect();

    #endif

    auto btnMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(15))
        .parent(m_mainMenu)
        .pos(fromCenter({ 0.f, -107.f }))
        .scale(0.7f)
        .collect();

    auto cancelBtn = Build(ButtonSprite::create("Cancel", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this](CCObject* sender) { this->onClose(sender); })
        .parent(btnMenu)
        .collect();

    auto joinBtn = Build(ButtonSprite::create("Join", "bigFont.fnt", "GJ_button_01.png"))
        .runAction(
            CCRepeatForever::create(
                CCSequence::create(
                    CCEaseInOut::create(CCScaleTo::create(1.1f, 1.2f), 2.f),
                    CCEaseInOut::create(CCScaleTo::create(1.1f, 1.f), 2.f), nullptr
                )
            )
        )
        .intoMenuItem([this](CCObject* sender) {
            web::openLinkInBrowser("https://discord.gg/tmf5xtCX5y");
            this->onClose(sender);
        })
        .parent(btnMenu)
        .intoParent()
        .updateLayout();

    m_interactiveMenuBG = Build(NineSlice::create("square02_small.png"))
        .contentSize({
            m_mainMenu->getContentWidth() - 40,
            m_mainMenu->getContentHeight() / 2 + 15
        })
        .opacity(100)
        .id("interactive-menu-bg")
        .parent(m_mainMenu)
        .pos({
            centerX(),
            centerY() - 15
        })
        .collect();

    this->m_interactiveMenu = Build(CCMenu::create())
        .contentSize(m_interactiveMenuBG->getContentSize())
        .pos({20, 45})
        .id("interactive-menu")
        .parent(m_mainMenu)
        .collect();

    auto activeNowMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(3)->setAutoScale(false))
        .scale(0.6f)
        .id("active-now-menu")
        .parent(m_interactiveMenu)
        .pos({
            m_interactiveMenu->getContentWidth() / 2,
            m_interactiveMenu->getContentHeight() - 15
        })
        .collect();

    auto onlineCounterDot = Build(CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png"))
        .color({0, 255, 106})
        .id("online-counter-dot")
        .scale(0.7f)
        .parent(activeNowMenu)
        .collect();

    auto loadingCounter = Build(LoadingSpinner::create(17))
        .id("loading-counter")
        .parent(activeNowMenu)
        .collect();

    auto onlineLabel = Build(CCLabelBMFont::create("online now", "goldFont.fnt"))
        .id("online-label")
        .scale(0.8f)
        .parent(activeNowMenu)
        .collect();

    activeNowMenu->updateLayout();

    auto activeNowMenuRef = Ref(activeNowMenu);

    m_listener.spawn(
        web::WebRequest().get("https://discord.com/api/guilds/1381620621515165756/widget.json"),
        [activeNowMenuRef](web::WebResponse res) {
            if (!activeNowMenuRef) return;
            activeNowMenuRef->removeAllChildrenWithCleanup(true);
            if (!res.ok()) {
                Notification::create("Failed to get online counter!", NotificationIcon::Error)->show();
                return;
            }

            auto jsonRes = res.json();

            if (!jsonRes) {
                Notification::create("Failed to get online counter!", NotificationIcon::Error)->show();
                return;
            }

            auto json = jsonRes.unwrap();

            if (json.contains("presence_count")) {
                int activeNowCount = json["presence_count"].asInt().unwrapOrDefault();

                auto onlineCounterDot = Build(CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png"))
                    .color({0, 255, 106})
                    .id("online-counter-dot")
                    .scale(0.7f)
                    .parent(activeNowMenuRef)
                    .collect();

                auto activeNowLabel = Build(CCLabelBMFont::create(
                    fmt::format("{} active now", activeNowCount).c_str(),
                    "goldFont.fnt"
                ))
                    .id("online-label")
                    .scale(0.8f)
                    .parent(activeNowMenuRef)
                    .collect();

                activeNowMenuRef->updateLayout();
            } else {
                Notification::create("Failed to get online counter!", NotificationIcon::Error)->show();
                return;
            }
        }
    );

    createPagesUI();

    wrapNodeIntoClippingNode(m_mainMenu);

    this->logOpened();

    this->scheduleUpdate();

    return true;
}

void DiscordPopup::createPagesUI() {
    if (m_pages.empty()) return;

    auto popupWidth = m_mainLayer->getContentWidth();

    m_pageArrowLeft = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .intoMenuItem([this](CCObject*) { this->turnPage(-1); })
        .pos({
            -20.f,
            m_interactiveMenuBG->getPositionY()
        })
        .id("page-arrow-left")
        .parent(m_buttonMenu)
        .collect();

    m_pageArrowRight = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .flipX(true)
        .intoMenuItem([this](CCObject*) { this->turnPage(1); })
        .pos({
            popupWidth + 20.f,
            m_interactiveMenuBG->getPositionY()
        })
        .id("page-arrow-right")
        .parent(m_buttonMenu)
        .collect();

    this->updatePage();
}

void DiscordPopup::turnPage(int dir) {
    if (!m_pageArrowLeft || !m_pageArrowRight) return;

    m_currentPage += dir;

    this->updatePage();
}

void DiscordPopup::updatePage() {
    if (!m_pageArrowLeft || !m_pageArrowRight) return;
    if (m_pages.empty()) return;

    auto const& currentPage = m_pages.at(m_currentPage);

    this->cleanUpPage();

    auto contentWidth = m_interactiveMenu->getContentWidth();
    auto contentHeight = m_interactiveMenu->getContentHeight();

    m_pageTitleLabel = Build(CCLabelBMFont::create(currentPage.title.c_str(), "bigFont.fnt"))
        .scale(0.7f)
        .pos({ contentWidth / 2.f, contentHeight - 35.f })
        .parent(m_interactiveMenu)
        .id("page-title-label")
        .collect();

    m_pageDescArea = Build(MDTextArea::create(
        currentPage.description,
        { contentWidth - 20.f, contentHeight - 60.f }
    ))
        .pos({ contentWidth / 2.f, (contentHeight - 60.f) / 2.f + 5.f })
        .parent(m_interactiveMenu)
        .id("page-desc-area")
        .collect();

    if (m_currentPage <= 0) {
        m_pageArrowLeft->setEnabled(false);
        m_pageArrowLeft->setColor({100, 100, 100});
    } else {
        m_pageArrowLeft->setEnabled(true);
        m_pageArrowLeft->setColor({255, 255, 255});
    }

    if (m_currentPage >= static_cast<int>(m_pages.size()) - 1) {
        m_pageArrowRight->setEnabled(false);
        m_pageArrowRight->setColor({100, 100, 100});
    } else {
        m_pageArrowRight->setEnabled(true);
        m_pageArrowRight->setColor({255, 255, 255});
    }
}

void DiscordPopup::cleanUpPage() {
    if (m_pageTitleLabel) {
        m_pageTitleLabel->removeFromParent();
        m_pageTitleLabel = nullptr;
    }
    if (m_pageDescArea) {
        m_pageDescArea->removeFromParent();
        m_pageDescArea = nullptr;
    }
}

}