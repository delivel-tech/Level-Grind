#include "GrindLeaderboardsLayer.hpp"
#include "../popups/GuidePopup.hpp"
#include <Geode/ui/Label.hpp>
#include "../ClanUI.hpp"
#include "../popups/GrindProfilePopup.hpp"
#include "../../managers/APIClient.hpp"

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/BasedButton.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

constexpr CCSize kListSize { 356.f, 220.f };
constexpr float kRowH = 34.f;

constexpr const char* kPeriods[] = { "daily", "weekly", "monthly" };
constexpr const char* kPeriodNames[] = { "Daily", "Weekly", "Monthly" };
constexpr const char* kTypes[] = { "stars", "moons", "demons", "coins", "gp" };

long long nowSec() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long long asLL(matjson::Value const& v) {
    return static_cast<long long>(v.asInt().unwrapOr(0));
}

int asI(matjson::Value const& v, int fallback = 0) {
    return static_cast<int>(v.asInt().unwrapOr(fallback));
}

CCSprite* typeIcon(int type, float size) {
    CCSprite* spr = nullptr;
    switch (type) {
        case 0: spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"); break;
        case 1: spr = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png"); break;
        case 2: spr = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png"); break;
        case 3: spr = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png"); break;
        default: spr = CCSprite::create("grind_point_icon.png"_spr); break;
    }
    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    spr->setScale(size / std::max(spr->getContentWidth(), spr->getContentHeight()));
    return spr;
}

GrindLeaderboardsLayer::Entry parseEntry(matjson::Value const& v) {
    GrindLeaderboardsLayer::Entry e;
    e.rank = asI(v["rank"]);
    e.accountId = asI(v["accountId"]);
    e.username = v["username"].asString().unwrapOrDefault();
    e.value = asLL(v["value"]);
    e.icon = asI(v["icon"], 1);
    e.color1 = asI(v["color1"]);
    e.color2 = asI(v["color2"], 3);
    e.color3 = asI(v["color3"]);
    return e;
}

}

GrindLeaderboardsLayer* GrindLeaderboardsLayer::create() {
    auto ret = new GrindLeaderboardsLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GrindLeaderboardsLayer::init() {
    if (!BaseLayer::init()) return false;
    GuidePopup::addInfoButton(this, GuidePage::LeaderboardsGuide, { 25.f, 25.f });

    m_period = std::clamp(Mod::get()->getSavedValue<int>("leaderboards-period", 0), 0, 2);
    m_type = std::clamp(Mod::get()->getSavedValue<int>("leaderboards-type", 0), 0, 4);

    float const cx = m_winSize.width / 2.f;
    float const cy = m_winSize.height / 2.f - 6.f;

    m_list = cue::ListNode::create(kListSize, cue::Brown, cue::ListBorderStyle::SlimLevels);
    m_list->setPosition({ cx, cy });
    m_list->setAutoUpdate(false);
    this->addChild(m_list, 6);

    auto tabsMenu = CCMenu::create();
    tabsMenu->setLayout(RowLayout::create()->setGap(12.f));
    tabsMenu->setScale(0.8f);
    tabsMenu->setContentSize({ 400.f, 40.f });
    tabsMenu->setPosition({ cx, cy + kListSize.height / 2.f + 28.f });
    this->addChild(tabsMenu, 5);
    for (int i = 0; i < 3; i++) {
        auto tab = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, kPeriodNames[i], this, menu_selector(GrindLeaderboardsLayer::onTab));
        tab->setTag(i);
        tab->toggle(i == m_period);
        tabsMenu->addChild(tab);
        m_tabs.push_back(tab);
    }
    tabsMenu->updateLayout();

    m_typeMenu = CCMenu::create();
    m_typeMenu->setLayout(ColumnLayout::create()->setGap(8.f)->setAxisReverse(true)->setAutoScale(false));
    m_typeMenu->setContentSize({ 40.f, 250.f });
    m_typeMenu->setScale(0.7f);
    m_typeMenu->setPosition({ cx + kListSize.width / 2.f + 42.f, cy });
    this->addChild(m_typeMenu, 6);
    this->buildTypeButtons();

    m_timer = geode::Label::create("", "bigFont.fnt");
    m_timer->setScale(0.32f);
    m_timer->setOpacity(190);
    m_timer->setAnchorPoint({ 1.f, 0.f });
    m_timer->setPosition({ m_winSize.width - 8.f, 3.f });
    this->addChild(m_timer, 6);

    addSideArt(this, SideArt::Bottom, SideArtStyle::LayerGray, false);

    m_status = geode::Label::create("", "bigFont.fnt");
    m_status->setScale(0.4f);
    m_status->setPosition({ cx, cy });
    this->addChild(m_status, 8);

    this->schedule(schedule_selector(GrindLeaderboardsLayer::tick), 1.f);
    this->load();
    return true;
}

