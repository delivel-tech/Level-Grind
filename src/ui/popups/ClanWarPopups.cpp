#include "ClanWarPopups.hpp"
#include "../ClanUI.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <fmt/format.h>

namespace levelgrind {

using M = GrindClansManager;

namespace {

CCLabelBMFont* text(CCNode* parent, std::string const& str, CCPoint pos, float scale, const char* font = "bigFont.fnt", CCPoint anchor = { 0.5f, 0.5f }) {
    auto l = CCLabelBMFont::create(str.c_str(), font);
    l->setScale(scale);
    l->setAnchorPoint(anchor);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

CCNode* clanSide(ClanInfo const& c, long long score, bool winning) {
    auto node = CCNode::create();
    node->setContentSize({ 110.f, 90.f });
    node->setAnchorPoint({ 0.5f, 0.5f });
    auto em = clanui::emblem(c.emblemId, c.colorId, 34.f, c.logoId);
    em->setAnchorPoint({ 0.5f, 0.5f });
    em->ignoreAnchorPointForPosition(false);
    em->setPosition({ 55.f, 60.f });
    node->addChild(em);
    auto name = text(node, c.name, { 55.f, 30.f }, 0.4f);
    name->limitLabelWidth(104.f, 0.4f, 0.12f);
    name->setColor(M::clanColor(c.colorId));
    if (c.level >= clanui::kRainbowLevel) clanui::makeRainbow(name);
    auto pts = text(node, M::formatGP(score), { 55.f, 10.f }, 0.5f, "goldFont.fnt");
    if (winning) pts->setColor({ 140, 255, 140 });
    return node;
}

}

ClanWarPopup* ClanWarPopup::create(ClanDetails const& d, long long now) {
    auto ret = new ClanWarPopup;
    if (ret->init(d, now)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanWarPopup::init(ClanDetails const& d, long long now) {
    if (!BasePopup::init(340.f, 230.f)) return false;
    this->setTitle("Clan War");
    float const w = right();

    if (!d.hasWar) {
        text(m_mainLayer, "No opponent this week", { w / 2.f, centerY() + 10.f }, 0.42f);
    } else {
        auto box = clanui::darkBox({ w - 30.f, 120.f }, 70);
        box->setPosition({ w / 2.f, top() - 104.f });
        m_mainLayer->addChild(box);

        auto mine = clanSide(d.info, d.warMine, d.warMine > d.warTheirs);
        mine->setPosition({ w / 2.f - 85.f, top() - 100.f });
        m_mainLayer->addChild(mine);
        auto theirs = clanSide(d.warOpponent, d.warTheirs, d.warTheirs > d.warMine);
        theirs->setPosition({ w / 2.f + 85.f, top() - 100.f });
        m_mainLayer->addChild(theirs);
        text(m_mainLayer, "VS", { w / 2.f, top() - 88.f }, 0.7f, "goldFont.fnt");

        long long total = d.warMine + d.warTheirs;
        auto bar = ProgressBar::create(ProgressBar::Style::Solid);
        bar->setFillColor(M::clanColor(d.info.colorId));
        bar->updateProgress(total > 0 ? static_cast<float>(d.warMine) / total * 100.f : 50.f);
        bar->setScale(0.75f);
        bar->setPosition({ w / 2.f, top() - 152.f });
        m_mainLayer->addChild(bar);

        text(m_mainLayer, fmt::format("Ends in {}  -  Win: +{} GP each", clanui::countdown(d.warEndsAt - now), d.warReward),
            { w / 2.f, top() - 170.f }, 0.3f)->setOpacity(200);
    }

    if (d.hasLastWar) {
        std::string result = d.lastWarDraw ? "Draw" : d.lastWarWon ? "Won" : "Lost";
        auto last = text(m_mainLayer, fmt::format("Last war: {} vs {} ({} - {})", result, d.lastWarOpponent,
            M::formatGP(d.lastWarMine), M::formatGP(d.lastWarTheirs)), { w / 2.f, 34.f }, 0.3f);
        last->limitLabelWidth(w - 30.f, 0.3f, 0.12f);
        last->setColor(d.lastWarDraw ? ccColor3B{ 220, 220, 220 } : d.lastWarWon ? ccColor3B{ 140, 255, 140 } : ccColor3B{ 255, 130, 130 });
    }
    text(m_mainLayer, fmt::format("Wars won: {}", d.warWins), { w / 2.f, 16.f }, 0.3f, "goldFont.fnt");
    return true;
}

ClanTreasuryPopup* ClanTreasuryPopup::create(ClanDetails const& d, GPWallet const& wallet, long long now, geode::Function<void()> onChanged) {
    auto ret = new ClanTreasuryPopup;
    if (ret->init(d, wallet, now, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanTreasuryPopup::init(ClanDetails const& d, GPWallet const& wallet, long long now, geode::Function<void()> onChanged) {
    if (!BasePopup::init(320.f, 230.f)) return false;
    this->setTitle("Clan Treasury");
    m_details = d;
    m_wallet = wallet;
    m_now = now;
    m_onChanged = std::move(onChanged);
    m_root = CCNode::create();
    m_root->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(m_root);
    this->build();
    return true;
}

void ClanTreasuryPopup::build() {
    m_root->removeAllChildren();
    float const w = right();
    auto const& d = m_details;
    Ref<ClanTreasuryPopup> self = this;

    auto amount = clanui::gpAmount(d.treasury, 0.7f);
    amount->setPosition({ w / 2.f, top() - 48.f });
    m_root->addChild(amount);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_root->addChild(menu, 5);

    auto donateRow = CCMenu::create();
    donateRow->setLayout(RowLayout::create()->setGap(6.f));
    donateRow->setContentSize({ w - 40.f, 30.f });
    donateRow->setPosition({ w / 2.f, top() - 80.f });
    m_root->addChild(donateRow, 5);
    for (int value : { 50, 100, 500, 1000 }) {
        bool can = m_wallet.balance >= value;
        auto spr = ButtonSprite::create(fmt::format("+{}", value).c_str(), 44, true, "bigFont.fnt", can ? "GJ_button_01.png" : "GJ_button_04.png", 26.f, 0.5f);
        spr->setScale(0.75f);
        donateRow->addChild(CCMenuItemExt::createSpriteExtra(spr, [self, value](CCObject*) { if (self) self->donate(value); }));
    }
    donateRow->updateLayout();

    bool leader = d.myRole == 2;
    auto item = [&](float y, std::string const& name, std::string const& desc, long long until, std::string const& id, int cost) {
        auto box = clanui::darkBox({ w - 30.f, 42.f }, 70);
        box->setPosition({ w / 2.f, y });
        m_root->addChild(box);
        text(m_root, name, { 24.f, y + 8.f }, 0.4f, "goldFont.fnt", { 0.f, 0.5f });
        std::string sub = until > m_now ? fmt::format("Active - {}", clanui::countdown(until - m_now)) : desc;
        auto subLbl = text(m_root, sub, { 24.f, y - 9.f }, 0.26f, "bigFont.fnt", { 0.f, 0.5f });
        subLbl->limitLabelWidth(w - 130.f, 0.26f, 0.1f);
        if (until > m_now) subLbl->setColor({ 140, 255, 140 });
        bool can = leader && d.treasury >= cost;
        auto spr = ButtonSprite::create(M::formatGP(cost).c_str(), 54, true, "bigFont.fnt", can ? "GJ_button_02.png" : "GJ_button_04.png", 26.f, 0.5f);
        spr->setScale(0.7f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, id, cost](CCObject*) { if (self) self->buy(id, cost); });
        btn->setPosition({ w - 50.f, y });
        btn->setEnabled(leader);
        menu->addChild(btn);
    };
    item(top() - 128.f, "GP Boost", fmt::format("+{}% GP for every member for {}h", d.boostPct, d.boostHours), d.boostUntil, "boost", d.boostCost);
    item(top() - 176.f, "Golden Name", fmt::format("Golden clan name for {} days", d.goldDays), d.goldUntil, "gold", d.goldCost);
}

void ClanTreasuryPopup::donate(int amount) {
    if (m_wallet.balance < amount) {
        Notification::create("Not enough Grind Points", NotificationIcon::Error)->show();
        return;
    }
    auto body = M::authBody();
    body["amount"] = amount;
    Ref<ClanTreasuryPopup> self = this;
    M::getInstance().runAction("clan_treasury_donate", body, "Donating...", fmt::format("Donated {} GP!", amount), [self, amount](matjson::Value const& v) {
        if (!self) return;
        self->m_details.treasury += amount;
        self->m_wallet.balance -= amount;
        if (v["wallet"].isObject()) self->m_wallet = M::parseWallet(v["wallet"]);
        self->build();
        if (self->m_onChanged) self->m_onChanged();
    });
}

void ClanTreasuryPopup::buy(std::string const& item, int cost) {
    if (m_details.myRole != 2) return;
    if (m_details.treasury < cost) {
        Notification::create("Not enough GP in the treasury", NotificationIcon::Error)->show();
        return;
    }
    auto body = M::authBody();
    body["item"] = item;
    Ref<ClanTreasuryPopup> self = this;
    M::getInstance().runAction("clan_treasury_buy", body, "Buying...", "Bought!", [self](matjson::Value const& v) {
        if (!self) return;
        auto const& e = v["extras"];
        self->m_details.treasury = e["treasury"].asInt().unwrapOr(self->m_details.treasury);
        self->m_details.boostUntil = e["boostUntil"].asInt().unwrapOr(0);
        self->m_details.goldUntil = e["goldUntil"].asInt().unwrapOr(0);
        self->build();
        if (self->m_onChanged) self->m_onChanged();
    });
}

}
