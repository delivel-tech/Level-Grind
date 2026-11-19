#include "ClansLayer.hpp"
#include "../ClanUI.hpp"
#include "../popups/ClanPopups.hpp"
#include "../popups/GuidePopup.hpp"
#include "../../managers/BadgesManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <UIBuilder.hpp>
#include <ctime>
#include <fmt/format.h>
#include "../popups/GrindProfilePopup.hpp"
#include "../popups/ClanWarPopups.hpp"
#include "../../managers/PetManager.hpp"

namespace levelgrind {

using namespace clanui;
using M = GrindClansManager;

namespace {

constexpr CCSize kPanelSize { 460.f, 280.f };
constexpr CCPoint kLeftOrigin { 10.f, 10.f };
constexpr CCSize kLeftSize { 284.f, 236.f };
constexpr CCPoint kTasksOrigin { 300.f, 96.f };
constexpr CCSize kTasksSize { 150.f, 150.f };
constexpr CCPoint kActionsOrigin { 300.f, 10.f };
constexpr CCSize kActionsSize { 150.f, 80.f };

constexpr float kChatPoll = 4.f;

CCLabelBMFont* lbl(CCNode* parent, std::string const& text, CCPoint pos, float scale, const char* font = "goldFont.fnt", CCPoint anchor = { 0.f, 0.5f }) {
    auto l = CCLabelBMFont::create(text.c_str(), font);
    l->setScale(scale);
    l->setAnchorPoint(anchor);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

CCMenu* overlayMenu(CCNode* parent, int z = 5) {
    auto menu = CCMenu::create();
    menu->setContentSize(parent->getContentSize());
    menu->ignoreAnchorPointForPosition(false);
    menu->setAnchorPoint({ 0.f, 0.f });
    menu->setPosition({ 0.f, 0.f });
    parent->addChild(menu, z);
    return menu;
}

ButtonSprite* segment(const char* text, bool on, int width = 64) {
    auto spr = ButtonSprite::create(text, width, true, "goldFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 24.f, 0.6f);
    return spr;
}

CCMenuItemSpriteExtra* actionButton(CCMenu* menu, CCNode* top, const char* caption, geode::Function<void()> cb, int badge = 0, float cellWidth = 46.f) {
    float const cw = cellWidth;
    float const icon = std::min(24.f, cw - 10.f);
    auto holder = CCNode::create();
    holder->setContentSize({ cw, 36.f });
    float s = icon / std::max(top->getContentWidth(), top->getContentHeight());
    top->setScale(s);
    top->setPosition({ cw / 2.f, 23.f });
    holder->addChild(top);

    auto cap = CCLabelBMFont::create(caption, "bigFont.fnt");
    cap->setPosition({ cw / 2.f, 5.f });
    cap->limitLabelWidth(cw - 2.f, 0.24f, 0.1f);
    holder->addChild(cap);

    if (badge > 0) {
        CCPoint const bp { cw / 2.f + icon / 2.f - 2.f, 23.f + icon / 2.f - 2.f };
        auto bubble = NineSlice::create("square02_small.png");
        bubble->setContentSize({ 14.f, 12.f });
        bubble->setColor({ 230, 50, 50 });
        bubble->setPosition(bp);
        holder->addChild(bubble, 2);
        auto n = CCLabelBMFont::create(fmt::format("{}", std::min(badge, 99)).c_str(), "bigFont.fnt");
        n->setScale(0.26f);
        n->setPosition(bp);
        holder->addChild(n, 3);
    }

    auto btn = CCMenuItemExt::createSpriteExtra(holder, [cb = std::move(cb)](CCObject*) mutable { cb(); });
    menu->addChild(btn);
    return btn;
}

CCNode* centeredText(std::vector<std::pair<std::string, ccColor3B>> const& lines, float scale = 0.6f) {
    auto col = CCNode::create();
    col->setLayout(ColumnLayout::create()->setGap(4.f)->setAxisReverse(true)->setAutoScale(false));
    col->setContentSize({ 260.f, 80.f });
    col->setAnchorPoint({ 0.5f, 0.5f });
    for (auto const& [text, color] : lines) {
        auto l = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        l->setScale(scale * 0.65f);
        l->setColor(color);
        l->limitLabelWidth(250.f, scale * 0.65f, 0.12f);
        col->addChild(l);
    }
    col->updateLayout();
    return col;
}

CCNode* accountTop(const char* frame, float frameScale, AccountBaseColor color) {
    return AccountButtonSprite::createWithSpriteFrameName(frame, frameScale, color);
}

CCNode* frameTop(const char* frame) {
    return CCSprite::createWithSpriteFrameName(frame);
}

ccColor3B rankColor(int rank) {
    switch (rank) {
        case 1: return { 255, 210, 60 };
        case 2: return { 210, 220, 235 };
        case 3: return { 230, 150, 90 };
    }
    return { 255, 255, 255 };
}

}

ClansLayer* ClansLayer::create() {
    auto ret = new ClansLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClansLayer::init() {
    if (!BaseLayer::init()) return false;

    addSideArt(this, SideArt::Bottom, SideArtStyle::LayerGray, false);
    this->playMusic(true);

    if (m_backBtn) {
        m_backBtn->removeFromParent();
        m_backBtn = nullptr;
    }
    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(ClansLayer::onBack)
    );
    backBtn->setPosition({ 25.f, m_winSize.height - 25.f });
    backBtn->setID("back-btn");
    auto backMenu = CCMenu::create();
    backMenu->setPosition({ 0.f, 0.f });
    backMenu->setID("back-menu");
    this->addChild(backMenu, 3);
    backMenu->addChild(backBtn);
    m_backBtn = backBtn;

    auto infoMenu = CCMenu::create();
    infoMenu->setPosition({ m_winSize.width - 25.f, 25.f });
    infoMenu->setID("info-btn-menu");
    this->addChild(infoMenu, 2);
    auto infoSpr = CCSprite::create("info_btn.png"_spr);
    infoSpr->setScale(0.7f);
    auto infoBtn = CCMenuItemExt::createSpriteExtra(infoSpr, [](CCObject*) {
        GuidePopup::create(GuidePage::ClansGuide, GuidePopupState::FromOutside)->show();
    });
    infoBtn->setID("info-btn");
    infoMenu->addChild(infoBtn);

    m_panel = NineSlice::create("GJ_square02.png");
    m_panel->setContentSize(kPanelSize);
    m_panel->setPosition(center());
    m_panel->setID("main-panel");
    this->addChild(m_panel);

    lbl(m_panel, "Grind Clans", { 16.f, kPanelSize.height - 18.f }, 0.75f)->setID("title");

    m_left = this->makeSection(kLeftOrigin, kLeftSize, nullptr);
    m_left->setID("left-section");
    m_tasks = this->makeSection(kTasksOrigin, kTasksSize, nullptr);
    m_tasks->setID("tasks-section");
    m_actions = this->makeSection(kActionsOrigin, kActionsSize, nullptr);
    m_actions->setID("actions-section");

    M::getInstance().flushClaims();
    M::getInstance().checkAllPacks();

    this->reload();
    return true;
}

void ClansLayer::onExit() {
    this->closeEmotePicker();
    this->unschedule(schedule_selector(ClansLayer::pollChat));
    BaseLayer::onExit();
}

void ClansLayer::playMusic(bool in) {
    if (in) {
        auto fmod = FMODAudioEngine::sharedEngine();
        fmod->pauseMusic(0);
        fmod->playMusic("secretLoop02.mp3", true, 1.f, 0);
    } else {
        GameManager::sharedState()->fadeInMenuMusic();
    }
}

void ClansLayer::onBack(CCObject*) {
    this->playMusic(false);
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

CCNode* ClansLayer::makeSection(CCPoint origin, CCSize size, const char*) {
    auto node = CCNode::create();
    node->setContentSize(size);
    node->setPosition(origin);
    m_panel->addChild(node);

    auto box = darkBox(size, 85);
    box->setAnchorPoint({ 0.f, 0.f });
    box->setID("section-bg");
    node->addChild(box, -2);
    return node;
}

void ClansLayer::clearSection(CCNode* section) {
    std::vector<CCNode*> remove;
    for (auto child : CCArrayExt<CCNode*>(section->getChildren())) {
        if (child->getID() != "section-bg") remove.push_back(child);
    }
    for (auto child : remove) child->removeFromParent();
}

void ClansLayer::showLoading() {
    this->unschedule(schedule_selector(ClansLayer::pollChat));
    m_list = nullptr;
    m_chat = nullptr;
    for (auto section : { m_left, m_tasks, m_actions }) {
        this->clearSection(section);
        auto spinner = LoadingSpinner::create(section == m_left ? 40.f : 22.f);
        spinner->setPosition(section->getContentSize() / 2.f);
        section->addChild(spinner);
    }
}

void ClansLayer::showError(std::string const& error) {
    for (auto section : { m_left, m_tasks, m_actions }) this->clearSection(section);

    auto msg = centeredText({ { "Couldn't load Grind Clans.", { 255, 110, 110 } }, { error, { 255, 255, 255 } } });
    msg->setPosition(kLeftSize / 2.f + CCSize{ 0.f, 16.f });
    m_left->addChild(msg);

    auto menu = overlayMenu(m_left);
    Ref<ClansLayer> self = this;
    auto retry = CCMenuItemExt::createSpriteExtra(textButton("Retry", "GJ_button_01.png", 0.7f), [self](CCObject*) {
        if (self) self->reload();
    });
    retry->setPosition({ kLeftSize.width / 2.f, kLeftSize.height / 2.f - 30.f });
    menu->addChild(retry);
}

void ClansLayer::updateWallet(GPWallet const& w) {
    if (m_walletNode) m_walletNode->removeFromParent();

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setContentSize({ 0.f, 0.f });
    m_panel->addChild(menu, 5);
    m_walletNode = menu;

    auto pill = CCNode::create();
    pill->setContentSize({ 150.f, 22.f });
    auto box = darkBox({ 150.f, 22.f }, 110);
    box->setAnchorPoint({ 0.f, 0.f });
    pill->addChild(box);
    auto amount = gpAmount(w.balance, 0.55f);
    amount->setAnchorPoint({ 0.f, 0.5f });
    amount->setPosition({ 8.f, 11.f });
    pill->addChild(amount);
    auto today = CCLabelBMFont::create(fmt::format("today {}/{}", w.todayEarned, w.dailyCap).c_str(), "bigFont.fnt");
    today->setScale(0.27f);
    today->setOpacity(190);
    today->setAnchorPoint({ 1.f, 0.5f });
    today->setPosition({ 142.f, 11.f });
    pill->addChild(today);

    auto btn = CCMenuItemExt::createSpriteExtra(pill, [w](CCObject*) { EarnGPPopup::create(w)->show(); });
    btn->m_scaleMultiplier = 1.05f;
    btn->setPosition({ kTasksOrigin.x + kTasksSize.width / 2.f, kPanelSize.height - 19.f });
    btn->setID("gp-wallet");
    menu->addChild(btn);
}

void ClansLayer::reload() {
    this->showLoading();

    Ref<ClansLayer> self = this;
    m_homeTask.spawn(M::post("clan_home", M::authBody()), [self](web::WebResponse res) {
        if (!self) return;
        auto home = M::parseHome(res);
        if (!home.ok) {
            self->showError(home.error);
            return;
        }
        self->m_home = std::move(home);
        self->m_loaded = true;
        if (self->m_home.hasClan) {
            int done = 0;
            for (auto const& t : self->m_home.clan.tasks) done += t.completed ? 1 : 0;
            badges::markClanTasksSeen(self->m_home.weekEndsAt, done);
        }
        self->m_serverNow = self->m_home.now;
        self->updateWallet(self->m_home.wallet);
        self->m_browsingInClan = false;
        if (self->m_home.hasClan) self->buildClanView();
        else self->buildBrowse();
        self->buildTasks();
        self->buildActions();
    });
}

void ClansLayer::buildBrowse() {
    this->clearSection(m_left);
    m_chat = nullptr;
    m_listHint = nullptr;
    Ref<ClansLayer> self = this;
    auto menu = overlayMenu(m_left);
    float const w = kLeftSize.width;
    float const topY = kLeftSize.height - 16.f;

    auto tabs = CCMenu::create();
    tabs->setLayout(RowLayout::create()->setGap(4.f)->setAutoScale(false)->setAxisAlignment(AxisAlignment::Start));
    tabs->setContentSize({ 130.f, 22.f });
    tabs->setAnchorPoint({ 0.f, 0.5f });
    tabs->setPosition({ 8.f, topY });
    m_left->addChild(tabs, 5);
    auto tab = [&](const char* text, std::string sort) {
        auto spr = segment(text, m_sort == sort);
        spr->setScale(0.7f);
        tabs->addChild(CCMenuItemExt::createSpriteExtra(spr, [self, sort](CCObject*) {
            if (!self || self->m_sort == sort) return;
            self->m_sort = sort;
            self->m_page = 0;
            self->buildBrowse();
        }));
    };
    tab("This Week", "week");
    tab("All Time", "all");
    tabs->updateLayout();

    auto searchRow = CCMenu::create();
    searchRow->setLayout(RowLayout::create()->setGap(4.f)->setAutoScale(false)->setAxisAlignment(AxisAlignment::End));
    searchRow->setContentSize({ 130.f, 22.f });
    searchRow->setAnchorPoint({ 1.f, 0.5f });
    searchRow->setPosition({ w - 8.f, topY });
    m_left->addChild(searchRow, 6);

    m_searchInput = TextInput::create(170.f, "Search", "bigFont.fnt");
    m_searchInput->setScale(0.55f);
    m_searchInput->setMaxCharCount(20);
    m_searchInput->setString(m_search);
    searchRow->addChild(m_searchInput);

    auto find = CCSprite::createWithSpriteFrameName("gj_findBtn_001.png");
    find->setScale(0.55f);
    searchRow->addChild(CCMenuItemExt::createSpriteExtra(find, [self](CCObject*) {
        if (!self || !self->m_searchInput) return;
        self->m_search = self->m_searchInput->getString();
        self->m_page = 0;
        self->loadBrowse();
    }));
    searchRow->updateLayout();

    m_list = cue::ListNode::create({ w - 10.f, 176.f }, { 0, 0, 0, 0 }, cue::ListBorderStyle::None);
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(34.f);
    m_list->setCellColors(ccColor4B{ 0, 0, 0, 0 }, ccColor4B{ 0, 0, 0, 30 });
    m_list->setAnchorPoint({ 0.5f, 0.f });
    m_list->setPosition({ w / 2.f, 28.f });
    m_left->addChild(m_list);

    auto prev = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    prev->setScale(0.4f);
    m_prevBtn = CCMenuItemExt::createSpriteExtra(prev, [self](CCObject*) {
        if (self && self->m_page > 0) { self->m_page--; self->loadBrowse(); }
    });
    m_prevBtn->setPosition({ 14.f, 14.f });
    menu->addChild(m_prevBtn);

    auto next = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    next->setScale(0.4f);
    next->setFlipX(true);
    m_nextBtn = CCMenuItemExt::createSpriteExtra(next, [self](CCObject*) {
        if (self) { self->m_page++; self->loadBrowse(); }
    });
    m_nextBtn->setPosition({ w - 14.f, 14.f });
    menu->addChild(m_nextBtn);

    m_pageLabel = lbl(m_left, "", { w / 2.f, 14.f }, 0.28f, "bigFont.fnt", { 0.5f, 0.5f });


    this->loadBrowse();
}

void ClansLayer::loadBrowse() {
    if (!m_list) return;
    m_list->clear();
    m_list->updateLayout(false);
    if (m_listHint) {
        m_listHint->removeFromParent();
        m_listHint = nullptr;
    }
    m_pageLabel->setString("Loading clans...");
    m_prevBtn->setEnabled(false);
    m_nextBtn->setEnabled(false);

    matjson::Value body;
    body["sort"] = m_sort;
    body["search"] = m_search;
    body["page"] = m_page;

    Ref<ClansLayer> self = this;
    m_listTask.spawn(M::post("clan_browse", body), [self](web::WebResponse res) {
        if (!self || !self->m_list) return;
        self->fillBrowse(M::parseBrowse(res));
    });
}

void ClansLayer::fillBrowse(ClanBrowse const& res) {
    auto setArrow = [](CCMenuItemSpriteExtra* btn, bool on) {
        btn->setEnabled(on);
        if (auto spr = typeinfo_cast<CCSprite*>(btn->getNormalImage())) {
            spr->setColor(on ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 90, 90, 90 });
            spr->setOpacity(on ? 255 : 150);
        }
    };

    if (!res.ok) {
        m_pageLabel->setString(res.error.c_str());
        m_pageLabel->limitLabelWidth(kLeftSize.width - 50.f, 0.28f, 0.12f);
        setArrow(m_prevBtn, false);
        setArrow(m_nextBtn, false);
        return;
    }

    float width = kLeftSize.width - 10.f;
    for (auto const& c : res.clans) {
        m_list->addCell(this->makeClanCell(c, width, res.ranked));
    }
    m_list->updateLayout(false);
    m_list->scrollToTop();

    setArrow(m_prevBtn, m_page > 0);
    setArrow(m_nextBtn, res.hasMore);

    if (res.clans.empty()) {
        m_pageLabel->setString("");
        auto hint = m_search.empty()
            ? centeredText({ { "No clans yet.", { 255, 230, 90 } }, { "Be the first one: press Create Clan!", { 255, 255, 255 } } })
            : centeredText({ { "Nothing found.", { 255, 110, 110 } }, { "Try another name.", { 255, 255, 255 } } });
        hint->setPosition({ kLeftSize.width / 2.f, 116.f });
        m_left->addChild(hint, 4);
        m_listHint = hint;
    } else {
        long long left = m_home.weekEndsAt - m_home.now;
        m_pageLabel->setString(m_sort == "week"
            ? fmt::format("Page {}  -  week ends in {}", m_page + 1, countdown(left)).c_str()
            : fmt::format("Page {}  -  all time", m_page + 1).c_str());
        m_pageLabel->limitLabelWidth(kLeftSize.width - 50.f, 0.28f, 0.12f);
    }
}

CCNode* ClansLayer::makeClanCell(ClanInfo const& c, float width, bool ranked) {
    auto row = CCNode::create();
    row->setContentSize({ width, 34.f });
    auto menu = overlayMenu(row, 2);

    if (ranked && c.rank > 0) {
        if (c.rank <= 3) {
            auto medal = CCSprite::createWithSpriteFrameName(c.rank == 1 ? "rankIcon_1_001.png" : c.rank == 2 ? "rankIcon_top10_001.png" : "rankIcon_top50_001.png");
            if (medal) {
                medal->setScale(12.f / medal->getContentHeight());
                medal->setPosition({ 13.f, 17.f });
                row->addChild(medal);
            }
        } else {
            lbl(row, numToString(c.rank), { 13.f, 17.f }, 0.42f, "goldFont.fnt", { 0.5f, 0.5f });
        }
    }

    auto info = CCNode::create();
    info->setContentSize({ 150.f, 32.f });
    auto em = emblem(c.emblemId, c.colorId, 22.f, c.logoId);
    em->setPosition({ 12.f, 16.f });
    info->addChild(em);
    auto name = lbl(info, c.name, { 28.f, 21.f }, 0.38f, "bigFont.fnt");
    name->limitLabelWidth(108.f, 0.38f, 0.15f);
    if (ranked && c.rank > 0 && c.rank <= 3) name->setColor(rankColor(c.rank));
    if (c.golden) name->setColor({ 255, 210, 60 });
    if (c.level >= kRainbowLevel) makeRainbow(name);
    if (c.level >= kPrestigeLevel) addPrestige(row, row->getContentSize(), M::clanColor(c.colorId));
    if (c.featured) {
        auto star = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
        star->setScale(0.3f);
        star->setPosition({ 28.f + name->getScaledContentWidth() + 7.f, 21.f });
        info->addChild(star);
    }
    lbl(info, fmt::format("Lv {}  -  {}/{}  -  {}", c.level, c.members, c.maxMembers, c.joinType == 0 ? "open" : c.joinType == 1 ? "request" : "invite only"),
        { 28.f, 8.f }, 0.24f, "bigFont.fnt")->setOpacity(175);

    Ref<ClansLayer> self = this;
    auto infoBtn = CCMenuItemExt::createSpriteExtra(info, [self, c](CCObject*) {
        if (!self) return;
        ClanViewPopup::create(c.id, [self](ClanInfo const& clan) { if (self) self->onJoin(clan); })->show();
    });
    infoBtn->m_scaleMultiplier = 1.03f;
    infoBtn->setPosition({ 26.f + 75.f, 17.f });
    menu->addChild(infoBtn);

    auto score = gpAmount(m_sort == "week" ? c.weekXp : c.xp, 0.42f);
    score->setPosition({ width - 78.f, 17.f });
    row->addChild(score);

    bool requested = m_home.requested.contains(c.id);
    bool full = c.members >= c.maxMembers;
    bool locked = m_home.wallet.lifetime < c.minGp;
    bool closed = c.joinType == 2;
    const char* text = requested ? "Sent" : full ? "Full" : closed ? "Invite" : locked ? "Locked" : c.joinType == 0 ? "Join" : "Ask";
    bool active = !(requested || full || locked || closed);
    auto joinSpr = ButtonSprite::create(text, 40, true, "goldFont.fnt", active ? "GJ_button_01.png" : "GJ_button_04.png", 24.f, 0.6f);
    joinSpr->setScale(0.6f);
    auto joinBtn = CCMenuItemExt::createSpriteExtra(joinSpr, [self, c, requested, full, locked, closed](CCObject*) {
        if (!self) return;
        if (requested) {
            createQuickPopup("Request sent", fmt::format("Cancel your request to <cy>{}</c>?", c.name), "Keep", "Cancel it",
                [self, c](auto, bool yes) {
                    if (!yes || !self) return;
                    auto body = M::authBody();
                    body["clanId"] = c.id;
                    M::getInstance().runAction("clan_cancel_request", body, "Cancelling...", "Request cancelled.", [self](matjson::Value const&) {
                        if (self) self->reload();
                    });
                });
            return;
        }
        if (full) {
            Notification::create("This clan is full", NotificationIcon::Info)->show();
            return;
        }
        if (closed) {
            Notification::create("This clan is invite only", NotificationIcon::Info)->show();
            return;
        }
        if (locked) {
            Notification::create(fmt::format("You need {} lifetime Grind Points to join", c.minGp), NotificationIcon::Info)->show();
            return;
        }
        self->onJoin(c);
    });
    joinBtn->setPosition({ width - 24.f, 17.f });
    if (!m_home.hasClan) menu->addChild(joinBtn);

    return row;
}

void ClansLayer::onJoin(ClanInfo const& c) {
    auto body = M::authBody();
    body["clanId"] = c.id;
    Ref<ClansLayer> self = this;
    bool open = c.joinType == 0;
    M::getInstance().runAction(
        "clan_join", body,
        open ? "Joining..." : "Sending request...",
        open ? fmt::format("Welcome to {}!", c.name) : "Request sent! An officer will look at it.",
        [self](matjson::Value const&) {
            if (self) self->reload();
        }
    );
}

void ClansLayer::buildClanView() {
    this->closeEmotePicker();
    m_mentionBar = nullptr;
    m_pinnedBar = nullptr;
    m_pinnedId = 0;
    this->clearSection(m_left);
    m_list = nullptr;
    m_lastMessageId = 0;

    auto const& c = m_home.clan.info;
    float const w = kLeftSize.width;
    float const h = kLeftSize.height;

    auto em = emblem(c.emblemId, c.colorId, 26.f, c.logoId);
    em->setPosition({ 24.f, h - 24.f });
    if (m_home.clan.myRole >= 1 && c.level >= 30) {
        auto logoMenu = overlayMenu(m_left, 6);
        Ref<ClansLayer> logoSelf = this;
        auto logoBtn = CCMenuItemExt::createSpriteExtra(em, [logoSelf](CCObject*) {
            if (!logoSelf) return;
            ClanLogoPopup::create(logoSelf->m_home.clan.info, logoSelf->m_home.clan.logoPending, [logoSelf] {
                if (logoSelf) logoSelf->reload();
            })->show();
        });
        logoBtn->setPosition({ 24.f, h - 24.f });
        logoMenu->addChild(logoBtn);
    } else {
        m_left->addChild(em);
    }

    auto name = lbl(m_left, c.name, { 46.f, h - 14.f }, 0.45f, "bigFont.fnt");
    name->limitLabelWidth(150.f, 0.45f, 0.2f);
    name->setColor(M::clanColor(c.colorId));
    auto info = lbl(m_left, fmt::format("Lv {}  -  {}/{} members  -  +{}%", c.level, c.members, c.maxMembers, c.gpBonus),
        { 46.f, h - 28.f }, 0.38f, "chatFont.fnt");
    info->setOpacity(200);
    auto bonusIcon = gpIcon(0.42f, 1.f);
    bonusIcon->setAnchorPoint({ 0.f, 0.5f });
    bonusIcon->setPosition({ 46.f + info->getScaledContentWidth() + 2.f, h - 28.f });
    m_left->addChild(bonusIcon);

    auto bar = ProgressBar::create(ProgressBar::Style::Solid);
    bar->setFillColor(M::clanColor(c.colorId));
    bar->setScale(0.32f);
    bar->setAnchorPoint({ 0.f, 0.5f });
    bar->setPosition({ 46.f, h - 40.f });
    float pct = 100.f;
    if (!c.maxLevel && c.nextLevelXp > c.levelXp) {
        pct = 100.f * static_cast<float>(c.xp - c.levelXp) / static_cast<float>(c.nextLevelXp - c.levelXp);
    }
    bar->updateProgress(std::clamp(pct, 0.f, 100.f));
    m_left->addChild(bar);
    lbl(m_left, c.maxLevel ? "max level" : fmt::format("{}/{}", M::formatGP(c.xp), M::formatGP(c.nextLevelXp)),
        { 46.f + bar->getScaledContentWidth() + 5.f, h - 40.f }, 0.22f, "bigFont.fnt")->setOpacity(190);

    lbl(m_left, fmt::format("#{}", m_home.clan.weekRank), { w - 10.f, h - 15.f }, 0.55f, "bigFont.fnt", { 1.f, 0.5f })
        ->setColor(rankColor(m_home.clan.weekRank));
    auto week = gpAmount(c.weekXp, 0.36f);
    week->setAnchorPoint({ 1.f, 0.5f });
    week->setPosition({ w - 10.f, h - 31.f });
    m_left->addChild(week);
    lbl(m_left, "this week", { w - 10.f, h - 42.f }, 0.21f, "bigFont.fnt", { 1.f, 0.5f })->setOpacity(160);

    auto line = divider(w - 16.f);
    line->setPosition({ 8.f, h - 52.f });
    m_left->addChild(line);

    m_chat = cue::ListNode::create({ w - 8.f, h - 88.f }, { 0, 0, 0, 0 }, cue::ListBorderStyle::None);
    m_chat->setAutoUpdate(false);
    m_chat->setCellColor(ccColor4B{ 0, 0, 0, 0 });
    m_chat->setAnchorPoint({ 0.5f, 0.f });
    m_chat->setPosition({ w / 2.f, 33.f });
    m_left->addChild(m_chat);

    auto empty = centeredText({ { "No messages yet.", { 255, 230, 90 } }, { "Say hi to your clan!", { 255, 255, 255 } } }, 0.55f);
    empty->setPosition({ w / 2.f, 33.f + (h - 88.f) / 2.f });
    m_left->addChild(empty, 3);
    m_chatEmpty = empty;

    auto inputBg = darkBox({ w - 12.f, 26.f }, 110);
    inputBg->setAnchorPoint({ 0.f, 0.f });
    inputBg->setPosition({ 6.f, 4.f });
    m_left->addChild(inputBg);

    m_chatInput = TextInput::create(260.f, "Message your clan...", "chatFont.fnt");
    m_chatInput->setScale(0.72f);
    m_chatInput->setMaxCharCount(150);
    m_chatInput->setCommonFilter(CommonFilter::Any);
    m_chatInput->hideBG();
    m_chatInput->setDelegate(this);
    m_chatInput->setPosition({ 6.f + 260.f * 0.72f / 2.f + 4.f, 17.f });
    m_chatInput->getInputNode()->setParseEmotes(EmoteSprite::maxEmotesMessage());
    m_left->addChild(m_chatInput, 6);

    auto menu = overlayMenu(m_left);
    Ref<ClansLayer> self = this;
    auto sendSpr = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
    sendSpr->setScale(0.42f);
    auto send = CCMenuItemExt::createSpriteExtra(sendSpr, [self](CCObject*) { if (self) self->onSend(); });
    send->setPosition({ w - 20.f, 17.f });
    menu->addChild(send);

    auto emoteSpr = CCSprite::createWithSpriteFrameName("diffIcon_01_btn_001.png");
    emoteSpr->setScale(17.f / std::max(1.f, emoteSpr->getContentHeight()));
    auto emoteBtn = CCMenuItemExt::createSpriteExtra(emoteSpr, [self](CCMenuItemSpriteExtra* btn) {
        if (!self || !self->m_chatInput) return;
        self->m_emotePicker = EmotePickerPopup::toggle(
            self->m_emotePicker, self.data(), btn, self->m_chatInput->getInputNode(), EmoteSprite::maxEmotesMessage(), 150
        );
    });
    emoteBtn->setPosition({ w - 44.f, 17.f });
    menu->addChild(emoteBtn);

    auto lvlSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    lvlSpr->setScale(17.f / std::max(1.f, lvlSpr->getContentHeight()));
    auto lvlBtn = CCMenuItemExt::createSpriteExtra(lvlSpr, [self](CCObject*) {
        if (!self) return;
        ShareLevelPopup::create([self](int id, std::string const& name) {
            if (!self || !self->m_chatInput) return;
            std::string text = self->m_chatInput->getString();
            if (!text.empty() && text.back() != ' ') text += " ";
            self->m_chatInput->setString(text + levelToken(id, name) + " ");
        })->show();
    });
    lvlBtn->setPosition({ w - 68.f, 17.f });
    menu->addChild(lvlBtn);

    m_mentionBar = CCMenu::create();
    m_mentionBar->setLayout(RowLayout::create()->setGap(4.f)->setAutoScale(false)->setAxisAlignment(AxisAlignment::Start));
    m_mentionBar->ignoreAnchorPointForPosition(false);
    m_mentionBar->setAnchorPoint({ 0.f, 0.5f });
    m_mentionBar->setContentSize({ w - 16.f, 18.f });
    m_mentionBar->setPosition({ 8.f, 40.f });
    m_mentionBar->setVisible(false);
    m_left->addChild(m_mentionBar, 8);

    this->showPinned(m_home.clan.hasPinned, m_home.clan.pinned);
    this->fetchChat();
    this->schedule(schedule_selector(ClansLayer::pollChat), kChatPoll);
}

void ClansLayer::showPinned(bool has, ClanChatMessage const& msg) {
    if (m_pinnedBar) {
        m_pinnedBar->removeFromParent();
        m_pinnedBar = nullptr;
    }
    m_pinnedId = has ? msg.id : 0;
    if (!has || !m_left) return;

    float const w = kLeftSize.width;
    float const h = kLeftSize.height;
    auto bar = CCNode::create();
    bar->setContentSize({ w - 16.f, 18.f });
    bar->setAnchorPoint({ 0.5f, 1.f });
    bar->setPosition({ w / 2.f, h - 56.f });
    auto bg = darkBox({ w - 16.f, 18.f }, 170);
    bg->setAnchorPoint({ 0.f, 0.f });
    bar->addChild(bg);
    auto title = CCLabelBMFont::create("Pinned", "goldFont.fnt");
    title->setScale(0.32f);
    title->setAnchorPoint({ 0.f, 0.5f });
    title->setPosition({ 5.f, 9.f });
    bar->addChild(title);
    auto text = CCLabelBMFont::create(fmt::format("{}: {}", msg.username, msg.message).c_str(), "chatFont.fnt");
    text->setScale(0.42f);
    text->setAnchorPoint({ 0.f, 0.5f });
    text->setPosition({ 8.f + title->getScaledContentWidth(), 9.f });
    text->limitLabelWidth(w - 40.f - title->getScaledContentWidth(), 0.42f, 0.2f);
    bar->addChild(text);
    m_left->addChild(bar, 9);
    m_pinnedBar = bar;
}

void ClansLayer::pollChat(float) {
    this->fetchChat();
}

void ClansLayer::fetchChat() {
    if (!m_chat || m_chatBusy) return;
    m_chatBusy = true;

    auto body = M::authBody();
    body["afterId"] = static_cast<int64_t>(m_lastMessageId);
    Ref<ClansLayer> self = this;
    m_chatTask.spawn(M::post("clan_chat_fetch", body), [self](web::WebResponse res) {
        if (!self) return;
        self->m_chatBusy = false;
        if (!self->m_chat) return;

        if (auto json = res.json(); json && res.ok()) {
            auto v = json.unwrap();
            self->m_serverNow = v["now"].asInt().unwrapOr(self->m_serverNow);
            ClanChatMessage pinned;
            bool has = M::parsePinned(v["pinned"], pinned);
            if (has != (self->m_pinnedId > 0) || (has && pinned.id != self->m_pinnedId)) self->showPinned(has, pinned);
        }
        bool ok = false;
        auto msgs = M::parseChat(res, ok);
        if (!ok) {
            if (res.code() == 403) {
                self->reload();
            } else {
                log::warn("clan_chat_fetch failed: {} {}", res.code(), res.string().unwrapOrDefault());
            }
            return;
        }
        if (!msgs.empty()) self->appendChat(msgs);
    });
}

void ClansLayer::appendChat(std::vector<ClanChatMessage> const& messages) {
    float width = kLeftSize.width - 8.f;
    for (auto const& m : messages) {
        if (m.id <= m_lastMessageId) continue;
        m_chat->addCell(this->makeChatCell(m, width));
        m_lastMessageId = m.id;
    }
    while (m_chat->size() > 120) m_chat->removeCell(static_cast<size_t>(0));

    m_chat->updateLayout(false);
    m_chat->scrollToBottom();
    if (m_chatEmpty) m_chatEmpty->setVisible(m_chat->size() == 0);
    badges::markClanChatSeen(m_lastMessageId);
}

CCNode* ClansLayer::makeChatCell(ClanChatMessage const& msg, float width) {
    int role = 0;
    for (auto const& mem : m_home.clan.members) {
        if (mem.accountId == msg.accountId) role = mem.role;
    }
    int myId = GJAccountManager::get()->m_accountID;
    bool mine = msg.accountId == myId;
    bool canDelete = mine || m_home.clan.myRole >= 1;

    float const iconSpace = 26.f;
    float const maxBubble = width - iconSpace - 34.f;
    float const pad = 7.f;
    float const textScale = 0.52f;

    ChatLinks links;
    Ref<ClansLayer> linkSelf = this;
    links.onLevel = std::make_shared<geode::Function<void(int)>>([linkSelf](int id) {
        if (linkSelf) linkSelf->openSharedLevel(id);
    });
    links.onMention = std::make_shared<geode::Function<void(std::string const&)>>([linkSelf](std::string const& name) {
        if (linkSelf) linkSelf->openMention(name);
    });
    CCNode* text = chatText(msg.message, textScale, maxBubble - pad * 2.f, links);
    if (!text) {
        auto plain = CCLabelBMFont::create(msg.message.c_str(), "chatFont.fnt", (maxBubble - pad * 2.f) / textScale, kCCTextAlignmentLeft);
        plain->setScale(textScale);
        text = plain;
    }
    text->setAnchorPoint({ 0.f, 1.f });
    float const dotsW = 16.f;

    std::string when = M::timeAgo(msg.at, m_serverNow > 0 ? m_serverNow : static_cast<long long>(time(nullptr)));
    auto nameLbl = CCLabelBMFont::create(mine ? "You" : msg.username.c_str(), "goldFont.fnt");
    nameLbl->setScale(0.36f);
    nameLbl->setColor(mine ? ccColor3B{ 170, 255, 170 } : M::roleColor(role));
    auto timeLbl = CCLabelBMFont::create(when.c_str(), "chatFont.fnt");
    timeLbl->setScale(0.32f);
    timeLbl->setOpacity(150);

    bool const showTime = role == 0 || mine;
    float headerW = nameLbl->getScaledContentWidth() + 8.f + (showTime ? timeLbl->getScaledContentWidth() : 30.f) + dotsW;
    float bubbleW = std::min(maxBubble, std::max(text->getScaledContentWidth(), headerW) + pad * 2.f);
    float bubbleH = text->getScaledContentHeight() + 18.f;
    float rowH = bubbleH + 6.f;

    auto row = CCNode::create();
    row->setContentSize({ width, rowH });

    float bubbleX = mine ? width - iconSpace - bubbleW : iconSpace;
    auto bubble = NineSlice::create("square02_small.png");
    bubble->setContentSize({ bubbleW, bubbleH });
    bubble->setAnchorPoint({ 0.f, 0.f });
    bubble->setPosition({ bubbleX, 3.f });
    bubble->setColor(mine ? ccColor3B{ 40, 110, 50 } : ccColor3B{ 20, 20, 30 });
    bubble->setOpacity(mine ? 170 : 130);
    row->addChild(bubble);

    auto icon = playerIcon(msg.icon, msg.color1, msg.color2, msg.color3, 0.42f);
    icon->setPosition({ mine ? width - 13.f : 13.f, 3.f + 11.f });
    row->addChild(icon);
    for (auto const& m : m_home.clan.members) {
        if (m.accountId != msg.accountId || !m.hasPet) continue;
        auto pet = PetManager::createMiniPet(m.petLevel, m.petAppearance, m.icon, m.color1, m.color2, m.color3);
        pet->setScale(0.2f);
        pet->setPosition({ mine ? width - 4.f : 22.f, 3.f + 4.f });
        row->addChild(pet, 2);
        break;
    }

    nameLbl->setAnchorPoint({ 0.f, 0.5f });
    nameLbl->setPosition({ bubbleX + pad, 3.f + bubbleH - 8.f });
    row->addChild(nameLbl);
    if (role > 0 && !mine) {
        auto roleLbl = CCLabelBMFont::create(M::roleName(role).c_str(), "chatFont.fnt");
        roleLbl->setScale(0.3f);
        roleLbl->setColor(M::roleColor(role));
        roleLbl->setAnchorPoint({ 0.f, 0.5f });
        roleLbl->setPosition({ bubbleX + pad + nameLbl->getScaledContentWidth() + 4.f, 3.f + bubbleH - 8.f });
        row->addChild(roleLbl);
    }
    timeLbl->setAnchorPoint({ 1.f, 0.5f });
    timeLbl->setPosition({ bubbleX + bubbleW - pad - dotsW, 3.f + bubbleH - 8.f });
    if (showTime) row->addChild(timeLbl);

    text->setPosition({ bubbleX + pad, 3.f + bubbleH - 15.f });
    row->addChild(text);

    {
        auto menu = overlayMenu(row);
        auto hit = CCNode::create();
        hit->setContentSize({ dotsW, 14.f });
        auto dots = CCLabelBMFont::create("...", "bigFont.fnt");
        dots->setScale(0.35f);
        dots->setPosition({ dotsW / 2.f, 9.f });
        dots->setOpacity(200);
        hit->addChild(dots);
        Ref<ClansLayer> self = this;
        auto btn = CCMenuItemExt::createSpriteExtra(hit, [self, msg, canDelete](CCObject*) {
            bool canPin = self && self->m_home.clan.myRole >= 1;
            bool isPinned = self && self->m_pinnedId == msg.id;
            ClanMessagePopup::create(msg, canDelete, canPin, isPinned, [self] {
                if (self && self->m_chat) self->buildClanView();
            })->show();
        });
        btn->setPosition({ bubbleX + bubbleW - pad / 2.f - dotsW / 2.f, 3.f + bubbleH - 8.f });
        menu->addChild(btn);
    }

    return row;
}

void ClansLayer::textInputReturn(CCTextInputNode*) {
    this->onSend();
}

void ClansLayer::keyDown(enumKeyCodes key, double timestamp) {
    if ((key == enumKeyCodes::KEY_Enter || key == enumKeyCodes::KEY_NumEnter) && m_chatInput
        && (m_chatInput->getInputNode()->m_selected || m_emotePicker)) {
        this->onSend();
        return;
    }
    BaseLayer::keyDown(key, timestamp);
}

void ClansLayer::closeEmotePicker() {
    if (!m_emotePicker) return;
    auto picker = m_emotePicker;
    m_emotePicker = nullptr;
    picker->onClose(nullptr);
}

void ClansLayer::emotePickerClosed(EmotePickerPopup* picker) {
    if (picker == m_emotePicker) m_emotePicker = nullptr;
}

void ClansLayer::emotePicked(EmotePickerPopup*, EmoteDef const*) {

}

void ClansLayer::emotePickerKeyDown(EmotePickerPopup*, enumKeyCodes key, double) {
    if (key == enumKeyCodes::KEY_Enter || key == enumKeyCodes::KEY_NumEnter) this->onSend();
}

void ClansLayer::onSend() {
    if (!m_chatInput || m_sending) return;
    std::string msg = m_chatInput->getString();
    if (msg.find_first_not_of(' ') == std::string::npos) return;

    m_sending = true;
    auto body = M::authBody();
    body["message"] = msg;
    m_chatInput->setString("");
    if (m_mentionBar) {
        m_mentionBar->removeAllChildren();
        m_mentionBar->setVisible(false);
    }

    Ref<ClansLayer> self = this;
    m_sendTask.spawn(M::post("clan_chat_send", body), [self, msg](web::WebResponse res) {
        if (!self) return;
        self->m_sending = false;
        std::string err;
        if (!M::parseOk(res, err)) {
            Notification::create(err, NotificationIcon::Error)->show();
            if (self->m_chatInput && self->m_chatInput->getString().empty()) self->m_chatInput->setString(msg);
            return;
        }
        self->m_chatBusy = false;
        self->fetchChat();
    });
}

void ClansLayer::buildTasks() {
    this->clearSection(m_tasks);
    auto const w = kTasksSize.width;
    auto const h = kTasksSize.height;

    if (m_home.hasClan) {
        lbl(m_tasks, "Weekly Tasks", { 8.f, h - 12.f }, 0.45f);
        lbl(m_tasks, fmt::format("new tasks in {}", countdown(m_home.weekEndsAt - m_home.now)), { 8.f, h - 25.f }, 0.22f, "bigFont.fnt")
            ->setOpacity(180);

        int const tiers = M::tierCount(m_home.clan.tasks);
        m_taskTier = std::clamp(m_taskTier, 0, tiers - 1);

        float y = h - 48.f;
        for (auto const& t : m_home.clan.tasks) {
            if (t.tier != m_taskTier) continue;
            CCSprite* icon = M::taskIcon(t.type) ? CCSprite::createWithSpriteFrameName(M::taskIcon(t.type)) : gpIcon(1.f);
            if (icon) {
                icon->setScale(14.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
                icon->setPosition({ 13.f, y + 5.f });
                m_tasks->addChild(icon);
            }
            auto title = lbl(m_tasks, M::taskTitle(t), { 24.f, y + 8.f }, 0.26f, "bigFont.fnt");
            title->limitLabelWidth(w - 62.f, 0.26f, 0.12f);
            if (t.completed) title->setColor({ 140, 255, 140 });

            auto bar = ProgressBar::create(ProgressBar::Style::Solid);
            bar->setFillColor(t.completed ? ccColor3B{ 90, 230, 90 } : ccColor3B{ 255, 200, 60 });
            bar->setScale(0.26f);
            bar->setAnchorPoint({ 0.f, 0.5f });
            bar->setPosition({ 24.f, y - 3.f });
            bar->updateProgress(std::clamp(100.f * t.progress / t.target, 0.f, 100.f));
            m_tasks->addChild(bar);
            lbl(m_tasks, fmt::format("{}/{}", t.progress, t.target), { 13.f, y - 7.f }, 0.28f, "chatFont.fnt", { 0.5f, 0.5f })
                ->limitLabelWidth(22.f, 0.28f, 0.1f);

            if (t.completed) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                check->setScale(0.42f);
                check->setPosition({ w - 16.f, y + 2.f });
                m_tasks->addChild(check);
            } else {
                auto reward = gpAmount(t.rewardGp, 0.34f);
                reward->setPosition({ w - 18.f, y + 2.f });
                m_tasks->addChild(reward);
            }
            y -= 32.f;
        }

        auto tierLbl = lbl(m_tasks, M::tierName(m_taskTier), { w / 2.f, 14.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f });
        tierLbl->limitLabelWidth(72.f, 0.3f, 0.15f);
        tierLbl->setColor(M::tierColor(m_taskTier));
        if (tiers > 1) {
            auto tierMenu = overlayMenu(m_tasks, 6);
            Ref<ClansLayer> self = this;
            for (int dir : { -1, 1 }) {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
                spr->setScale(0.3f);
                spr->setFlipX(dir > 0);
                auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, dir, tiers](CCObject*) {
                    if (!self) return;
                    self->m_taskTier = (self->m_taskTier + dir + tiers) % tiers;
                    Loader::get()->queueInMainThread([self] { if (self) self->buildTasks(); });
                });
                btn->setPosition({ w / 2.f + dir * 46.f, 14.f });
                tierMenu->addChild(btn);
            }
        }
        return;
    }

