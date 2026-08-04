#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<NewWeeklyAchResponse> {
    static geode::Result<NewWeeklyAchResponse> fromJson(matjson::Value const& json) {
        NewWeeklyAchResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<NewWeeklyAchResponse> BackendManager::newWeeklyAch(AchievementCellInfo info1, AchievementCellInfo info2, AchievementCellInfo info3) {
    matjson::Value body;

    body["addedByID"] = GJAccountManager::get()->m_accountID;
    body["token"] = DataManager::getInstance().getUserToken();
    body["addedBy"] = GJAccountManager::get()->m_username.c_str();

    body["username1"] = info1.username;
    body["username2"] = info2.username;
    body["username3"] = info3.username;

    body["accountID1"] = info1.accountID;
    body["accountID2"] = info2.accountID;
    body["accountID3"] = info3.accountID;

    body["icon1"] = info1.icon;
    body["icon2"] = info2.icon;
    body["icon3"] = info3.icon;

    body["colorFirst1"] = info1.colorFirst;
    body["colorFirst2"] = info2.colorFirst;
    body["colorFirst3"] = info3.colorFirst;

    body["colorSecond1"] = info1.colorSecond;
    body["colorSecond2"] = info2.colorSecond;
    body["colorSecond3"] = info3.colorSecond;

    body["glow1"] = info1.glow;
    body["glow2"] = info2.glow;
    body["glow3"] = info3.glow;

    body["type1"] = info1.type;
    body["type2"] = info2.type;
    body["type3"] = info3.type;

    body["title1"] = info1.title;
    body["title2"] = info2.title;
    body["title3"] = info3.title;

    body["info1"] = info1.info;
    body["info2"] = info2.info;
    body["info3"] = info3.info;

    co_return co_await request<NewWeeklyAchResponse>("POST", "/new_weekly_achievements", body);
}

template <>
struct matjson::Serialize<WeeklyAchResponse> {
    static geode::Result<WeeklyAchResponse> fromJson(matjson::Value const& json) {
        WeeklyAchResponse ret;

        ret.ok = json["ok"].asBool().unwrapOrDefault();

        std::vector<std::pair<std::string, AchievementCellInfo>> achIDs{
            {"achievement_1", AchievementCellInfo{}}, {"achievement_2", AchievementCellInfo{}}, {"achievement_3", AchievementCellInfo{}}
        };

        for (auto& id : achIDs) {
            if (!json.contains(id.first)) {
                ret.ok = false;
                return geode::Ok(ret);
            }

            auto ach = json[id.first];

            id.second.accountID = ach["accountID"].asInt().unwrapOrDefault();
            id.second.username = ach["username"].asString().unwrapOrDefault();
            id.second.icon = ach["icon"].asInt().unwrapOrDefault();
            id.second.colorFirst = ach["colorFirst"].asInt().unwrapOrDefault();
            id.second.colorSecond = ach["colorSecond"].asInt().unwrapOrDefault();
            id.second.glow = ach["glow"].asInt().unwrapOrDefault();
            id.second.title = ach["title"].asString().unwrapOrDefault();
            id.second.type = ach["type"].asInt().unwrapOrDefault();
            id.second.info = ach["info"].asString().unwrapOrDefault();
        }

        ret.cell1 = achIDs[0].second;
        ret.cell2 = achIDs[1].second;
        ret.cell3 = achIDs[2].second;

        return geode::Ok(ret);
    }
};

arc::Future<WeeklyAchResponse> BackendManager::getWeeklyAch() {
    co_return co_await request<WeeklyAchResponse>("GET", "/get_weekly_achievements");
}


}
