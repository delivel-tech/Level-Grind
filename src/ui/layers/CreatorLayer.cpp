#include "../popups/AchievementsPopup.hpp"
#include "../../managers/BadgesManager.hpp"
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
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <optional>

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
#include "SettingsLayer.hpp"
#include "PetLayer.hpp"
#include "ClansLayer.hpp"
#include "../../managers/PetManager.hpp"

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

    // cat - category
    // petCat lol

    auto catMenu = Build(CCMenu::create())
        .pos(
            centerX(), centerY() - 10
        )
        .scale(0.83f)
        .contentSize({
            400, 240
        })
        .layout(RowLayout::create()->setGap(5)->setGrowCrossAxis(true)->setCrossAxisOverflow(false))
        .parent(this)
        .collect();

    auto searchCat = Build(CCSprite::createWithSpriteFrameName("search_cat.png"_spr))
        .intoMenuItem([] {
            MainLayer::create()->open();
        })
        .parent(catMenu)
        .id("search-cat")
        .collect();

    auto packsCat = Build(CCSprite::createWithSpriteFrameName("packs_cat.png"_spr))
        .intoMenuItem([] {
            badges::markSeen(badges::Kind::Packs);
            GrindPacksLayer::create()->open();
        })
        .parent(catMenu)
        .id("packs-cat")
        .collect();

    auto weeklyAchCat = Build(CCSprite::createWithSpriteFrameName("weekly_ach_cat.png"_spr))
        .intoMenuItem([] {
            badges::markSeen(badges::Kind::WeeklyAch);
            WeeklyAchievementPopup::create()->show();
        })
        .parent(catMenu)
        .id("weekly-ach-cat")
        .collect();

    auto petCatSpr = CCSprite::createWithSpriteFrameName("pet_cat.png"_spr);
    auto topPetSpr = []() {
		if (Mod::get()->getSavedValue<int>("last-pet-lvl") < 1 || Mod::get()->getSavedValue<int>("last-pet-lvl") > 30) {
			auto spr =  PetManager::getInstance().getPetSprByStyle(PetManager::PetStyle::StandardCube);
			return spr;
		} else {
			auto spr = PetManager::getInstance().getPetSprByStyle(PetManager::getInstance().getStyleByLevel(Mod::get()->getSavedValue<int>("last-pet-lvl")));
			return spr;
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

    if (!Mod::get()->getSavedValue<bool>("disable-pet")) {
        auto petCat = Build(petCatSpr)
            .intoMenuItem([] {
                PetLayer::create()->open();
            })
            .parent(catMenu)
            .id("pet-cat")
            .collect();
    }

    auto dailyCat = Build(CCSprite::createWithSpriteFrameName("daily_cat.png"_spr))
        .intoMenuItem([] {
            badges::markSeen(badges::Kind::Daily);
            EventPopup::create(EventType::Daily)->show();
        })
        .parent(catMenu)
        .id("daily-cat")
        .collect();

    auto weeklyCat = Build(CCSprite::createWithSpriteFrameName("weekly_cat.png"_spr))
        .intoMenuItem([] {
            badges::markSeen(badges::Kind::Weekly);
            EventPopup::create(EventType::Weekly)->show();
        })
        .parent(catMenu)
        .id("weekly-cat")
        .collect();

    auto monthlyCat = Build(CCSprite::createWithSpriteFrameName("monthly_cat.png"_spr))
        .intoMenuItem([] {
            badges::markSeen(badges::Kind::Monthly);
            EventPopup::create(EventType::Monthly)->show();
        })
        .parent(catMenu)
        .id("monthly-cat")
        .collect();

    auto clansCat = Build(CCSprite::createWithSpriteFrameName("clans_cat.png"_spr))
        .intoMenuItem([] {
            ClansLayer::create()->open();
        })
        .parent(catMenu)
        .id("clans-cat")
        .collect();

    // page 2
    auto helperSuggestionsCat = Build(CCSprite::createWithSpriteFrameName("helper_suggestions_cat.png"_spr))
        .intoMenuItem([] {
            SuggestionsLayer::create()->open();
        })
        .parent(catMenu)
        .id("helper-suggestions-cat")
        .collect();

    for (const auto& obj : CCArrayExt(catMenu->m_pChildren)) {
        CCMenuItemSpriteExtra* currentCat = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
        if (!currentCat) continue;
        currentCat->m_scaleMultiplier = 1.1f;
        m_cats.push_back(currentCat);
    }
    m_catMenu = catMenu;

    auto arrowMenu = Build<CCMenu>::create()
        .pos({ 0.f, 0.f })
        .contentSize(winSize)
        .zOrder(2)
        .id("cat-page-menu")
        .parent(this)
        .collect();

    auto makeArrow = [&](bool next) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        spr->setFlipX(next);
        spr->setScale(0.8f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, next](CCObject*) {
            this->showCatPage(m_catPage + (next ? 1 : -1));
        });
        float half = catMenu->getScaledContentWidth() / 2.f + 18.f;
        btn->setPosition({ catMenu->getPositionX() + (next ? half : -half), catMenu->getPositionY() });
        btn->setID(next ? "next-cat-page" : "prev-cat-page");
        arrowMenu->addChild(btn);
        return btn;
    };
    m_prevPageBtn = makeArrow(false);
    m_nextPageBtn = makeArrow(true);

    this->showCatPage(0);

    achievements::check();

    Ref<CreatorLayer> badgeSelf = this;
    badges::refresh([badgeSelf] {
        if (badgeSelf) badgeSelf->updateBadges();
    });
    m_badgesAsked = true;
    this->schedule(schedule_selector(CreatorLayer::tickBadges), 0.5f);

    return true;
}