std::string GrindLeaderboardsLayer::key() const {
    return fmt::format("{}:{}", kPeriods[m_period], kTypes[m_type]);
}

void GrindLeaderboardsLayer::onTab(CCObject* sender) {
    int i = sender->getTag();
    for (auto tab : m_tabs) tab->toggle(tab->getTag() == i);
    if (i == m_period) return;
    m_period = i;
    Mod::get()->setSavedValue<int>("leaderboards-period", i);
    this->load();
}

void GrindLeaderboardsLayer::buildTypeButtons() {
    m_typeMenu->removeAllChildren();
    WeakRef<GrindLeaderboardsLayer> weak = this;
    for (int i = 0; i < 5; i++) {
        bool on = i == m_type;
        auto bg = NineSlice::create(on ? "GJ_button_02.png" : "GJ_button_01.png");
        bg->setContentSize({ 40.f, 40.f });
        auto icon = typeIcon(i, 24.f);
        icon->setPosition({ 20.f, 20.f });
        bg->addChild(icon);
        auto btn = CCMenuItemExt::createSpriteExtra(bg, [weak, i](CCObject*) {
            auto self = weak.lock();
            if (!self || self->m_type == i) return;
            self->m_type = i;
            Mod::get()->setSavedValue<int>("leaderboards-type", i);
            Loader::get()->queueInMainThread([weak] {
                auto self = weak.lock();
                if (!self) return;
                self->buildTypeButtons();
                self->load();
            });
        });
        m_typeMenu->addChild(btn);
    }
    m_typeMenu->updateLayout();
}

void GrindLeaderboardsLayer::load() {
    int seq = ++m_requestSeq;
    auto k = this->key();
    if (auto it = m_cache.find(k); it != m_cache.end() && nowSec() - it->second.fetchedAt < 60) {
        this->show(it->second);
        return;
    }

    m_list->clear();
    m_list->updateLayout(false);
    m_status->setString("");
    if (m_spinner) m_spinner->removeFromParent();
    auto spinner = LoadingSpinner::create(40.f);
    spinner->setPosition(m_list->getPosition());
    this->addChild(spinner, 9);
    m_spinner = spinner;

    auto req = web::WebRequest();
    req.param("period", kPeriods[m_period]);
    req.param("type", kTypes[m_type]);
    int me = GJAccountManager::get()->m_accountID;
    if (me > 0) req.param("accountId", numToString(me));
    req.timeout(asp::Duration::fromSecs(20));

    WeakRef<GrindLeaderboardsLayer> weak = this;
    m_task.spawn(req.get(fmt::format("{}/leaderboard", APIClient::getInstance().baseUrl)), [weak, seq, k](web::WebResponse res) {
        auto self = weak.lock();
        if (!self || seq != self->m_requestSeq) return;
        if (self->m_spinner) {
            self->m_spinner->removeFromParent();
            self->m_spinner = nullptr;
        }
        auto json = res.json();
        if (!res.ok() || !json || !json.unwrap()["ok"].asBool().unwrapOr(false)) {
            self->m_status->setString("Failed to load");
            return;
        }
        auto const& v = json.unwrap();
        Board board;
        if (auto arr = v["entries"].asArray()) {
            for (auto const& e : arr.unwrap()) board.entries.push_back(parseEntry(e));
        }
        if (v["me"].isObject()) board.me = parseEntry(v["me"]);
        board.players = asI(v["players"]);
        board.resetsAt = asLL(v["resetsAt"]);
        board.fetchedAt = nowSec();
        board.serverOffset = asLL(v["now"]) - board.fetchedAt;
        self->m_cache[k] = board;
        self->show(board);
    });
}

