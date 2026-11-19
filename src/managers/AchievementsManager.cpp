#include "Auth.hpp"
#include "AchievementsManager.hpp"
#include "APIClient.hpp"
#include "GrindClansManager.hpp"
#include "NotifyManager.hpp"

#include <Geode/utils/web.hpp>
#include <fmt/format.h>
#include "Consent.hpp"

namespace levelgrind::achievements {

std::vector<Category> const& categories() {
    static std::vector<Category> list = {
        { "levels", "Levels", "GJ_starsIcon_001.png" },
        { "demons", "Demons", "GJ_demonIcon_001.png" },
        { "platformer", "Platformer", "GJ_moonsIcon_001.png" },
        { "events", "Events", "gj_dailyCrown_001.png" },
        { "packs", "Packs", "pack_indicator.png"_spr, true },
        { "gp", "Grind Points", "grind_point_icon.png"_spr, true },
        { "clans", "Clans", "GJ_sFriendsIcon_001.png" },
        { "pets", "Pets", "GJ_likesIcon_001.png" },
    };
    return list;
}

Category const* category(std::string const& id) {
    for (auto const& c : categories()) {
        if (id == c.id) return &c;
    }
    return nullptr;
}

static Achievement parse(matjson::Value const& v) {
    Achievement a;
    a.id = v["id"].asString().unwrapOrDefault();
    a.category = v["category"].asString().unwrapOrDefault();
    a.title = v["title"].asString().unwrapOrDefault();
    a.description = v["description"].asString().unwrapOrDefault();
    a.target = std::max<long long>(1, v["target"].asInt().unwrapOr(1));
    a.progress = v["progress"].asInt().unwrapOr(0);
    a.gp = static_cast<int>(v["gp"].asInt().unwrapOr(0));
    a.unlocked = v["unlocked"].asBool().unwrapOr(false);
    return a;
}

void check(geode::Function<void(std::vector<Achievement> const&)> onDone) {
    if (!consent::granted()) {
        if (onDone) onDone({});
        return;
    }
    static TaskHolder<web::WebResponse> task;
    auto body = GrindClansManager::authBody();
    if (body["accountId"].asInt().unwrapOr(0) <= 0) {
        if (onDone) onDone({});
        return;
    }
    auto done = std::make_shared<geode::Function<void(std::vector<Achievement> const&)>>(std::move(onDone));
    task.spawn(auth::post(fmt::format("{}/achievements_check", APIClient::getInstance().baseUrl), std::move(body), std::chrono::seconds{ 15 }), [done](web::WebResponse res) {
        std::vector<Achievement> all;
        auto json = res.json();
        if (res.ok() && json) {
            auto v = json.unwrap();
            if (auto arr = v["achievements"].asArray()) {
                for (auto const& a : arr.unwrap()) all.push_back(parse(a));
            }
            if (auto arr = v["unlocked"].asArray()) {
                for (auto const& n : arr.unwrap()) {
                    auto a = parse(n);
                    notify::showAchievement(a.title, a.description, a.gp);
                }
            }
        }
        if (*done) (*done)(all);
    });
}

}
