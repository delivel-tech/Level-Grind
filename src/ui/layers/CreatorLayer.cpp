#include "GrindCalendarLayer.hpp"
#include "../popups/AchievementsPopup.hpp"
#include "../../managers/BadgesManager.hpp"
#include "CreatorLayer.hpp"
#include "Geode/cocos/actions/CCActionEase.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include <Geode/ui/Label.hpp>
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
#include "GrindLeaderboardsLayer.hpp"
#include "../popups/GrindProfilePopup.hpp"
#include <cue/PlayerIcon.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include "../../managers/Consent.hpp"
#include "alphalaneous.alphas-ui-pack/nodes/scroll/AdvancedScrollLayer.hpp"
#include "alphalaneous.alphas-ui-pack/nodes/scroll/ScrollBarStyles.hpp"
#include "alphalaneous.alphas-ui-pack/nodes/scroll/elements/RoundedScrollHandle.hpp"

namespace levelgrind {

namespace {

CCGLProgram* createCategoryFade(float left, float right, float fadeWidth, float screenWidth) {
    constexpr auto vertex = R"(
        #ifndef GL_ES
        #define mediump
        #define lowp
        #endif
        attribute vec4 a_position;
        attribute vec2 a_texCoord;
        attribute vec4 a_color;
        varying mediump vec2 v_texCoord;
        varying lowp vec4 v_color;
        varying mediump float v_screenX;

        void main() {
            gl_Position = CC_MVPMatrix * a_position;
            v_texCoord = a_texCoord;
            v_color = a_color;
            v_screenX = gl_Position.x / gl_Position.w;
        }
    )";
    constexpr auto fragment = R"(
        #ifdef GL_ES
        precision mediump float;
        #else
        #define mediump
        #define lowp
        #endif
        varying mediump vec2 v_texCoord;
        varying lowp vec4 v_color;
        varying mediump float v_screenX;
        uniform sampler2D CC_Texture0;
        uniform vec2 u_edges;
        uniform float u_fadeWidth;

        void main() {
            float distanceToEdge = min(v_screenX - u_edges.x, u_edges.y - v_screenX);
            float opacity = smoothstep(0.0, u_fadeWidth, distanceToEdge);
            gl_FragColor = texture2D(CC_Texture0, v_texCoord) * v_color * opacity;
        }
    )";

    auto program = new CCGLProgram();
    program->autorelease();
    if (!program->initWithVertexShaderByteArray(vertex, fragment)) return nullptr;
    program->addAttribute("a_position", kCCVertexAttrib_Position);
    program->addAttribute("a_texCoord", kCCVertexAttrib_TexCoords);
    program->addAttribute("a_color", kCCVertexAttrib_Color);
    if (!program->link()) return nullptr;
    program->updateUniforms();
    program->use();
    float toClip = 2.f / screenWidth;
    program->setUniformLocationWith2f(program->getUniformLocationForName("u_edges"),
        left * toClip - 1.f, right * toClip - 1.f);
    program->setUniformLocationWith1f(program->getUniformLocationForName("u_fadeWidth"), fadeWidth * toClip);
    return program;
}

void applyCategoryFade(CCNode* node, CCGLProgram* program) {
    if (typeinfo_cast<CCSprite*>(node) || typeinfo_cast<CCParticleSystemQuad*>(node) || typeinfo_cast<geode::Label*>(node)) {
        node->setShaderProgram(program);
    }
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        applyCategoryFade(child, program);
    }
}

}

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

    auto catMenu = CCMenu::create();
    m_catMenu = catMenu;

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

    CCMenuItemSpriteExtra* petCat = nullptr;
    if (!Mod::get()->getSavedValue<bool>("disable-pet")) {
        petCat = Build(petCatSpr)
            .intoMenuItem([] {
            if (!consent::guard()) return;
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
            if (!consent::guard()) return;
            ClansLayer::create()->open();
        })
        .parent(catMenu)
        .id("clans-cat")
        .collect();

    auto helperSuggestionsCat = Build(CCSprite::createWithSpriteFrameName("helper_suggestions_cat.png"_spr))
        .intoMenuItem([] {
            SuggestionsLayer::create()->open();
        })
        .parent(catMenu)
        .id("helper-suggestions-cat")
        .collect();

    auto calendarCat = Build(CCSprite::createWithSpriteFrameName("stat_tracker_cat.png"_spr))
        .intoMenuItem([] {
            if (!consent::guard()) return;
            GrindCalendarLayer::create()->open();
        })
        .parent(catMenu)
        .id("calendar-cat")
        .collect();

    auto leaderboardsCat = Build(CCSprite::createWithSpriteFrameName("leaderboard_cat.png"_spr))
        .intoMenuItem([] {
            if (!consent::guard()) return;
            GrindLeaderboardsLayer::create()->open();
        })
        .parent(catMenu)
        .id("leaderboards-cat")
        .collect();

    for (const auto& obj : CCArrayExt(catMenu->m_pChildren)) {
        CCMenuItemSpriteExtra* currentCat = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
        if (!currentCat) continue;
        currentCat->m_scaleMultiplier = 1.1f;
        m_cats.push_back(currentCat);
    }
    if (!this->initCategoryScroll(
        { searchCat, packsCat, weeklyAchCat, petCat, calendarCat, leaderboardsCat },
        { dailyCat, weeklyCat, monthlyCat, clansCat, helperSuggestionsCat }
    )) return false;

    achievements::check();

    Ref<CreatorLayer> badgeSelf = this;
    badges::refresh([badgeSelf] {
        if (badgeSelf) badgeSelf->updateBadges();
    });
    m_badgesAsked = true;
    this->schedule(schedule_selector(CreatorLayer::tickBadges), 0.5f);

    this->applyConsent();
    return true;
}