    lbl(m_tasks, "Earn Grind Points", { 8.f, h - 12.f }, 0.42f);

    auto const& wl = m_home.wallet;
    auto bar = ProgressBar::create(ProgressBar::Style::Solid);
    bar->setFillColor({ 255, 210, 70 });
    bar->setScale(0.3f);
    bar->setAnchorPoint({ 0.f, 0.5f });
    bar->setPosition({ 8.f, h - 28.f });
    float pct = wl.dailyCap > 0 ? 100.f * wl.todayEarned / wl.dailyCap : 0.f;
    bar->updateProgress(std::clamp(pct, 0.f, 100.f));
    m_tasks->addChild(bar);
    lbl(m_tasks, fmt::format("{}/{} today", wl.todayEarned, wl.dailyCap), { 8.f + bar->getScaledContentWidth() + 4.f, h - 28.f }, 0.21f, "bigFont.fnt");

    struct Way { const char* icon; bool spr; const char* text; const char* reward; ccColor3B color; };
    Way const ways[] = {
        { "GJ_starsIcon_001.png", false, "Any rated level", "1 / star", { 255, 255, 255 } },
        { "GJ_starsIcon_001.png", false, "Level Grind level", "2 / star", { 140, 255, 140 } },
        { "GJ_timeIcon_001.png", false, "Event Level", "up to x4", { 255, 200, 90 } },
        { "pack_indicator.png"_spr, true, "Grind Pack", "+50", { 255, 150, 255 } },
        { "GJ_sFriendsIcon_001.png", false, "Clan weekly tasks", "+40-80", { 120, 220, 255 } },
    };
    float y = h - 48.f;
    for (auto const& way : ways) {
        CCSprite* icon = way.spr ? CCSprite::create(way.icon) : CCSprite::createWithSpriteFrameName(way.icon);
        if (icon) {
            icon->setScale(13.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
            icon->setPosition({ 13.f, y });
            m_tasks->addChild(icon);
        }
        auto wayLbl = lbl(m_tasks, way.text, { 24.f, y }, 0.28f, "bigFont.fnt");
        wayLbl->limitLabelWidth(w - 80.f, 0.28f, 0.12f);
        wayLbl->setColor(way.color);
        lbl(m_tasks, way.reward, { w - 8.f, y }, 0.34f, "goldFont.fnt", { 1.f, 0.5f });
        y -= 20.f;
    }
}

void ClansLayer::buildActions() {
    this->clearSection(m_actions);
    auto menu = overlayMenu(m_actions);
    Ref<ClansLayer> self = this;
    float const w = kActionsSize.width;

    if (!m_home.hasClan) {
        auto row = CCMenu::create();
        row->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));
        row->setContentSize({ w - 10.f, 34.f });
        row->setPosition({ w / 2.f, kActionsSize.height / 2.f });
        m_actions->addChild(row, 5);

