#include "GrindListsData.hpp"
#include "APIClient.hpp"

#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/utils/base64.hpp>
#include <algorithm>
#include <fmt/format.h>

namespace levelgrind::lists {

Filters Filters::load() {
    auto mod = Mod::get();
    Filters f;
    f.classic = mod->getSavedValue<bool>("lists-classic", false);
    f.platformer = mod->getSavedValue<bool>("lists-platformer", false);
    f.demons = mod->getSavedValue<bool>("lists-demons", false);
    f.coins = mod->getSavedValue<bool>("lists-coins", false);
    f.minDiamonds = mod->getSavedValue<int>("lists-min-diamonds", 0);
    f.maxRequired = mod->getSavedValue<int>("lists-max-required", 0);
    f.maxLeft = mod->getSavedValue<int>("lists-max-left", 0);
    f.hideFinished = mod->getSavedValue<bool>("lists-hide-finished", false);
    f.featuredOnly = mod->getSavedValue<bool>("lists-featured-only", false);
    f.unratedOnly = mod->getSavedValue<bool>("lists-unrated-only", false) && !f.featuredOnly;
    for (int i = 0; i < 11; i++) f.diffs[i] = mod->getSavedValue<bool>(fmt::format("lists-diff-{}", i), false);
    f.sort = static_cast<Sort>(std::clamp(mod->getSavedValue<int>("lists-sort", 0), 0, 3));
    return f;
}

void Filters::save() const {
    auto mod = Mod::get();
    mod->setSavedValue("lists-classic", classic);
    mod->setSavedValue("lists-platformer", platformer);
    mod->setSavedValue("lists-demons", demons);
    mod->setSavedValue("lists-coins", coins);
    mod->setSavedValue("lists-min-diamonds", minDiamonds);
    mod->setSavedValue("lists-max-required", maxRequired);
    mod->setSavedValue("lists-max-left", maxLeft);
    mod->setSavedValue("lists-hide-finished", hideFinished);
    mod->setSavedValue("lists-featured-only", featuredOnly);
    mod->setSavedValue("lists-unrated-only", unratedOnly);
    for (int i = 0; i < 11; i++) mod->setSavedValue(fmt::format("lists-diff-{}", i), diffs[i]);
    mod->setSavedValue("lists-sort", static_cast<int>(sort));
}

matjson::Value Filters::serverBody() const {
    matjson::Value body;
    std::vector<std::string> types;
    if (classic) types.push_back("classic");
    if (platformer) types.push_back("platformer");
    if (demons) types.push_back("demon");
    if (coins) types.push_back("coins");
    body["types"] = types;
    body["minDiamonds"] = minDiamonds;
    body["maxRequired"] = maxRequired;
    std::vector<int> picked;
    for (int i = 0; i <= 5; i++) if (diffs[i]) picked.push_back(i);
    if (demons) for (int i = 6; i <= 10; i++) if (diffs[i]) picked.push_back(i);
    body["difficulties"] = picked;
    return body;
}

bool listsMode() {
    return Mod::get()->getSavedValue<bool>("lists-mode", false);
}

void setListsMode(bool on) {
    Mod::get()->setSavedValue("lists-mode", on);
}

web::WebFuture fetchLists(Filters const& f) {
    auto req = web::WebRequest();
    req.bodyJSON(f.serverBody());
    return req.post(fmt::format("{}/get_lists", APIClient::getInstance().baseUrl));
}

static int asI(matjson::Value const& v, int fallback = 0) {
    return static_cast<int>(v.asInt().unwrapOr(fallback));
}

std::vector<GrindList> parseLists(web::WebResponse& res, Filters const& f, std::string& error) {
    std::vector<GrindList> out;
    auto json = res.json();
    if (!res.ok() || !json || !json.unwrap()["ok"].asBool().unwrapOr(false)) {
        error = res.code() == 404 ? "Grind Lists are not live on the server yet."
                                  : APIClient::errorMessage(res, fmt::format("Failed to load lists (code {})", res.code()));
        return out;
    }

    auto gsm = GameStatsManager::get();
    auto arr = json.unwrap()["lists"].asArray();
    if (!arr) return out;

    for (auto const& v : arr.unwrap()) {
        GrindList l;
        l.id = asI(v["id"]);
        l.name = v["name"].asString().unwrapOrDefault();
        l.description = v["description"].asString().unwrapOrDefault();
        l.author = v["author"].asString().unwrapOrDefault();
        l.accountId = asI(v["accountId"]);
        l.difficulty = asI(v["difficulty"], -1);
        l.diamonds = asI(v["diamonds"]);
        l.levelsRequired = asI(v["levelsRequired"]);
        l.featured = v["featured"].asBool().unwrapOr(false);
        l.likes = asI(v["likes"]);
        l.downloads = asI(v["downloads"]);
        l.uploadDate = asI(v["uploadDate"]);
        l.totalStars = asI(v["totalStars"]);
        l.totalMoons = asI(v["totalMoons"]);
        l.addedAt = v["addedAt"].asInt().unwrapOr(0);
        if (auto levels = v["levels"].asArray()) {
            for (auto const& lv : levels.unwrap()) {
                ListLevel level;
                level.id = asI(lv["id"]);
                level.stars = asI(lv["stars"]);
                level.moon = lv["moon"].asBool().unwrapOr(false);
                level.demon = lv["demon"].asBool().unwrapOr(false);
                level.coins = asI(lv["coins"]);
                level.loaded = lv["loaded"].asBool().unwrapOr(true);
                l.levels.push_back(level);
            }
        }
        if (l.id <= 0 || l.levels.empty()) continue;

        for (auto const& level : l.levels) {
            if (gsm && gsm->hasCompletedOnlineLevel(level.id)) l.beaten++;
        }
        int required = l.levelsRequired > 0 ? l.levelsRequired : static_cast<int>(l.levels.size());
        l.left = std::max(0, required - l.beaten);
        l.finished = l.left == 0;

        if (f.hideFinished && l.finished) continue;
        if (f.featuredOnly && !l.featured) continue;
        if (f.unratedOnly && l.featured) continue;
        if (f.maxLeft > 0 && l.left > f.maxLeft) continue;
        out.push_back(std::move(l));
    }

    auto value = [](GrindList const& l) {
        return l.finished ? -1.0 : static_cast<double>(l.diamonds) / std::max(1, l.left);
    };
    switch (f.sort) {
        case Sort::Value:
            std::stable_sort(out.begin(), out.end(), [&](auto const& a, auto const& b) {
                double va = value(a), vb = value(b);
                if (va != vb) return va > vb;
                return a.diamonds > b.diamonds;
            });
            break;
        case Sort::AlmostDone:
            std::stable_sort(out.begin(), out.end(), [](auto const& a, auto const& b) {
                if (a.finished != b.finished) return !a.finished;
                if (a.left != b.left) return a.left < b.left;
                return a.diamonds > b.diamonds;
            });
            break;
        case Sort::Diamonds:
            std::stable_sort(out.begin(), out.end(), [](auto const& a, auto const& b) { return a.diamonds > b.diamonds; });
            break;
        case Sort::Newest:
            std::stable_sort(out.begin(), out.end(), [](auto const& a, auto const& b) { return a.addedAt > b.addedAt; });
            break;
    }
    return out;
}

GJLevelList* makeLevelList(GrindList const& l) {
    auto list = GJLevelList::create();
    list->m_listID = l.id;
    list->m_listName = l.name;
    list->m_listDesc = utils::base64::encode(l.description);
    list->m_creatorName = l.author;
    list->m_accountID = l.accountId;
    list->m_difficulty = l.difficulty;
    list->m_diamonds = l.diamonds;
    list->m_levelsToClaim = l.levelsRequired;
    list->m_featured = l.featured;
    list->m_likes = l.likes;
    list->m_downloads = l.downloads;
    list->m_uploadDate = l.uploadDate;
    list->m_uploaded = true;
    list->m_isEditable = false;
    list->m_levels.clear();
    std::string ids;
    for (auto const& level : l.levels) {
        list->m_levels.push_back(level.id);
        if (!ids.empty()) ids += ",";
        ids += numToString(level.id);
    }
    list->m_levelsString = ids;
    return list;
}

}