void CreatorLayer::onEnter() {
    BaseLayer::onEnter();
    if (m_badgesAsked) {
        Ref<CreatorLayer> self = this;
        badges::refresh([self] {
            if (self) self->updateBadges();
        });
    }
}

void CreatorLayer::tickBadges(float) {
    this->updateBadges();
}

void CreatorLayer::updateBadges() {
    if (!m_catMenu) return;
    struct { const char* id; badges::Kind kind; } const buttons[] = {
        { "daily-cat", badges::Kind::Daily },
        { "weekly-cat", badges::Kind::Weekly },
        { "monthly-cat", badges::Kind::Monthly },
        { "weekly-ach-cat", badges::Kind::WeeklyAch },
        { "packs-cat", badges::Kind::Packs },
        { "clans-cat", badges::Kind::Clans },
    };
    for (auto const& b : buttons) {
        if (auto item = typeinfo_cast<CCMenuItemSpriteExtra*>(m_catMenu->getChildByID(b.id))) {
            badges::setMark(item->getNormalImage(), badges::isNew(b.kind));
        }
    }
}

int CreatorLayer::catPageCount() const {
    return std::max<int>(1, static_cast<int>((m_cats.size() + kCatsPerPage - 1) / kCatsPerPage));
}

void CreatorLayer::showCatPage(int page) {
    int pages = this->catPageCount();
    page = std::clamp(page, 0, pages - 1);
    m_catPage = page;

    m_catMenu->removeAllChildrenWithCleanup(false);
    size_t first = static_cast<size_t>(page) * kCatsPerPage;
    size_t last = std::min(first + kCatsPerPage, m_cats.size());
    for (size_t i = first; i < last; i++) {
        m_catMenu->addChild(m_cats[i]);
    }
    m_catMenu->updateLayout();

    auto setArrow = [](CCMenuItemSpriteExtra* btn, bool active) {
        if (!btn) return;
        btn->setEnabled(active);
        if (auto spr = typeinfo_cast<CCSprite*>(btn->getNormalImage())) {
            spr->setColor(active ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 90, 90, 90 });
            spr->setOpacity(active ? 255 : 150);
        }
    };
    setArrow(m_prevPageBtn, page > 0);
    setArrow(m_nextPageBtn, page < pages - 1);
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

    Build(CircleButtonSprite::createWithSpriteFrameName("rankIcon_1_001.png", 1.f, CircleBaseColor::Blue))
        .scale(1.2f)
        .intoMenuItem([] {
            AchievementsPopup::create()->show();
        })
        .scaleMult(1.1f)
        .id("achievements-btn")
        .parent(leftSideMenu);

    auto getBadge = [] {
        GrindPosition pos = DataManager::getInstance().getUserPosition();
        if (pos == GrindPosition::Helper) return "badge_helper.png"_spr;
        else if (pos == GrindPosition::Admin) return "badge_admin.png"_spr;
        else if (pos == GrindPosition::Owner) return "badge_owner.png"_spr;
        else return "";
    };

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Helper
    || DataManager::getInstance().getUserPosition() == GrindPosition::Admin
    || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        auto staffBtn = Build(CircleButtonSprite::createWithSpriteFrameName(getBadge(), 0.8f, CircleBaseColor::Blue))
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
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    CCSize const boxSize { 170.f, 64.f };

    auto statusBox = Build(NineSlice::create("square02_small.png"))
        .contentSize(boxSize)
        .anchorPoint({ 0.5f, 0.f })
        .pos({ winSize.width / 2.f, -boxSize.height / 2.f + 2.f })
        .opacity(110)
        .id("status-box")
        .zOrder(3)
        .parent(this)
        .collect();

    auto makeLine = [&](const char* key, float y) {
        auto row = CCNode::create();
        row->setLayout(RowLayout::create()->setGap(3.f)->setAutoScale(false));
        row->setContentSize({ boxSize.width - 10.f, 14.f });
        row->setAnchorPoint({ 0.5f, 0.5f });
        row->setPosition({ boxSize.width / 2.f, y });
        auto k = CCLabelBMFont::create(key, "bigFont.fnt");
        k->setScale(0.32f);
        row->addChild(k);
        auto v = CCLabelBMFont::create("Checking...", "bigFont.fnt");
        v->setScale(0.32f);
        v->setColor({ 200, 200, 200 });
        row->addChild(v);
        row->updateLayout();
        statusBox->addChild(row);
        return std::make_pair(Ref<CCNode>(row), Ref<CCLabelBMFont>(v));
    };

    auto [statusRow, statusValue] = makeLine("Status:", boxSize.height - 12.f);
    auto [versionRow, versionValue] = makeLine("Version:", boxSize.height - 26.f);

    m_listener.spawn(
        APIClient::getInstance().health(),
        [self = Ref(this), statusRow, statusValue, versionRow, versionValue](web::WebResponse res) {
            if (!statusValue || !versionValue) return;
            bool online = APIClient::getInstance().healthParse(res);

            statusValue->setString(online ? "Online" : "Offline");
            statusValue->setColor(online ? ccColor3B{ 100, 255, 100 } : ccColor3B{ 255, 90, 90 });
            statusRow->updateLayout();

            std::string latest;
            if (auto json = res.json()) latest = json.unwrap()["latestModVersion"].asString().unwrapOrDefault();
            auto current = Mod::get()->getVersion();
            std::optional<geode::VersionInfo> latestVer;
            if (!latest.empty()) {
                if (auto parsed = geode::VersionInfo::parse(latest)) latestVer = parsed.unwrap();
            }

            if (!latestVer) {
                versionValue->setString(current.toVString().c_str());
                versionValue->setColor({ 200, 200, 200 });
            } else if (current < *latestVer) {
                versionValue->setString("Outdated");
                versionValue->setColor({ 255, 90, 90 });
                static bool warned = false;
                if (!warned && self) {
                    warned = true;
                    std::string text = fmt::format(
                        "Your <cp>Level Grind</c> version is <cr>outdated</c> ({}).\nPlease <cg>update the mod</c> to {}.",
                        current.toVString(), latestVer->toVString()
                    );
                    self->runAction(CCSequence::create(
                        CCDelayTime::create(0.5f),
                        CallFuncExt::create([text] {
                            FLAlertLayer::create("Update available", text, "OK")->show();
                        }),
                        nullptr
                    ));
                }
            } else {
                versionValue->setString("Up to date");
                versionValue->setColor({ 100, 255, 100 });
            }
            versionRow->updateLayout();
        }
    );

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
}