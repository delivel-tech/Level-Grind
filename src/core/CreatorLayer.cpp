#include "CreatorLayer.hpp"
#include "Geode/cocos/actions/CCActionEase.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/particle_nodes/CCParticleExamples.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <UIBuilder.hpp>

#include "../features/announcements/AnnouncementsPopup.hpp"
#include "../features/credits/CreditsPopup.hpp"
#include "../features/discord/DiscordPopup.hpp"
#include "GuidePopup.hpp"
#include "../features/events/EventPopup.hpp"
#include "../features/weekly-achievements/WeeklyAchievementPopup.hpp"
#include "DataManager.hpp"
#include "BackendManager.hpp"
#include "../features/admin/StaffPopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/cocos.hpp"
#include "../features/levels/GrindPacksLayer.hpp"
#include "MainLayer.hpp"
#include "../features/suggestions/SuggestionsLayer.hpp"
#include "SettingsLayer.hpp"
#include "../features/pets/PetLayer.hpp"
#include "../features/pets/PetManager.hpp"
#include "../features/clans/ClanViewerLayer.hpp"

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
        .scale(1.1f)
        .pos({ winSize.width / 2, (winSize.height / 4) * 3.3f + 14.f })
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

    setupCategoryMenu();

    return true;
}

CCSprite* CreatorLayer::buildPetCategorySprite() {
    auto petCatSpr = CCSprite::create("pet_cat.png"_spr);

    auto topPetSpr = []() -> CCSprite* {
        int lvl = Mod::get()->getSavedValue<int>("last-pet-lvl");
        if (lvl < 1 || lvl > 30) {
            return PetManager::getInstance().getPetSprByStyle(PetManager::PetStyle::StandardCube);
        } else {
            return PetManager::getInstance().getPetSprByStyle(
                PetManager::getInstance().getStyleByLevel(lvl)
            );
        }
    };

    auto top = topPetSpr();
    top->setScale(0.5f);

    auto pulseSeq = CCSequence::create(
        CCEaseInOut::create(CCScaleTo::create(1.1f, 1.4f), 2.f),
        CCEaseInOut::create(CCScaleTo::create(1.1f, 1.25f), 2.f), nullptr
    );
    top->runAction(CCRepeatForever::create(pulseSeq));

    #ifndef GEODE_IS_IOS
    auto particles = CCParticleGalaxy::create();
    particles->setLife(0.8f);
    petCatSpr->addChild(particles);
    particles->setPosition({
        petCatSpr->getContentWidth() / 2,
        petCatSpr->getContentHeight() / 2 + 8
    });
    #endif

    petCatSpr->addChild(top);
    top->setPosition({
        petCatSpr->getContentWidth() / 2,
        petCatSpr->getContentHeight() / 2 + 8
    });

    return petCatSpr;
}

void CreatorLayer::buildCategoryList() {
    m_categories.clear();

    m_categories.push_back({
        [] { return CCSprite::create("search_cat.png"_spr); },
        [] { MainLayer::create()->open(); },
        "search-cat"
    });

    m_categories.push_back({
        [] { return CCSprite::create("packs_cat.png"_spr); },
        [] { GrindPacksLayer::create()->open(); },
        "packs-cat"
    });

    m_categories.push_back({
        .spriteBuilder = [] { return CCSprite::create("search_cat.png"_spr); },
        .callback = [] { ClanViewerLayer::create()->open(); }, 
        .id = "clans-cat"
    });

    m_categories.push_back({
        [] { return CCSprite::create("weekly_ach_cat.png"_spr); },
        [] { WeeklyAchievementPopup::create()->show(); },
        "weekly-ach-cat"
    });

    if (!Mod::get()->getSavedValue<bool>("disable-pet")) {
        m_categories.push_back({
            [this] { return buildPetCategorySprite(); },
            [] { PetLayer::create()->open(); },
            "pet-cat"
        });
    }

    m_categories.push_back({
        [] { return CCSprite::create("daily_cat.png"_spr); },
        [] { EventPopup::create(EventType::Daily)->show(); },
        "daily-cat"
    });

    m_categories.push_back({
        [] { return CCSprite::create("weekly_cat.png"_spr); },
        [] { EventPopup::create(EventType::Weekly)->show(); },
        "weekly-cat"
    });

    m_categories.push_back({
        [] { return CCSprite::create("monthly_cat.png"_spr); },
        [] { EventPopup::create(EventType::Monthly)->show(); },
        "monthly-cat"
    });

    m_categories.push_back({
        [] { return CCSprite::create("helper_suggestions_cat.png"_spr); },
        [] { SuggestionsLayer::create()->open(); },
        "helper-suggestions-cat"
    });

    // new cats here
}

int CreatorLayer::getCategoryPageCount() {
    if (m_categories.empty()) return 1;
    return (int)((m_categories.size() + CATS_PER_PAGE - 1) / CATS_PER_PAGE);
}