void GrindLeaderboardsLayer::show(Board const& board) {
    if (m_spinner) {
        m_spinner->removeFromParent();
        m_spinner = nullptr;
    }
    m_resetsAt = board.resetsAt;
    m_serverOffset = board.serverOffset;
    this->tick(0.f);

    int me = GJAccountManager::get()->m_accountID;
    m_list->clear();
    for (auto const& e : board.entries) m_list->addCell(this->makeRow(e, me > 0 && e.accountId == me));
    m_list->updateLayout(false);
    m_list->scrollToTop();

    m_status->setString(board.entries.empty() ? "Nobody here yet" : "");
}

void GrindLeaderboardsLayer::tick(float) {
    if (!m_timer) return;
    if (m_resetsAt <= 0) {
        m_timer->setString("");
        return;
    }
    long long left = m_resetsAt - (nowSec() + m_serverOffset);
    if (left <= 0) {
        m_timer->setString("");
        return;
    }
    m_timer->setString(fmt::format("{} resets in {}", kPeriodNames[m_period], clanui::countdown(left)).c_str());
}

CCNode* GrindLeaderboardsLayer::makeRow(Entry const& e, bool mine) {
    float const w = kListSize.width;
    auto cell = CCMenu::create();
    cell->ignoreAnchorPointForPosition(false);
    cell->setContentSize({ w, kRowH });

    if (mine) cell->addChild(CCLayerColor::create({ 255, 255, 255, 40 }, w, kRowH), -1);

    if (e.rank >= 1 && e.rank <= 3) {
        auto trophy = CCSprite::createWithSpriteFrameName(e.rank == 1 ? "rankIcon_1_001.png" : e.rank == 2 ? "rankIcon_top50_001.png" : "rankIcon_top100_001.png");
        trophy->setScale(22.f / std::max(trophy->getContentWidth(), trophy->getContentHeight()));
        trophy->setPosition({ 22.f, kRowH / 2.f });
        cell->addChild(trophy);
    } else {
        auto rank = geode::Label::create(numToString(e.rank).c_str(), "bigFont.fnt");
        rank->setLimitLabelWidth(30.f, 0.4f, 0.15f);
        rank->setPosition({ 22.f, kRowH / 2.f });
        cell->addChild(rank);
    }

    int const id = e.accountId;
    std::string const name = e.username;
    auto open = [id, name](CCObject*) { GrindProfilePopup::create(id, name)->show(); };

    auto icon = cue::PlayerIcon::create(cue::Icons{ IconType::Cube, std::max(1, e.icon), e.color1, e.color2, e.color3 > 0 ? e.color3 : -1 });
    icon->setScale(0.62f);
    auto iconBtn = CCMenuItemExt::createSpriteExtra(icon, open);
    iconBtn->setPosition({ 54.f, kRowH / 2.f });
    cell->addChild(iconBtn);

    auto nameLbl = geode::Label::create(name.empty() ? "Player" : name.c_str(), "goldFont.fnt");
    nameLbl->setLimitLabelWidth(170.f, 0.6f, 0.2f);
    auto nameBtn = CCMenuItemExt::createSpriteExtra(nameLbl, open);
    nameBtn->setAnchorPoint({ 0.f, 0.5f });
    nameBtn->setPosition({ 74.f, kRowH / 2.f });
    cell->addChild(nameBtn);

    auto metric = typeIcon(m_type, 15.f);
    metric->setPosition({ w - 18.f, kRowH / 2.f });
    cell->addChild(metric);
    auto value = geode::Label::create(numToString(e.value).c_str(), "bigFont.fnt");
    value->setAnchorPoint({ 1.f, 0.5f });
    value->setLimitLabelWidth(80.f, 0.42f, 0.15f);
    value->setPosition({ w - 30.f, kRowH / 2.f });
    cell->addChild(value);

    return cell;
}

}