void CreatorLayer::onEnterTransitionDidFinish() {
    BaseLayer::onEnterTransitionDidFinish();
    this->applyConsent();
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

static void enableCascade(CCNode* node) {
    if (auto rgba = typeinfo_cast<CCNodeRGBA*>(node)) rgba->setCascadeColorEnabled(true);
    else if (auto layer = typeinfo_cast<CCLayerRGBA*>(node)) layer->setCascadeColorEnabled(true);
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) enableCascade(child);
}

static void tintTree(CCNode* node, ccColor3B color, bool parentCascades) {
    auto rgba = typeinfo_cast<CCNodeRGBA*>(node);
    if (rgba && !parentCascades) rgba->updateDisplayedColor(color);
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) tintTree(child, color, rgba != nullptr);
}

void CreatorLayer::applyConsent() {
    ccColor3B const color = consent::granted() ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 180, 180, 180 };
    auto dim = [color](CCNode* node) {
        auto item = typeinfo_cast<CCMenuItemSpriteExtra*>(node);
        if (!item || !item->getNormalImage()) return;
        auto image = item->getNormalImage();
        enableCascade(image);
        if (auto rgba = typeinfo_cast<CCNodeRGBA*>(image)) {
            rgba->setColor(color);
            for (auto child : CCArrayExt<CCNode*>(image->getChildren())) tintTree(child, color, true);
        } else {
            tintTree(image, color, false);
        }
    };
    for (auto const& cat : m_cats) {
        auto id = cat->getID();
        if (id == "pet-cat" || id == "clans-cat" || id == "calendar-cat" || id == "leaderboards-cat") dim(cat);
    }
    if (auto menu = this->getChildByID("left-side-menu")) {
        dim(menu->getChildByID("achievements-btn"));
        dim(menu->getChildByID("grind-profile-btn"));
    }
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
            auto image = item->getNormalImage();
            badges::setMark(image, badges::isNew(b.kind));
            if (m_categoryFade) {
                if (auto mark = image->getChildByID("new-mark"_spr)) mark->setShaderProgram(m_categoryFade);
            }
        }
    }
}

