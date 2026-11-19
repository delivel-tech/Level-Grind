#include "AchievementsManager.hpp"
#include "GrindClansManager.hpp"
#include "APIClient.hpp"
#include "DataManager.hpp"
#include "../ui/layers/GrindCalendarLayer.hpp"

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <Geode/ui/Notification.hpp>
#include <fmt/format.h>
#include "Consent.hpp"

namespace levelgrind {

matjson::Value GrindClansManager::playerLook() {
    auto gm = GameManager::get();
    matjson::Value v;
    v["icon"] = gm->getPlayerFrame();
    v["color1"] = gm->getPlayerColor();
    v["color2"] = gm->getPlayerColor2();
    v["color3"] = gm->getPlayerGlow() ? gm->getPlayerGlowColor() : 0;
    return v;
}

matjson::Value GrindClansManager::authBody() {
    matjson::Value body = playerLook();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    return body;
}

auth::Future GrindClansManager::post(std::string const& endpoint, matjson::Value body) {
    return auth::post(fmt::format("{}/{}", APIClient::getInstance().baseUrl, endpoint), std::move(body));
}

static long long asLL(matjson::Value const& v) {
    return static_cast<long long>(v.asInt().unwrapOr(0));
}

static int asI(matjson::Value const& v, int fallback = 0) {
    return static_cast<int>(v.asInt().unwrapOr(fallback));
}

GPWallet GrindClansManager::parseWallet(matjson::Value const& v) {
    GPWallet w;
    w.balance = asLL(v["balance"]);
    w.lifetime = asLL(v["lifetime"]);
    w.todayEarned = asI(v["todayEarned"]);
    w.dailyCap = asI(v["dailyCap"], 400);
    return w;
}

ClanInfo GrindClansManager::parseClan(matjson::Value const& v) {
    ClanInfo c;
    c.id = asI(v["id"]);
    c.name = v["name"].asString().unwrapOrDefault();
    c.description = v["description"].asString().unwrapOrDefault();
    c.ownerId = asI(v["ownerId"]);
    c.colorId = asI(v["colorId"]);
    c.emblemId = asI(v["emblemId"], 1);
    c.joinType = asI(v["joinType"]);
    c.minGp = asI(v["minGp"]);
    c.xp = asLL(v["xp"]);
    c.level = asI(v["level"], 1);
    c.levelXp = asLL(v["levelXp"]);
    c.nextLevelXp = asLL(v["nextLevelXp"]);
    c.maxLevel = v["maxLevel"].asBool().unwrapOr(false);
    c.weekXp = asLL(v["weekXp"]);
    c.members = asI(v["members"]);
    c.maxMembers = asI(v["maxMembers"], 20);
    c.gpBonus = asI(v["gpBonus"]);
    c.featured = v["featured"].asBool().unwrapOr(false);
    c.rank = asI(v["rank"]);
    c.logoId = asLL(v["logoId"]);
    c.golden = v["golden"].asBool().unwrapOr(false);
    c.warWins = asI(v["warWins"]);
    return c;
}

ClanDetails GrindClansManager::parseDetails(matjson::Value const& v) {
    ClanDetails d;
    d.info = parseClan(v["info"]);
    d.weekRank = asI(v["weekRank"]);
    d.myRole = asI(v["myRole"]);
    d.logoPending = v["logoPending"].asBool().unwrapOr(false);
    d.hasPinned = parsePinned(v["pinned"], d.pinned);
    d.treasury = asLL(v["treasury"]);
    d.warWins = asI(v["warWins"]);
    d.boostUntil = asLL(v["boostUntil"]);
    d.goldUntil = asLL(v["goldUntil"]);
    if (v["shop"].isObject()) {
        d.boostCost = asI(v["shop"]["boostCost"], 1500);
        d.boostPct = asI(v["shop"]["boostPct"], 10);
        d.boostHours = asI(v["shop"]["boostHours"], 24);
        d.goldCost = asI(v["shop"]["goldCost"], 3000);
        d.goldDays = asI(v["shop"]["goldDays"], 7);
    }
    if (v["war"].isObject()) {
        d.hasWar = true;
        d.warOpponent = parseClan(v["war"]["opponent"]);
        d.warMine = asLL(v["war"]["myScore"]);
        d.warTheirs = asLL(v["war"]["theirScore"]);
        d.warEndsAt = asLL(v["war"]["endsAt"]);
        d.warReward = asI(v["war"]["reward"], 100);
    }
    if (v["lastWar"].isObject()) {
        d.hasLastWar = true;
        d.lastWarOpponent = v["lastWar"]["opponentName"].asString().unwrapOrDefault();
        d.lastWarMine = asLL(v["lastWar"]["myScore"]);
        d.lastWarTheirs = asLL(v["lastWar"]["theirScore"]);
        d.lastWarWon = v["lastWar"]["won"].asBool().unwrapOr(false);
        d.lastWarDraw = v["lastWar"]["draw"].asBool().unwrapOr(false);
    }

    if (auto arr = v["members"].asArray()) {
        for (auto const& m : arr.unwrap()) {
            ClanMemberInfo mi;
            mi.accountId = asI(m["accountId"]);
            mi.username = m["username"].asString().unwrapOrDefault();
            mi.role = asI(m["role"]);
            mi.icon = asI(m["icon"], 1);
            mi.color1 = asI(m["color1"]);
            mi.color2 = asI(m["color2"], 3);
            mi.color3 = asI(m["color3"]);
            mi.contributed = asLL(m["contributed"]);
            mi.weekContributed = asI(m["weekContributed"]);
            mi.lastActive = asLL(m["lastActive"]);
            mi.joinedAt = asLL(m["joinedAt"]);
            if (m["pet"].isObject()) {
                mi.hasPet = true;
                mi.petLevel = asI(m["pet"]["level"], 1);
                mi.petAppearance = m["pet"]["appearance"];
            }
            d.members.push_back(mi);
        }
    }
    if (auto arr = v["tasks"].asArray()) {
        for (auto const& t : arr.unwrap()) {
            ClanTaskInfo ti;
            ti.slot = asI(t["slot"]);
            ti.type = asI(t["type"]);
            ti.target = std::max(1, asI(t["target"], 1));
            ti.progress = asI(t["progress"]);
            ti.completed = t["completed"].asBool().unwrapOr(false);
            ti.rewardGp = asI(t["rewardGp"]);
            ti.tier = asI(t["tier"], ti.slot / 3);
            d.tasks.push_back(ti);
        }
    }
    if (auto arr = v["feed"].asArray()) {
        for (auto const& f : arr.unwrap()) {
            ClanFeedEntry e;
            e.id = asLL(f["id"]);
            e.kind = asI(f["kind"]);
            e.accountId = asI(f["accountId"]);
            e.username = f["username"].asString().unwrapOrDefault();
            e.value = asI(f["value"]);
            e.text = f["text"].asString().unwrapOrDefault();
            e.at = asLL(f["at"]);
            d.feed.push_back(e);
        }
    }
    if (auto arr = v["requests"].asArray()) {
        for (auto const& r : arr.unwrap()) {
            ClanJoinRequestInfo ri;
            ri.accountId = asI(r["accountId"]);
            ri.username = r["username"].asString().unwrapOrDefault();
            ri.icon = asI(r["icon"], 1);
            ri.color1 = asI(r["color1"]);
            ri.color2 = asI(r["color2"], 3);
            ri.color3 = asI(r["color3"]);
            ri.lifetimeGp = asLL(r["lifetimeGp"]);
            ri.at = asLL(r["at"]);
            d.requests.push_back(ri);
        }
    }
    return d;
}

bool GrindClansManager::parseOk(web::WebResponse& res, std::string& error) {
    auto json = res.json();
    if (res.ok() && json && json.unwrap()["ok"].asBool().unwrapOr(false)) return true;
    if (res.code() == 404 && !json) {
        error = "Grind Clans are not live on the server yet.";
    } else {
        error = APIClient::errorMessage(res, fmt::format("Try again later (code {}).", res.code()));
    }
    return false;
}

ClanHome GrindClansManager::parseHome(web::WebResponse& res) {
    ClanHome h;
    if (!parseOk(res, h.error)) return h;
    auto json = res.json().unwrap();

    h.ok = true;
    h.wallet = parseWallet(json["wallet"]);
    h.createCost = asI(json["createCost"], 300);
    h.weekEndsAt = asLL(json["weekEndsAt"]);
    h.now = asLL(json["now"]);
    if (auto arr = json["requested"].asArray()) {
        for (auto const& id : arr.unwrap()) h.requested.insert(asI(id));
    }
    if (auto arr = json["invites"].asArray()) {
        for (auto const& v : arr.unwrap()) {
            ClanInviteInfo inv;
            inv.clan = parseClan(v["clan"]);
            inv.invitedBy = v["invitedBy"].asString().unwrapOrDefault();
            inv.at = asLL(v["at"]);
            h.invites.push_back(inv);
        }
    }
    if (json["clan"].isObject()) {
        h.hasClan = true;
        h.clan = parseDetails(json["clan"]);
    }

    auto& mgr = GrindClansManager::getInstance();
    mgr.m_lastWallet = h.wallet;
    mgr.m_hasWallet = true;
    mgr.m_myClanRole = h.hasClan ? h.clan.myRole : -1;
    return h;
}

ClanBrowse GrindClansManager::parseBrowse(web::WebResponse& res) {
    ClanBrowse b;
    if (!parseOk(res, b.error)) return b;
    auto json = res.json().unwrap();
    b.ok = true;
    b.page = asI(json["page"]);
    b.hasMore = json["hasMore"].asBool().unwrapOr(false);
    b.ranked = json["ranked"].asBool().unwrapOr(true);
    b.weekEndsAt = asLL(json["weekEndsAt"]);
    if (auto arr = json["clans"].asArray()) {
        for (auto const& c : arr.unwrap()) b.clans.push_back(parseClan(c));
    }
    return b;
}

bool GrindClansManager::parsePinned(matjson::Value const& v, ClanChatMessage& out) {
    if (!v.isObject() || asLL(v["id"]) <= 0) return false;
    out.id = asLL(v["id"]);
    out.accountId = asI(v["accountId"]);
    out.username = v["username"].asString().unwrapOrDefault();
    out.message = v["message"].asString().unwrapOrDefault();
    out.at = asLL(v["at"]);
    return true;
}

std::vector<ClanChatMessage> GrindClansManager::parseChat(web::WebResponse& res, bool& ok) {
    std::vector<ClanChatMessage> out;
    std::string err;
    ok = parseOk(res, err);
    if (!ok) return out;
    auto json = res.json().unwrap();
    if (auto arr = json["messages"].asArray()) {
        for (auto const& m : arr.unwrap()) {
            ClanChatMessage msg;
            msg.id = asLL(m["id"]);
            msg.accountId = asI(m["accountId"]);
            msg.username = m["username"].asString().unwrapOrDefault();
            msg.icon = asI(m["icon"], 1);
            msg.color1 = asI(m["color1"]);
            msg.color2 = asI(m["color2"], 3);
            msg.color3 = asI(m["color3"]);
            msg.message = m["message"].asString().unwrapOrDefault();
            msg.at = asLL(m["at"]);
            out.push_back(msg);
        }
    }
    return out;
}

std::vector<ClanPalette> const& GrindClansManager::palette() {
    static std::vector<ClanPalette> p = {
        { "Gold",    { 255, 200,  60 } },
        { "Orange",  { 255, 130,  40 } },
        { "Red",     { 235,  70,  70 } },
        { "Pink",    { 255, 110, 190 } },
        { "Purple",  { 160,  90, 255 } },
        { "Indigo",  {  90, 100, 255 } },
        { "Blue",    {  60, 160, 255 } },
        { "Cyan",    {  40, 220, 230 } },
        { "Mint",    {  60, 230, 160 } },
        { "Green",   {  90, 220,  60 } },
        { "Lime",    { 200, 240,  60 } },
        { "Silver",  { 200, 205, 215 } },
    };
    return p;
}

ccColor3B GrindClansManager::clanColor(int colorId) {
    auto const& p = palette();
    if (colorId < 0 || colorId >= static_cast<int>(p.size())) return p[0].color;
    return p[colorId].color;
}

std::string GrindClansManager::roleName(int role) {
    switch (role) {
        case 2: return "Leader";
        case 1: return "Officer";
        default: return "Member";
    }
}

ccColor3B GrindClansManager::roleColor(int role) {
    switch (role) {
        case 2: return { 255, 210, 70 };
        case 1: return { 120, 220, 255 };
        default: return { 255, 255, 255 };
    }
}

std::string GrindClansManager::taskTitle(ClanTaskInfo const& t) {
    switch (t.type) {
        case 0: return fmt::format("Beat {} Level Grind levels", t.target);
        case 1: return fmt::format("Earn {} Grind Points", t.target);
        case 2: return fmt::format("Beat {} live event levels", t.target);
        case 3: return fmt::format("Beat {} levels of 6+ stars/moons", t.target);
        case 4: return fmt::format("Beat {} platformer levels", t.target);
        case 5: return fmt::format("Beat {} demons", t.target);
    }
    return fmt::format("Task ({})", t.target);
}

const char* GrindClansManager::tierName(int tier) {
    switch (tier) {
        case 0: return "Easy";
        case 1: return "Medium";
        case 2: return "Hard";
        case 3: return "Very Hard";
    }
    return "Tasks";
}

ccColor3B GrindClansManager::tierColor(int tier) {
    switch (tier) {
        case 0: return { 120, 255, 120 };
        case 1: return { 255, 210, 80 };
        case 2: return { 255, 100, 100 };
        case 3: return { 220, 110, 255 };
    }
    return { 255, 255, 255 };
}

int GrindClansManager::tierCount(std::vector<ClanTaskInfo> const& tasks) {
    int top = 0;
    for (auto const& t : tasks) top = std::max(top, t.tier);
    return tasks.empty() ? 1 : top + 1;
}

const char* GrindClansManager::taskIcon(int type) {
    switch (type) {
        case 0: return "GJ_starsIcon_001.png";
        case 1: return nullptr;
        case 2: return "GJ_timeIcon_001.png";
        case 3: return "diffIcon_05_btn_001.png";
        case 4: return "GJ_moonsIcon_001.png";
        case 5: return "GJ_demonIcon_001.png";
    }
    return "GJ_starsIcon_001.png";
}

std::string GrindClansManager::feedText(ClanFeedEntry const& e) {
    switch (e.kind) {
        case 0: return fmt::format("<cy>{}</c> founded the clan", e.username);
        case 1: return fmt::format("<cg>{}</c> joined", e.username);
        case 2: return fmt::format("<cr>{}</c> left", e.username);
        case 3: return fmt::format("<cr>{}</c> was removed by {}", e.username, e.text);
        case 4: return fmt::format("<cl>{}</c> beat a level for <cy>+{} GP</c>", e.username, e.value);
        case 5: {
            static const char* names[] = { "Level run", "GP push", "Event rush", "Hard grind", "Platformer run", "Demon hunt" };
            const char* n = (e.value >= 0 && e.value < 6) ? names[e.value] : "Weekly";
            return fmt::format("Weekly task done: <cg>{}</c> (thanks {})", n, e.username);
        }
        case 6: return fmt::format("The clan reached <cy>level {}</c>!", e.value);
        case 7: return fmt::format("<cl>{}</c> finished a Grind Pack for <cy>+{} GP</c>", e.username, e.value);
        case 8: return fmt::format("<cl>{}</c> is now {}", e.username, e.value == 1 ? "an Officer" : "a Member");
        case 9: return fmt::format("<cy>{}</c> is the new Leader", e.username);
        case 10: return fmt::format("The clan <cg>won the Clan War</c>! Every member got <cy>+{} GP</c>", e.value);
        case 11: return "The clan <cr>lost the Clan War</c>";
        case 12: return fmt::format("<cl>{}</c> donated <cy>{} GP</c> to the treasury", e.username, e.value);
        case 13: return fmt::format("<cl>{}</c> bought {} for <cy>{} GP</c>", e.username, e.text == "boost" ? "a GP Boost" : "a Golden Name", e.value);
    }
    return e.username;
}

std::string GrindClansManager::timeAgo(long long at, long long now) {
    long long d = std::max(0LL, now - at);
    if (d < 60) return "now";
    if (d < 3600) return fmt::format("{}m", d / 60);
    if (d < 86400) return fmt::format("{}h", d / 3600);
    return fmt::format("{}d", d / 86400);
}

std::string GrindClansManager::formatGP(long long gp) {
    if (gp >= 1000000) return fmt::format("{:.1f}M", gp / 1000000.0);
    if (gp >= 10000) return fmt::format("{:.1f}K", gp / 1000.0);
    return numToString(gp);
}

static constexpr auto kQueueKey = "gp-claim-queue";

void GrindClansManager::queueLevelClaim(int levelId, int stars, bool platformer) {
    if (!consent::granted()) return;
    if (levelId <= 0 || stars <= 0) return;

    auto queue = Mod::get()->getSavedValue<matjson::Value>(kQueueKey, matjson::Value::array());
    if (!queue.isArray()) queue = matjson::Value::array();

    for (auto const& item : queue.asArray().unwrap()) {
        if (asI(item["levelId"]) == levelId) {
            this->flushClaims();
            return;
        }
    }

    matjson::Value item;
    item["levelId"] = levelId;
    item["stars"] = stars;
    item["platformer"] = platformer;
    item["at"] = static_cast<long long>(std::time(nullptr));
    queue.push(item);

    while (queue.size() > 200) {
        auto arr = queue.asArray().unwrap();
        arr.erase(arr.begin());
        queue = matjson::Value(arr);
    }

    Mod::get()->setSavedValue(kQueueKey, queue);
    this->flushClaims();
}

void GrindClansManager::flushClaims() {
    if (!consent::granted()) return;
    if (GJAccountManager::get()->m_accountID <= 0) return;
    if (GJAccountManager::get()->m_accountID <= 0) return;
    if (!m_packClaiming) this->sendNextPackClaim();
    if (!m_claiming) this->sendNextClaim();
}

static constexpr auto kPackQueueKey = "gp-pack-queue";
static constexpr auto kPacksClaimedKey = "gp-packs-claimed";

static matjson::Value savedIntArray(const char* key) {
    auto v = Mod::get()->getSavedValue<matjson::Value>(key, matjson::Value::array());
    return v.isArray() ? v : matjson::Value::array();
}

static bool arrayHas(matjson::Value const& arr, int id) {
    for (auto const& v : arr.asArray().unwrap()) {
        if (asI(v) == id) return true;
    }
    return false;
}

void GrindClansManager::checkPacks(std::vector<GrindPack> const& packs) {
    m_packs = packs;
    m_packsLoaded = true;

    auto gsm = GameStatsManager::get();
    auto claimed = savedIntArray(kPacksClaimedKey);
    auto queue = savedIntArray(kPackQueueKey);
    bool added = false;

    for (auto const& p : packs) {
        if (p.id <= 0 || arrayHas(claimed, p.id) || arrayHas(queue, p.id)) continue;
        if (gsm->hasCompletedOnlineLevel(p.levels.id1)
            && gsm->hasCompletedOnlineLevel(p.levels.id2)
            && gsm->hasCompletedOnlineLevel(p.levels.id3)) {
            queue.push(p.id);
            added = true;
        }
    }

    if (added) {
        Mod::get()->setSavedValue(kPackQueueKey, queue);
        this->flushClaims();
    }
}

void GrindClansManager::checkAllPacks() {
    if (!consent::granted()) return;
    if (m_packsLoaded) {
        this->checkPacks(m_packs);
        return;
    }
    m_packListTask.spawn(APIClient::getInstance().getGrindPacks(), [this](web::WebResponse res) {
        auto parsed = APIClient::getInstance().getGrindPacksParse(res);
        if (parsed.ok) this->checkPacks(parsed.packs);
    });
}

void GrindClansManager::sendNextPackClaim() {
    auto queue = savedIntArray(kPackQueueKey);
    if (queue.size() == 0) {
        m_packClaiming = false;
        return;
    }
    m_packClaiming = true;
    int packId = asI(queue.asArray().unwrap().front());

    auto body = authBody();
    body["packId"] = packId;

    m_packTask.spawn(post("gp_claim_pack", body), [this, packId](web::WebResponse res) {
        auto json = res.json();
        bool ok = res.ok() && json && json.unwrap()["ok"].asBool().unwrapOr(false);

        auto drop = [packId](bool claimed) {
            auto q = savedIntArray(kPackQueueKey);
            auto arr = q.asArray().unwrap();
            std::erase_if(arr, [packId](matjson::Value const& v) { return asI(v) == packId; });
            Mod::get()->setSavedValue(kPackQueueKey, matjson::Value(arr));
            if (claimed) {
                auto c = savedIntArray(kPacksClaimedKey);
                if (!arrayHas(c, packId)) c.push(packId);
                Mod::get()->setSavedValue(kPacksClaimedKey, c);
            }
        };

        if (!ok) {
            if (res.code() == 404 || res.code() == 400) {
                drop(false);
                this->sendNextPackClaim();
                return;
            }
            m_packClaiming = false;
            return;
        }

        auto data = json.unwrap();
        m_lastWallet = parseWallet(data["wallet"]);
        m_hasWallet = true;
        drop(true);
        achievements::check();
        this->sendNextPackClaim();
    });
}

static constexpr auto kCoinQueueKey = "gp-coin-queue";

void GrindClansManager::queueCoinClaim(int levelId, std::vector<int> const& coins) {
    if (!consent::granted()) return;
    if (levelId <= 0 || coins.empty()) return;
    auto queue = Mod::get()->getSavedValue<matjson::Value>(kCoinQueueKey, matjson::Value::array());
    if (!queue.isArray()) queue = matjson::Value::array();
    auto arr = queue.asArray().unwrap();
    std::erase_if(arr, [levelId](matjson::Value const& v) { return asI(v["levelId"]) == levelId; });
    matjson::Value item;
    item["levelId"] = levelId;
    std::vector<matjson::Value> list;
    for (int c : coins) list.push_back(c);
    item["coins"] = matjson::Value(list);
    arr.push_back(item);
    while (arr.size() > 200) arr.erase(arr.begin());
    Mod::get()->setSavedValue(kCoinQueueKey, matjson::Value(arr));
    this->flushCoinClaims();
}

void GrindClansManager::flushCoinClaims() {
    if (!consent::granted()) return;
    if (m_coinClaiming) return;
    auto queue = Mod::get()->getSavedValue<matjson::Value>(kCoinQueueKey, matjson::Value::array());
    if (!queue.isArray() || queue.size() == 0) return;
    if (GJAccountManager::get()->m_accountID <= 0) return;
    m_coinClaiming = true;

    auto item = queue.asArray().unwrap().front();
    int levelId = asI(item["levelId"]);
    auto body = authBody();
    body["levelId"] = levelId;
    body["coins"] = item["coins"];

    m_coinTask.spawn(post("gp_claim_coins", body), [this, levelId](web::WebResponse res) {
        m_coinClaiming = false;
        if (res.ok()) GrindCalendarLayer::checkGoals();
        bool drop = res.ok() || res.code() == 400;
        if (!drop) return;
        auto q = Mod::get()->getSavedValue<matjson::Value>(kCoinQueueKey, matjson::Value::array());
        if (!q.isArray()) return;
        auto arr = q.asArray().unwrap();
        std::erase_if(arr, [levelId](matjson::Value const& v) { return asI(v["levelId"]) == levelId; });
        Mod::get()->setSavedValue(kCoinQueueKey, matjson::Value(arr));
        if (!arr.empty()) this->flushCoinClaims();
    });
}

void GrindClansManager::sendNextClaim() {
    auto queue = Mod::get()->getSavedValue<matjson::Value>(kQueueKey, matjson::Value::array());
    if (!queue.isArray() || queue.size() == 0) {
        m_claiming = false;
        return;
    }
    m_claiming = true;

    auto item = queue.asArray().unwrap().front();
    int levelId = asI(item["levelId"]);

    auto body = authBody();
    body["levelId"] = levelId;
    body["stars"] = asI(item["stars"]);
    body["platformer"] = item["platformer"].asBool().unwrapOr(false);
    if (auto at = item["at"].asInt(); at && at.unwrap() > 0) body["completedAt"] = at.unwrap();

    m_claimTask.spawn(
        post("gp_claim_level", body),
        [this, levelId](web::WebResponse res) {
            auto json = res.json();
            bool ok = res.ok() && json && json.unwrap()["ok"].asBool().unwrapOr(false);

            if (!ok) {
                if (res.code() == 400) {
                    auto q = Mod::get()->getSavedValue<matjson::Value>(kQueueKey, matjson::Value::array());
                    if (q.isArray() && q.size() > 0) {
                        auto arr = q.asArray().unwrap();
                        arr.erase(arr.begin());
                        Mod::get()->setSavedValue(kQueueKey, matjson::Value(arr));
                    }
                }
                m_claiming = false;
                return;
            }

            auto data = json.unwrap();
            std::string reason = data["reason"].asString().unwrapOrDefault();
            m_lastWallet = parseWallet(data["wallet"]);
            m_hasWallet = true;
            if (reason.empty()) GrindCalendarLayer::checkGoals();

            if (reason == "daily_cap") {
                m_claiming = false;
                return;
            }

            auto q = Mod::get()->getSavedValue<matjson::Value>(kQueueKey, matjson::Value::array());
            if (q.isArray()) {
                auto arr = q.asArray().unwrap();
                std::erase_if(arr, [levelId](matjson::Value const& v) { return asI(v["levelId"]) == levelId; });
                Mod::get()->setSavedValue(kQueueKey, matjson::Value(arr));
            }

            achievements::check();
            this->sendNextClaim();
        }
    );
}

void GrindClansManager::runAction(
    std::string const& endpoint, matjson::Value body,
    std::string const& loadingText, std::string const& successText,
    geode::Function<void(matjson::Value const&)> onSuccess
) {
    auto uPopup = UploadActionPopup::create(nullptr, loadingText);
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;

    m_actionTask.spawn(
        post(endpoint, std::move(body)),
        [uPopupRef, successText, onSuccess = std::move(onSuccess)](web::WebResponse res) mutable {
            std::string error;
            bool ok = parseOk(res, error);
            if (!ok) {
                if (uPopupRef) uPopupRef->showFailMessage(fmt::format("Failed! {}", error));
                return;
            }
            if (uPopupRef) uPopupRef->showSuccessMessage(successText);
            if (onSuccess) onSuccess(res.json().unwrapOrDefault());
        }
    );
}

}
