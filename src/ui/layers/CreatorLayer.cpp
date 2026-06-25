#include "CreatorLayer.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/platform/windows.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <UIBuilder.hpp>

#include "../popups/AnnouncementsPopup.hpp"
#include "../popups/CreditsPopup.hpp"
#include "../popups/DiscordPopup.hpp"
#include "../popups/GuidePopup.hpp"
#include "../popups/EventPopup.hpp"
#include "../popups/WeeklyAchievementPopup.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include "../popups/StaffPopup.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"
#include "GrindPacksLayer.hpp"
#include "MainLayer.hpp"
#include "SuggestionsLayer.hpp"

namespace levelgrind {

CreatorLayer* CreatorLayer::create() {
    auto ret = new CreatorLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CreatorLayer::init() {
    if (!BaseLayer::init()) return false;
    if (!initFarMenus()) return false;
    if (!initMd()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto levelGrindLogo = Build<CCSprite>::create("lg-logo.png"_spr)
        .scale(1.2f)
        .pos({ winSize.width / 2, (winSize.height / 4) * 3.3f + 4.f })
        .id("level-grind-logo")
        .parent(this)
        .collect();

    if (!levelGrindLogo) return false;

    addSideArt(
        this,
        SideArt::Bottom,
        SideArtStyle::LayerGray,
        false
    );

    // cat - category
    // petCat lol

    auto catMenu = Build(CCMenu::create())
        .pos(
            centerX(), centerY() - 10
        )
        .scale(0.8f)
        .contentSize({
            400, 240
        })
        .layout(RowLayout::create()->setGap(10)->setGrowCrossAxis(true)->setCrossAxisOverflow(false))
        .parent(this)
        .collect();

    auto searchCat = Build(CCSprite::create("search_cat.png"_spr))
        .intoMenuItem([] {
            MainLayer::create()->open();
        })
        .parent(catMenu)
        .id("search-cat")
        .collect();

    auto packsCat = Build(CCSprite::create("packs_cat.png"_spr))
        .intoMenuItem([] {
            GrindPacksLayer::create()->open();
        })
        .parent(catMenu)
        .id("packs-cat")
        .collect();

    auto weeklyAchCat = Build(CCSprite::create("weekly_ach_cat.png"_spr))
        .intoMenuItem([] {
            WeeklyAchievementPopup::create()->show();
        })
        .parent(catMenu)
        .id("weekly-ach-cat")
        .collect();

    auto petCat = Build(CCSprite::create("pet_cat.png"_spr))
        .intoMenuItem([] {

        })
        .parent(catMenu)
        .id("pet-cat")
        .collect();

    auto dailyCat = Build(CCSprite::create("daily_cat.png"_spr))
        .intoMenuItem([] {
            EventPopup::create(EventType::Daily)->show();
        })
        .parent(catMenu)
        .id("daily-cat")
        .collect();

    auto weeklyCat = Build(CCSprite::create("weekly_cat.png"_spr))
        .intoMenuItem([] {
            EventPopup::create(EventType::Weekly)->show();
        })
        .parent(catMenu)
        .id("weekly-cat")
        .collect();

    auto monthlyCat = Build(CCSprite::create("monthly_cat.png"_spr))
        .intoMenuItem([] {
            EventPopup::create(EventType::Monthly)->show();
        })
        .parent(catMenu)
        .id("monthly-cat")
        .collect();

    auto helperSuggestionsCat = Build(CCSprite::create("helper_suggestions_cat.png"_spr))
        .intoMenuItem([] {
            SuggestionsLayer::create()->open();
        })
        .parent(catMenu)
        .id("helper-suggestions-cat")
        .collect();

    // making scale mult for all category buttons smaller

    for (const auto& obj : CCArrayExt(catMenu->m_pChildren)) {
        CCMenuItemSpriteExtra* currentCat = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
        currentCat->m_scaleMultiplier = 1.1f;
    }

    catMenu->updateLayout();

    return true;
}

bool CreatorLayer::initFarMenus() {
    auto leftSideMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()
                    ->setGap(5)
                    ->setAxisAlignment(AxisAlignment::Start))
        .parent(this)
        .contentSize(48.f, 250.f)
        .anchorPoint(0.f, 0.f)
        .scale(0.75f)
        .pos(fromBottomLeft({ 15.f, 13.f }))
        .zOrder(1)
        .id("left-side-menu")
        .collect();

    if (!leftSideMenu) return false;

    auto announcementBtn = Build<CCSprite>::create("ann_btn.png"_spr)
        .intoMenuItem([] {
            AnnouncementsPopup::create()->show();
        })
        .scaleMult(1.1f)
        .id("announcement-btn")
        .parent(leftSideMenu)
        .collect();

    auto getBadge = [] {
        GrindPosition pos = DataManager::getInstance().getUserPosition();
        if (pos == GrindPosition::Admin) return "badge_admin.png"_spr;
        else if (pos == GrindPosition::Owner) return "badge_owner.png"_spr;
        else return "";
    };

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin
    || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        auto staffBtn = Build(CircleButtonSprite::createWithSprite(getBadge(), 0.8f, CircleBaseColor::Blue))
            .with([](CircleButtonSprite* spr) {
                spr->getTopNode()->setPosition({
                    spr->getTopNode()->getPositionX() + 1,
                    spr->getTopNode()->getPositionY() - 1
                });
            })
            .scale(1.2f)
            .intoMenuItem([] {
                StaffPopup::create()->show();
            })
            .scaleMult(1.1f)
            .id("staff-btn")
            .parent(leftSideMenu)
            .collect();
    }

    leftSideMenu->updateLayout();

    auto rightSideMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()
                    ->setGap(5)
                    ->setAxisAlignment(AxisAlignment::Start))
        .parent(this)
        .contentSize(48.f, 250.f)
        .scale(0.75f)
        .anchorPoint(1.f, 0.f)
        .pos(fromBottomRight({ 10.f, 13.f }))
        .id("right-side-menu")
        .zOrder(1)
        .collect();