        bool canAfford = m_home.wallet.balance >= m_home.createCost;
        row->addChild(CCMenuItemExt::createSpriteExtra(
            textButton("Create Clan", canAfford ? "GJ_button_01.png" : "GJ_button_04.png", 0.6f),
            [self](CCObject*) {
                if (!self) return;
                CreateClanPopup::create(self->m_home.createCost, self->m_home.wallet, [self] {
                    if (self) self->reload();
                })->show();
            }));

        if (!m_home.invites.empty()) {
            row->addChild(CCMenuItemExt::createSpriteExtra(
                textButton(fmt::format("Invites ({})", m_home.invites.size()).c_str(), "GJ_button_02.png", 0.6f),
                [self](CCObject*) {
                    if (self) ClanInvitesPopup::create(self->m_home.invites, [self] { if (self) self->reload(); })->show();
                }));
        }

        auto refresh = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        refresh->setScale(0.42f);
        row->addChild(CCMenuItemExt::createSpriteExtra(refresh, [self](CCObject*) { if (self) self->reload(); }));
        row->updateLayout();

        return;
    }

    auto& d = m_home.clan;
    struct Act {
        CCNode* top;
        const char* caption;
        geode::Function<void()> cb;
        int badge;
    };
    std::vector<Act> acts;

    acts.push_back({ accountTop("rankIcon_1_001.png", 0.9f, AccountBaseColor::Blue), m_browsingInClan ? "My Clan" : "All Clans", [self] {
        if (!self) return;
        self->m_browsingInClan = !self->m_browsingInClan;
        if (self->m_browsingInClan) {
            self->closeEmotePicker();
            self->unschedule(schedule_selector(ClansLayer::pollChat));
        }
        Loader::get()->queueInMainThread([self] {
            if (!self) return;
            if (self->m_browsingInClan) self->buildBrowse();
            else self->buildClanView();
            self->buildActions();
        });
    }, 0 });
    acts.push_back({ frameTop("accountBtn_friends_001.png"), "Members", [self] {
        if (self) ClanMembersPopup::create(self->m_home.clan, [self] { if (self) self->reload(); })->show();
    }, 0 });
    acts.push_back({ frameTop("accountBtn_messages_001.png"), "Activity", [self] {
        if (self) ClanFeedPopup::create(self->m_home.clan, self->m_home.now)->show();
    }, 0 });
    acts.push_back({ accountTop("rankIcon_top10_001.png", 0.8f, AccountBaseColor::Purple), "War", [self] {
        if (self) ClanWarPopup::create(self->m_home.clan, self->m_home.now)->show();
    }, 0 });
    acts.push_back({ AccountButtonSprite::create(clanui::gpIcon(1.f, 1.f), AccountBaseColor::Blue), "Treasury", [self] {
        if (self) ClanTreasuryPopup::create(self->m_home.clan, self->m_home.wallet, self->m_home.now, [self] { if (self) self->reload(); })->show();
    }, 0 });
    if (d.myRole >= 1) {
        acts.push_back({ frameTop("accountBtn_requests_001.png"), "Requests", [self] {
            if (self) ClanRequestsPopup::create(self->m_home.clan, [self] { if (self) self->reload(); })->show();
        }, static_cast<int>(d.requests.size()) });
    }
    if (d.myRole == 2) {
        acts.push_back({ frameTop("accountBtn_settings_001.png"), "Settings", [self] {
            if (self) ClanSettingsPopup::create(self->m_home.clan.info, [self] { if (self) self->reload(); })->show();
        }, 0 });
    }