bool CreatorLayer::initCategoryScroll(std::initializer_list<CCMenuItemSpriteExtra*> topRow,
    std::initializer_list<CCMenuItemSpriteExtra*> bottomRow) {
    using namespace alpha::ui;

    constexpr float gap = 5.f;
    constexpr float inset = 10.f;
    constexpr float fadeWidth = 20.f;
    constexpr float sideClearance = 55.f;
    float buttonWidth = 0.f;
    float buttonHeight = 0.f;
    for (auto const& cat : m_cats) {
        buttonWidth = std::max(buttonWidth, cat->getContentWidth());
        buttonHeight = std::max(buttonHeight, cat->getContentHeight());
    }

    float availableWidth = right() - 2.f * sideClearance;
    float scale = 0.9f * std::min({ 0.83f, availableWidth / (4.4f * (buttonWidth + gap)),
        180.f / (2.f * buttonHeight + gap) });
    float columnStep = buttonWidth * scale + gap;
    float rowStep = buttonHeight * scale + gap;
    auto countButtons = [](auto const& row) {
        return std::count_if(row.begin(), row.end(), [](auto* button) { return button != nullptr; });
    };
    auto columns = std::max(countButtons(topRow), countButtons(bottomRow));
    CCSize contentSize = { columns * columnStep - gap + 2.f * fadeWidth,
        2.f * rowStep - gap + 2.f * inset };
    CCSize viewport = { std::min({ availableWidth, 4.35f * columnStep + fadeWidth, contentSize.width }),
        contentSize.height };

    auto scroll = AdvancedScrollLayer::create(viewport);
    if (!scroll) return false;
    scroll->setID("category-scroll");
    scroll->setPosition({ centerX(), centerY() - 10.f });
    scroll->setHorizontalScroll(true);
    scroll->setVerticalScroll(false);
    scroll->setVerticalScrollForHorizontal(true);
    scroll->setOvershoot(0.f);
    addChild(scroll);

    m_catMenu->setID("category-menu");
    m_catMenu->setAnchorPoint({ 0.f, 0.f });
    m_catMenu->ignoreAnchorPointForPosition(false);
    m_catMenu->setPosition({ 0.f, 0.f });
    m_catMenu->setContentSize(contentSize);

    float viewportLeft = scroll->getPositionX() - viewport.width / 2.f;
    m_categoryFade = createCategoryFade(viewportLeft, viewportLeft + viewport.width, fadeWidth, right());

    auto placeRow = [&](auto const& row, float y) {
        float x = fadeWidth + buttonWidth * scale / 2.f;
        for (auto* cat : row) {
            if (!cat) continue;
            cat->setScale(scale);
            cat->m_baseScale = scale;
            cat->setPosition({ x, y });
            if (m_categoryFade) applyCategoryFade(cat, m_categoryFade);
            x += columnStep;
        }
    };
    float bottomY = inset + buttonHeight * scale / 2.f;
    placeRow(topRow, bottomY + rowStep);
    placeRow(bottomRow, bottomY);
    scroll->getContentLayer()->addChild(m_catMenu);
    scroll->setInnerContentSize(contentSize);
    scroll->setScrollX(0.f);

    if (contentSize.width > viewport.width) {
        auto bar = AdvancedScrollBar::create(scroll, ScrollOrientation::Horizontal);
        if (!bar) return false;
        constexpr float barThickness = 8.f;
        auto style = RoundedScrollStyle();
        style.m_scrollBarWidth = barThickness;
        style.m_showArrowButtons = true;
        bar->setStyle(style);
        bar->setPadding(2.f, 2.f, 2.f, 2.f);
        bar->setMargins(0.f, 0.f, 0.f, 0.f);
        bar->setMinHandleHeight(20.f);
        auto handle = RoundedScrollHandle::create();
        handle->setBackgroundColor({ 125, 125, 125, 255 });
        handle->setHoverColor({ 140, 140, 140, 255 });
        handle->setClickColor({ 150, 150, 150, 255 });
        bar->setHandle(handle);
        bar->setID("category-scrollbar");
        bar->setContentSize({ barThickness, (viewport.width - 2.f * fadeWidth) - 90 });
        bar->setPosition({ centerX(), scroll->getPositionY() - viewport.height / 2.f - 9.f });
        addChild(bar);
    }

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

    auto gm = GameManager::get();
    auto ownIcon = cue::PlayerIcon::create(cue::Icons{
        IconType::Cube, gm->getPlayerFrame(), gm->getPlayerColor(), gm->getPlayerColor2(),
        gm->getPlayerGlow() ? gm->getPlayerGlowColor() : -1
    });
    Build(CircleButtonSprite::create(ownIcon, CircleBaseColor::Blue))
        .with([](CircleButtonSprite* spr) {
            spr->setTopRelativeScale(0.8f);
        })
        .scale(1.2f)
        .intoMenuItem([] {
            if (!consent::guard()) return;
            int id = GJAccountManager::get()->m_accountID;
            if (id <= 0) {
                Notification::create("Log in to your GD account to see your Grind Profile", NotificationIcon::Info)->show();
                return;
            }
            GrindProfilePopup::create(id, GJAccountManager::get()->m_username)->show();
        })
        .scaleMult(1.1f)
        .id("grind-profile-btn")
        .parent(leftSideMenu);

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
            if (!consent::guard()) return;
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
        auto k = geode::Label::create(key, "bigFont.fnt");
        k->setScale(0.32f);
        row->addChild(k);
        auto v = geode::Label::create("Checking...", "bigFont.fnt");
        v->setScale(0.32f);
        v->setColor({ 200, 200, 200 });
        row->addChild(v);
        row->updateLayout();
        statusBox->addChild(row);
        return std::make_pair(Ref<CCNode>(row), Ref<geode::Label>(v));
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

    WeakRef<CreatorLayer> weakSelf = this;
    consent::setOnChange([weakSelf] {
        if (auto self = weakSelf.lock()) self->applyConsent();
    });
    this->applyConsent();

    if (!consent::decided()) return true;

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