void CreatorLayer::setupCategoryMenu() {
    buildCategoryList();

    m_catMenu = Build(CCMenu::create())
        .pos(centerX(), centerY() - 10)
        .scale(0.8f)
        .contentSize({400, 240})
        .layout(RowLayout::create()->setGap(10)->setGrowCrossAxis(true)->setCrossAxisOverflow(false))
        .id("cat-menu")
        .parent(this)
        .collect();

    m_catArrowMenu = Build(CCMenu::create())
        .pos(centerX(), centerY() - 10 - 115.f)
        .id("cat-arrow-menu")
        .parent(this)
        .collect();

    m_catArrowLeft = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .scale(0.6f)
        .intoMenuItem([this] { turnCategoryPage(-1); })
        .id("cat-arrow-left")
        .pos({-35.f, 0.f})
        .parent(m_catArrowMenu)
        .collect();

    m_catArrowRight = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .scale(0.6f)
        .flipX(true)
        .intoMenuItem([this] { turnCategoryPage(1); })
        .id("cat-arrow-right")
        .pos({35.f, 0.f})
        .parent(m_catArrowMenu)
        .collect();

    m_catPageLabel = Build(CCLabelBMFont::create("", "chatFont.fnt"))
        .scale(0.5f)
        .opacity(150)
        .pos({0.f, 0.f})
        .parent(m_catArrowMenu)
        .id("cat-page-label")
        .collect();

    updateCategoryPage();
}

void CreatorLayer::updateCategoryPage() {
    if (!m_catMenu) return;

    int totalPages = getCategoryPageCount();
    if (m_catPage >= totalPages) m_catPage = std::max(0, totalPages - 1);
    if (m_catPage < 0) m_catPage = 0;

    m_catMenu->removeAllChildrenWithCleanup(true);

    int startIdx = m_catPage * CATS_PER_PAGE;
    int endIdx = std::min((int)m_categories.size(), startIdx + CATS_PER_PAGE);

    for (int i = startIdx; i < endIdx; i++) {
        auto& entry = m_categories[i];
        auto spr = entry.spriteBuilder();

        auto item = Build(spr)
            .intoMenuItem(entry.callback)
            .id(entry.id)
            .parent(m_catMenu)
            .collect();

        item->m_scaleMultiplier = 1.1f;
    }

    m_catMenu->updateLayout();

    bool multiplePages = totalPages > 1;

    if (m_catPageLabel) {
        m_catPageLabel->setString(
            fmt::format("Page {} of {}", m_catPage + 1, totalPages).c_str()
        );
        m_catPageLabel->setVisible(multiplePages);
    }

    if (m_catArrowLeft) {
        bool canGoLeft = m_catPage > 0;
        m_catArrowLeft->setEnabled(canGoLeft);
        m_catArrowLeft->setColor(canGoLeft ? ccc3(255, 255, 255) : ccc3(100, 100, 100));
        m_catArrowLeft->setVisible(multiplePages);
    }

    if (m_catArrowRight) {
        bool canGoRight = m_catPage < totalPages - 1;
        m_catArrowRight->setEnabled(canGoRight);
        m_catArrowRight->setColor(canGoRight ? ccc3(255, 255, 255) : ccc3(100, 100, 100));
        m_catArrowRight->setVisible(multiplePages);
    }
}

void CreatorLayer::turnCategoryPage(int dir) {
    int totalPages = getCategoryPageCount();
    int newPage = m_catPage + dir;
    if (newPage < 0 || newPage >= totalPages) return;

    m_catPage = newPage;
    updateCategoryPage();
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

    auto settingsBtn = Build(CircleButtonSprite::createWithSprite("settings_gear.png"_spr, 1.f, CircleBaseColor::Blue))
        .with([](CircleButtonSprite* spr) {
            spr->getTopNode()->setPosition(
                spr->getTopNode()->getPositionX() + 1.f, spr->getTopNode()->getPositionY() - 0.5f
            );
        })
        .scale(1.2f)
        .intoMenuItem([] {
            SettingsLayer::create()->open();
        })
        .scaleMult(1.1f)
        .id("settings-btn")
        .parent(leftSideMenu)
        .collect();

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
        .intoMenuItem([] { DiscordPopup::create(DataManager::getInstance().getDiscordPages())->show(); })
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

    async::spawn(this->onLoadServerHealth(Ref(serverLabel)));

    if (Mod::get()->getSavedValue<bool>(fmt::format("discord-popup-opened-{}", Mod::get()->getVersion()))) return true;

    this->runAction(
        CCSequence::create(
            CCDelayTime::create(1),
            CallFuncExt::create([] {
                DiscordPopup::create(DataManager::getInstance().getDiscordPages())->show();
            }),
            nullptr
        )
    );

    return true;
}

arc::Future<> CreatorLayer::onLoadServerHealth(Ref<CCLabelBMFont> serverLabelRef) {
    auto parsed = co_await BackendManager::getInstance().health();

    co_await async::waitForMainThread([&] {
        if (!serverLabelRef) return;
        serverLabelRef->setString(parsed.ok ? "Server: Online" : "Server: Offline");
    });

    co_return;
}

}