    bool leaderWithMembers = d.myRole == 2 && d.info.members > 1;
    acts.push_back({ accountTop("GJ_deleteIcon_001.png", 0.8f, AccountBaseColor::Purple), leaderWithMembers ? "Disband" : "Leave",
        [self, leaderWithMembers] {
            if (!self) return;
            auto name = self->m_home.clan.info.name;
            if (leaderWithMembers) {
                createQuickPopup(
                    "Disband clan?",
                    fmt::format("<cr>{}</c> will be deleted for every member. To leave instead, make someone else the Leader in Members.", name),
                    "Cancel", "Disband",
                    [self](auto, bool yes) {
                        if (!yes || !self) return;
                        M::getInstance().runAction("clan_disband", M::authBody(), "Disbanding...", "The clan was disbanded.", [self](matjson::Value const&) {
                            if (self) self->reload();
                        });
                    }
                );
                return;
            }
            createQuickPopup(
                "Leave clan?",
                fmt::format("Leave <cy>{}</c>? Your Grind Points stay yours, but stop counting for the clan.", name),
                "Stay", "Leave",
                [self](auto, bool yes) {
                    if (!yes || !self) return;
                    M::getInstance().runAction("clan_leave", M::authBody(), "Leaving...", "You left the clan.", [self](matjson::Value const&) {
                        if (self) self->reload();
                    });
                }
            );
        }, 0 });

