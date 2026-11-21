#include "GrindProfilePopup.hpp"
#include "ClanPopups.hpp"
#include "UserManagePopup.hpp"
#include "../ClanUI.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/GDUsers.hpp"
#include "../../managers/GrindClansManager.hpp"
#include "../../managers/PetManager.hpp"
#include "../../utils/RoleBadges.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>
#include <Geode/binding/GameManager.hpp>
#include "../PetAnim.hpp"

namespace levelgrind {

namespace {

constexpr float kWidth = 290.f;
constexpr float kHeight = 200.f;

using M = GrindClansManager;

long long asLL(matjson::Value const& v) {
    return static_cast<long long>(v.asInt().unwrapOr(0));
}

int asI(matjson::Value const& v, int fallback = 0) {
    return static_cast<int>(v.asInt().unwrapOr(fallback));
}

CCLabelBMFont* text(CCNode* parent, std::string const& str, CCPoint pos, float scale, const char* font = "bigFont.fnt", CCPoint anchor = { 0.5f, 0.5f }) {
    auto lbl = CCLabelBMFont::create(str.c_str(), font);
    lbl->setScale(scale);
    lbl->setAnchorPoint(anchor);
    lbl->setPosition(pos);
    parent->addChild(lbl);
    return lbl;
}

ccColor3B rarityColor(int rarity) {
    switch (rarity) {
        case 2: return { 90, 200, 255 };
        case 3: return { 255, 170, 60 };
        case 4: return { 255, 90, 120 };
        default: if (rarity > 4) return { 255, 110, 200 };
    }
    return { 210, 210, 210 };
}

CCSprite* fit(CCSprite* spr, float size) {
    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    spr->setScale(size / std::max(spr->getContentWidth(), spr->getContentHeight()));
    return spr;
}

}

GrindProfilePopup* GrindProfilePopup::create(int accountId, std::string const& username) {
    auto ret = new GrindProfilePopup;
    if (ret->init(accountId, username)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GrindProfilePopup::init(int accountId, std::string const& username) {
    if (!BasePopup::init(kWidth, kHeight)) return false;
    m_data.accountId = accountId;
    m_data.username = username;
    m_nameHint = username;

    m_root = CCNode::create();
    m_root->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(m_root);

    auto spinner = LoadingSpinner::create(36.f);
    spinner->setPosition({ kWidth / 2.f, kHeight / 2.f - 10.f });
    m_mainLayer->addChild(spinner, 10);
    m_spinner = spinner;

    this->build();
    this->load();
    return true;
}

void GrindProfilePopup::load() {
    auto req = web::WebRequest();
    if (m_data.accountId > 0) req.param("accountId", numToString(m_data.accountId));
    else req.param("username", m_nameHint);
    req.timeout(asp::Duration::fromSecs(20));

    Ref<GrindProfilePopup> self = this;
    m_task.spawn(req.get(fmt::format("{}/player_profile", APIClient::getInstance().baseUrl)), [self](web::WebResponse res) {
        if (!self) return;
        if (self->m_spinner) {
            self->m_spinner->removeFromParent();
            self->m_spinner = nullptr;
        }
        auto json = res.json();
        if (!res.ok() || !json) {
            self->m_failed = true;
            self->m_loaded = true;
            self->build();
            if (self->m_data.accountId > 0) self->loadGD();
            return;
        }
        auto const& v = json.unwrap();
        auto& d = self->m_data;
        d.exists = v["exists"].asBool().unwrapOr(false);
        int id = asI(v["accountId"]);
        if (id > 0) d.accountId = id;
        auto name = v["username"].asString().unwrapOrDefault();
        if (!name.empty()) d.username = name;
        d.icon = asI(v["icon"]);
        d.color1 = asI(v["color1"]);
        d.color2 = asI(v["color2"], 3);
        d.color3 = asI(v["color3"]);
        d.gpLifetime = asLL(v["gp"]["lifetime"]);
        d.gpWeek = asLL(v["gp"]["week"]);
        d.achUnlocked = asI(v["achievements"]["unlocked"]);
        d.achTotal = asI(v["achievements"]["total"]);
        d.streak = asI(v["streak"]);
        d.packs = asI(v["packs"]);
        d.last30Levels = asI(v["last30"]["levels"]);
        d.last30Stars = asI(v["last30"]["stars"]);
        d.last30Moons = asI(v["last30"]["moons"]);
        d.last30Demons = asI(v["last30"]["demons"]);
        if (v["clan"].isObject()) {
            auto const& c = v["clan"];
            d.hasClan = true;
            d.clanId = asI(c["id"]);
            d.clanName = c["name"].asString().unwrapOrDefault();
            d.clanColor = asI(c["colorId"]);
            d.clanEmblem = asI(c["emblemId"], 1);
            d.clanLogo = asLL(c["logoId"]);
            d.clanLevel = asI(c["level"], 1);
            d.clanRole = asI(c["role"]);
            d.clanJoinType = asI(c["joinType"]);
        }
        if (v["pet"].isObject()) {
            d.hasPet = true;
            d.petName = v["pet"]["name"].asString().unwrapOrDefault();
            d.petLevel = asI(v["pet"]["level"], 1);
            d.petRarity = asI(v["pet"]["rarity"], 1);
            d.hasAppearance = v["pet"]["appearance"].isObject();
            d.petMood = asI(v["pet"]["mood"], 1);
            if (d.hasAppearance) d.appearance = v["pet"]["appearance"];
        }
        self->m_loaded = true;
        self->build();
        if (d.accountId > 0 && (d.username.empty() || d.icon <= 0)) self->loadGD();
    });
}

void GrindProfilePopup::loadGD() {
    Ref<GrindProfilePopup> self = this;
    m_gdTask.spawn(gdusers::info(m_data.accountId), [self](web::WebResponse res) {
        if (!self) return;
        auto user = gdusers::parseInfo(res);
        if (!user) return;
        auto& d = self->m_data;
        if (!user->name.empty()) d.username = user->name;
        d.icon = user->icon;
        d.color1 = user->color1;
        d.color2 = user->color2;
        d.color3 = user->glowColor < 0 ? 0 : user->glowColor;
        self->build();
    });
}

void GrindProfilePopup::build() {
    m_root->removeAllChildren();
    float const w = kWidth;
    float const h = kHeight;

    this->buildHeader(m_root, w, h);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_root->addChild(menu, 5);

    int const id = m_data.accountId;
    auto gdSpr = ButtonSprite::create("GD Profile", "goldFont.fnt", "GJ_button_01.png", 0.7f);
    gdSpr->setScale(0.6f);
    auto gdBtn = CCMenuItemExt::createSpriteExtra(gdSpr, [id](CCObject*) {
        if (id > 0) ProfilePage::create(id, false)->show();
    });
    gdBtn->setPosition({ w / 2.f, 17.f });
    gdBtn->setEnabled(id > 0);
    if (id <= 0) gdSpr->setColor({ 120, 120, 120 });
    menu->addChild(gdBtn);

    if (!m_loaded) return;

    if (m_failed) {
        text(m_root, "Failed to load", { w / 2.f, h / 2.f - 10.f }, 0.42f);
        return;
    }
    if (id <= 0) {
        text(m_root, "Player not found", { w / 2.f, h / 2.f - 10.f }, 0.42f);
        return;
    }
    if (!m_data.exists) {
        auto box = clanui::darkBox({ w - 24.f, 96.f }, 70);
        box->setPosition({ w / 2.f, 82.f });
        m_root->addChild(box);
        text(m_root, "No Grind Profile", { w / 2.f, 82.f }, 0.5f, "goldFont.fnt");
        return;
    }

    this->buildStats(m_root, w, 100.f);
    this->buildChips(m_root, w, 48.f);
}

void GrindProfilePopup::buildHeader(CCNode* root, float w, float h) {
    auto const& d = m_data;

    auto iconBg = clanui::darkBox({ 44.f, 44.f }, 70);
    iconBg->setPosition({ 52.f, h - 40.f });
    root->addChild(iconBg);
    if (d.icon > 0 || !m_loaded) {
        auto icon = cue::PlayerIcon::create(cue::Icons{ IconType::Cube, std::max(1, d.icon), d.color1, d.color2, d.color3 > 0 ? d.color3 : -1 });
        icon->setScale(0.85f);
        icon->setPosition({ 52.f, h - 40.f });
        root->addChild(icon);
    }

    auto badges = roles::forAccount(d.accountId);
    std::string name = d.username.empty() ? (m_loaded ? "Unknown player" : "Loading...") : d.username;
    auto nameLbl = text(root, name, { 84.f, badges.empty() ? h - 40.f : h - 30.f }, 0.7f, "goldFont.fnt", { 0.f, 0.5f });
    nameLbl->limitLabelWidth(w - 130.f, 0.7f, 0.3f);

    if (!badges.empty()) {
        auto row = CCMenu::create();
        row->setLayout(RowLayout::create()->setGap(3.f)->setAutoScale(false)->setAxisAlignment(AxisAlignment::Start));
        row->ignoreAnchorPointForPosition(false);
        row->setAnchorPoint({ 0.f, 0.5f });
        row->setContentSize({ w - 130.f, 16.f });
        row->setPosition({ 84.f, h - 51.f });
        for (auto const& b : badges) row->addChild(roles::badgeButton(b, 14.f));
        row->updateLayout();
        root->addChild(row, 5);
    }

    auto pos = DataManager::getInstance().getUserPosition();
    if (d.accountId > 0 && (pos == GrindPosition::Admin || pos == GrindPosition::Owner)) {
        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        root->addChild(menu, 5);
        auto top = CCSprite::create("user_config_spr.png"_spr);
        auto spr = AccountButtonSprite::create(top, AccountBaseColor::Blue);
        spr->getTopNode()->setScale(0.95f);
        spr->getTopNode()->setPosition(spr->getTopNode()->getPosition() + CCPoint{ 1.5f, -1.f });
        spr->setScale(0.55f);
        Ref<GrindProfilePopup> self = this;
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [self](CCObject*) {
            if (!self) return;
            auto const& d = self->m_data;
            auto score = GJUserScore::create();
            score->m_accountID = d.accountId;
            score->m_userName = d.username;
            score->m_playerCube = std::max(1, d.icon);
            score->m_color1 = d.color1;
            score->m_color2 = d.color2;
            score->m_color3 = d.color3;
            score->m_glowEnabled = d.color3 > 0;
            UserManagePopup::create(score)->show();
        });
        btn->setPosition({ w - 24.f, h - 24.f });
        menu->addChild(btn);
    }
}

void GrindProfilePopup::buildStats(CCNode* root, float w, float y) {
    auto const& d = m_data;
    float const boxW = w - 24.f;
    auto box = clanui::darkBox({ boxW, 56.f }, 70);
    box->setPosition({ w / 2.f, y });
    root->addChild(box);

    struct Col {
        CCSprite* icon;
        std::string value;
        std::string caption;
    };
    bool moons = d.last30Moons > d.last30Stars;
    Col cols[] = {
        { clanui::gpIcon(1.f, 1.f), M::formatGP(d.gpLifetime), fmt::format("+{} this week", M::formatGP(d.gpWeek)) },
        { CCSprite::createWithSpriteFrameName("rankIcon_1_001.png"), fmt::format("{}/{}", d.achUnlocked, d.achTotal), "Achievements" },
        { CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), d.streak == 1 ? "1 day" : fmt::format("{} days", d.streak), "Streak" },
        { CCSprite::create("pack_indicator.png"_spr), numToString(d.packs), "Packs" },
        { CCSprite::createWithSpriteFrameName(moons ? "GJ_moonsIcon_001.png" : "GJ_starsIcon_001.png"),
            numToString(moons ? d.last30Moons : d.last30Stars), "Last 30 days" },
    };
    int const colCount = static_cast<int>(std::size(cols));
    float const colW = boxW / colCount;
    for (int i = 0; i < colCount; i++) {
        float x = 12.f + colW * (i + 0.5f);
        auto icon = fit(cols[i].icon, 15.f);
        icon->setPosition({ x, y + 15.f });
        root->addChild(icon);
        auto val = text(root, cols[i].value, { x, y - 1.f }, 0.4f);
        val->limitLabelWidth(colW - 8.f, 0.4f, 0.15f);
        auto cap = text(root, cols[i].caption, { x, y - 15.f }, 0.22f);
        cap->limitLabelWidth(colW - 6.f, 0.22f, 0.1f);
        cap->setOpacity(160);
        if (i < colCount - 1) {
            auto line = CCLayerColor::create({ 255, 255, 255, 25 }, 1.f, 40.f);
            line->setPosition({ 12.f + colW * (i + 1), y - 20.f });
            root->addChild(line);
        }
    }
}

void GrindProfilePopup::buildChips(CCNode* root, float w, float y) {
    auto const& d = m_data;
    float const chipW = (w - 30.f) / 2.f;
    float const chipH = 40.f;

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    root->addChild(menu, 5);

    auto chip = [&](float x) {
        auto node = CCNode::create();
        node->setContentSize({ chipW, chipH });
        auto box = clanui::darkBox({ chipW, chipH }, 70);
        box->setPosition({ chipW / 2.f, chipH / 2.f });
        node->addChild(box);
        node->setAnchorPoint({ 0.5f, 0.5f });
        node->setPosition({ x, y });
        return node;
    };

    float const leftX = 12.f + chipW / 2.f;
    float const rightX = w - 12.f - chipW / 2.f;

    auto clan = chip(leftX);
    if (d.hasClan) {
        auto em = clanui::emblem(d.clanEmblem, d.clanColor, 20.f, d.clanLogo);
        em->setAnchorPoint({ 0.5f, 0.5f });
        em->ignoreAnchorPointForPosition(false);
        em->setPosition({ 19.f, chipH / 2.f });
        clan->addChild(em);
        auto name = text(clan, d.clanName, { 38.f, chipH / 2.f + 6.f }, 0.34f, "bigFont.fnt", { 0.f, 0.5f });
        name->limitLabelWidth(chipW - 44.f, 0.34f, 0.12f);
        name->setColor(M::clanColor(d.clanColor));
        if (d.clanLevel >= clanui::kRainbowLevel) clanui::makeRainbow(name);
        if (d.clanLevel >= clanui::kPrestigeLevel) clanui::addPrestige(clan, { chipW, chipH }, M::clanColor(d.clanColor));
        auto sub = text(clan, fmt::format("{} - Lv {}", M::roleName(d.clanRole), d.clanLevel), { 38.f, chipH / 2.f - 8.f }, 0.24f, "bigFont.fnt", { 0.f, 0.5f });
        sub->setColor(M::roleColor(d.clanRole));
        sub->limitLabelWidth(chipW - 44.f, 0.24f, 0.1f);

        clan->setPosition({ 0.f, 0.f });
        clan->setAnchorPoint({ 0.f, 0.f });
        int clanId = d.clanId;
        auto btn = CCMenuItemExt::createSpriteExtra(clan, [clanId](CCObject*) {
            ClanViewPopup::create(clanId, [](ClanInfo const& info) {
                auto body = M::authBody();
                body["clanId"] = info.id;
                bool open = info.joinType == 0;
                M::getInstance().runAction(
                    "clan_join", body,
                    open ? "Joining..." : "Sending request...",
                    open ? fmt::format("Welcome to {}!", info.name) : "Request sent! An officer will look at it.",
                    [](matjson::Value const&) {}
                );
            })->show();
        });
        btn->m_scaleMultiplier = 1.05f;
        btn->setPosition({ leftX, y });
        menu->addChild(btn);
    } else {
        root->addChild(clan);
        int me = GJAccountManager::get()->m_accountID;
        if (M::getInstance().m_myClanRole >= 1 && d.accountId > 0 && d.accountId != me) {
            text(clan, "No clan", { chipW / 2.f - 26.f, chipH / 2.f }, 0.32f)->setOpacity(130);
            int target = d.accountId;
            std::string uname = d.username;
            auto spr = ButtonSprite::create("Invite", "bigFont.fnt", "GJ_button_01.png", 0.6f);
            spr->setScale(0.5f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [target, uname](CCObject*) {
                auto body = M::authBody();
                body["targetAccountId"] = target;
                body["targetUsername"] = uname;
                M::getInstance().runAction("clan_invite", body, "Inviting...", fmt::format("Invited {}!", uname), [](matjson::Value const&) {});
            });
            btn->setPosition({ 12.f + chipW - 30.f, y });
            menu->addChild(btn);
        } else {
            text(clan, "No clan", { chipW / 2.f, chipH / 2.f }, 0.32f)->setOpacity(130);
        }
    }

    auto pet = chip(rightX);
    root->addChild(pet);
    if (d.hasPet) {
        auto appearance = d.hasAppearance ? PetManager::appearanceFromJson(d.appearance) : PetManager::Appearance{};
        PetManager::OwnerLook owner;
        auto gm = GameManager::sharedState();
        owner.frame = std::max(1, d.icon);
        owner.color1 = gm->colorForIdx(d.color1);
        owner.color2 = gm->colorForIdx(d.color2);
        owner.glow = d.color3 > 0;
        owner.glowColor = gm->colorForIdx(d.color3);
        auto petSpr = PetManager::createPetSprite(PetManager::getInstance().getStyleByLevel(d.petLevel), appearance, owner);
        petSpr->setScale(0.72f);
        petSpr->setPosition({ 20.f, chipH / 2.f });
        pet->addChild(petSpr);
        petanim::playMood(petSpr, d.petMood);

        auto name = text(pet, d.petName.empty() ? "Pet" : d.petName, { 38.f, chipH / 2.f + 7.f }, 0.34f, "bigFont.fnt", { 0.f, 0.5f });
        name->limitLabelWidth(chipW - 44.f, 0.34f, 0.12f);
        auto rarity = PetManager::getInstance().getRarityFromInt(d.petRarity);
        auto sub = text(pet, fmt::format("Lv {} - {}", d.petLevel, rarity.empty() ? "Common" : rarity), { 38.f, chipH / 2.f - 8.f }, 0.26f, "bigFont.fnt", { 0.f, 0.5f });
        sub->setColor(rarityColor(d.petRarity));
        sub->limitLabelWidth(chipW - 44.f, 0.26f, 0.1f);
    } else {
        text(pet, "No pet", { chipW / 2.f, chipH / 2.f }, 0.32f)->setOpacity(130);
    }
}

}