    if (!rightSideMenu) return false;

    auto infoBtn = Build<CCSprite>::create("info_btn.png"_spr)
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::MainPage, GuidePopupState::FromMainLayer)->show();
        })
        .scaleMult(1.1f)
        .id("info-btn")
        .parent(rightSideMenu)
        .collect();

    auto discordBtn = Build<CCSprite>::create("discord_btn.png"_spr)
        .intoMenuItem([] { DiscordPopup::create()->show(); })
        .scaleMult(1.1f)
        .id("discord-btn")
        .parent(rightSideMenu)
        .collect();

    Build<CCSprite>::create("credits_btn.png"_spr)
        .intoMenuItem([] {
            CreditsPopup::create()->show();
        })
        .scaleMult(1.1f)
        .id("credits-btn")
        .parent(rightSideMenu)
        .intoParent()
        .updateLayout();

    return true;
}

bool CreatorLayer::initMd() {
    auto versionLabel = Build(CCLabelBMFont::create(fmt::format("{}", Mod::get()->getVersion()).c_str(), "chatFont.fnt"))
        .opacity(128)
        .anchorPoint({ 1.f, 1.f })
        .pos(fromTopRight({ 5, 5 }))
        .scale(0.6f)
        .id("version-label")
        .parent(this)
        .collect();

    if (!versionLabel) return false;

    auto serverLabel = Build(CCLabelBMFont::create("Server: Loading...", "chatFont.fnt"))
        .opacity(128)
        .anchorPoint({ 1.f, 1.f })
        .pos(fromTopRight({ 5, 18 }))
        .scale(0.6f)
        .id("server-label")
        .parent(this)
        .collect();

    auto serverLabelRef = Ref(serverLabel);

    m_listener.spawn(
        APIClient::getInstance().health(),
        [serverLabelRef](web::WebResponse res) {
            if (!serverLabelRef) return;
            bool ok = APIClient::getInstance().healthParse(res);

            if (ok) {
                serverLabelRef->setString("Server: Online");
                return;
            } else {
                serverLabelRef->setString("Server: Offline");
            }
        }
    );

    return true;
}
}