    size_t const cols = acts.size() <= 6 ? 3 : 4;
    float const gap = 2.f;
    float const cellW = std::floor((w - 8.f - gap * (cols - 1)) / cols);
    auto grid = CCNode::create();
    grid->setLayout(ColumnLayout::create()->setGap(2.f)->setAxisReverse(true)->setAutoScale(false));
    grid->setContentSize({ w - 8.f, kActionsSize.height - 6.f });
    grid->setAnchorPoint({ 0.5f, 0.5f });
    grid->setPosition({ w / 2.f, kActionsSize.height / 2.f });
    m_actions->addChild(grid, 5);
    for (size_t i = 0; i < acts.size(); i += cols) {
        auto row = CCMenu::create();
        row->setLayout(RowLayout::create()->setGap(gap)->setAutoScale(false));
        row->setContentSize({ w - 8.f, 36.f });
        grid->addChild(row);
        for (size_t j = i; j < std::min(acts.size(), i + cols); ++j) {
            actionButton(row, acts[j].top, acts[j].caption, std::move(acts[j].cb), acts[j].badge, cellW);
        }
        row->updateLayout();
    }
    grid->updateLayout();
}

void ClansLayer::onEnter() {
    BaseLayer::onEnter();
    if (m_chat && m_home.hasClan) this->schedule(schedule_selector(ClansLayer::pollChat), kChatPoll);
}

void ClansLayer::openSharedLevel(int id) {
    auto glm = GameLevelManager::get();
    glm->m_searchType = SearchType::Type19;
    auto open = [](GJGameLevel* level) {
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, LevelInfoLayer::scene(level, false)));
    };
    if (auto saved = glm->getSavedLevel(id)) {
        open(saved);
        return;
    }
    auto search = GJSearchObject::create(SearchType::Type19, numToString(id));
    std::string key = search->getKey();
    if (auto stored = glm->getStoredOnlineLevels(key.c_str()); stored && stored->count() > 0) {
        open(static_cast<GJGameLevel*>(stored->objectAtIndex(0)));
        return;
    }
    m_pendingLevelKey = key;
    m_pendingLevelWait = 0.f;
    glm->m_levelManagerDelegate = nullptr;
    glm->getOnlineLevels(search);
    Notification::create("Loading level...", NotificationIcon::Loading, 1.f)->show();
    this->schedule(schedule_selector(ClansLayer::pollSharedLevel), 0.1f);
}

void ClansLayer::pollSharedLevel(float dt) {
    m_pendingLevelWait += dt;
    auto stored = GameLevelManager::get()->getStoredOnlineLevels(m_pendingLevelKey.c_str());
    if (stored && stored->count() > 0) {
        this->unschedule(schedule_selector(ClansLayer::pollSharedLevel));
        m_pendingLevelKey.clear();
        auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, LevelInfoLayer::scene(level, false)));
        return;
    }
    if (m_pendingLevelWait > 10.f) {
        this->unschedule(schedule_selector(ClansLayer::pollSharedLevel));
        m_pendingLevelKey.clear();
        Notification::create("Couldn't load that level", NotificationIcon::Error)->show();
    }
}

void ClansLayer::openMention(std::string const& name) {
    auto lower = string::toLower(name);
    for (auto const& m : m_home.clan.members) {
        if (string::toLower(m.username) == lower) {
            GrindProfilePopup::create(m.accountId)->show();
            return;
        }
    }
    Notification::create(fmt::format("{} is not in your clan", name), NotificationIcon::Info)->show();
}

void ClansLayer::textChanged(CCTextInputNode*) {
    if (!m_chatInput || !m_mentionBar) return;
    m_mentionBar->removeAllChildren();
    m_mentionBar->setVisible(false);

    std::string text = m_chatInput->getString();
    auto at = text.rfind('@');
    if (at == std::string::npos || (at > 0 && text[at - 1] != ' ')) return;
    std::string typed = text.substr(at + 1);
    if (typed.find(' ') != std::string::npos) return;
    auto prefix = string::toLower(typed);

    int myId = GJAccountManager::get()->m_accountID;
    int shown = 0;
    Ref<ClansLayer> self = this;
    for (auto const& m : m_home.clan.members) {
        if (m.accountId == myId) continue;
        if (!string::startsWith(string::toLower(m.username), prefix)) continue;
        auto spr = ButtonSprite::create(m.username.c_str(), "bigFont.fnt", "GJ_button_04.png", 0.6f);
        spr->setScale(0.45f);
        std::string name = m.username;
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, at, name](CCObject*) {
            if (!self || !self->m_chatInput) return;
            std::string now = self->m_chatInput->getString();
            self->m_chatInput->setString(now.substr(0, std::min(at, now.size())) + "@" + name + " ");
            self->m_mentionBar->removeAllChildren();
            self->m_mentionBar->setVisible(false);
            self->m_chatInput->focus();
        });
        m_mentionBar->addChild(btn);
        if (++shown == 4) break;
    }
    if (shown > 0) {
        m_mentionBar->setVisible(true);
        m_mentionBar->updateLayout();
    }
}

}
