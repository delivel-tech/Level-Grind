#include "../BackendManager.hpp"
#include "../DataManager.hpp"
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/async.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<BootupGetResponse> {
    static geode::Result<BootupGetResponse> fromJson(matjson::Value const& json) {
        BootupGetResponse ret;

        bool isOK = json["ok"].asBool().unwrapOrDefault();
        if (!isOK) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        auto staffs = json["staff"];

        auto owners = staffs["owners"].asArray();
        auto admins = staffs["admins"].asArray();
        auto helpers = staffs["helpers"].asArray();
        auto contributors = staffs["contributors"].asArray();
        auto artists = staffs["artists"].asArray();
        auto boosters = staffs["boosters"].asArray();

        if (!owners || !admins || !helpers || !contributors || !artists || !boosters) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        std::vector<int> ownersArr;
        std::vector<int> helpersArr;
        std::vector<int> adminsArr;
        std::vector<int> artistsArr;
        std::vector<int> boostersArr;
        std::vector<int> contributorsArr;

        for (auto const& val : owners.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            ownersArr.push_back(accountId.unwrapOrDefault());
        }

        for (auto const& val : admins.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            adminsArr.push_back(accountId.unwrapOrDefault());
        }

        for (auto const& val : helpers.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            helpersArr.push_back(accountId.unwrapOrDefault());
        }

        for (auto const& val : contributors.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            contributorsArr.push_back(accountId.unwrapOrDefault());
        }

        for (auto const& val : artists.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            artistsArr.push_back(accountId.unwrapOrDefault());
        }

        for (auto const& val : boosters.unwrap()) {
            auto accountId = val["accountId"].asInt();
            if (!accountId) continue;
            boostersArr.push_back(accountId.unwrapOrDefault());
        }

        ret.admins = adminsArr;
        ret.owners = ownersArr;
        ret.helpers = helpersArr;
        ret.artists = artistsArr;
        ret.boosters = boostersArr;
        ret.contributors = contributorsArr;

        auto levelsWithCoins = json["levelsWithCoins"].asArray();
        auto levelsWithoutCoins = json["levelsWithoutCoins"].asArray();

        if (!levelsWithCoins || !levelsWithoutCoins) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        std::unordered_set<int> lvlsWithCoinsArr;
        std::unordered_set<int> lvlsWithoutCoinsArr;

        for (auto id : levelsWithCoins.unwrap()) {
            if (auto idVal = id.asInt(); idVal) {
                lvlsWithCoinsArr.insert(idVal.unwrap());
            }
        }

        for (auto id : levelsWithoutCoins.unwrap()) {
            if (auto idVal = id.asInt(); idVal) {
                lvlsWithoutCoinsArr.insert(idVal.unwrap());
            }
        }

        ret.levelsWithCoins = lvlsWithCoinsArr;
        ret.levelsWithoutCoins = lvlsWithoutCoinsArr;

        auto notes = json["notes"].asArray();

        if (!notes) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        std::unordered_multimap<int, NoteInfo> notesMap;

        for (auto const& val : notes.unwrap()) {
            auto levelID = val["levelID"].asInt();
            auto note = val["note"].asString();
            auto addedBy = val["addedBy"].asString();

            if (!levelID || !note || !addedBy) {
                continue;
            }

            NoteInfo info;
            info.levelID = levelID.unwrap();
            info.note = note.unwrap();
            info.senderUsername = addedBy.unwrap();

            notesMap.insert({levelID.unwrap(), info});
        }

        ret.notes = notesMap;
        ret.ok = true;

        return geode::Ok(ret);
    }
};

arc::Future<BootupGetResponse> BackendManager::bootupGet() {
    co_return co_await request<BootupGetResponse>("GET", "/bootup_get");
}

template <>
struct matjson::Serialize<HealthResponse> {
    static geode::Result<HealthResponse> fromJson(matjson::Value const& json) {
        HealthResponse ret;
        ret.ok = json["status"].asString().unwrapOrDefault() == "healthy";
        return geode::Ok(ret);
    }
};

arc::Future<HealthResponse> BackendManager::health() {
    co_return co_await request<HealthResponse>("GET", "/health");
}

template <>
struct matjson::Serialize<GetCreditsResponse> {
    static geode::Result<GetCreditsResponse> fromJson(matjson::Value const& json) {
        GetCreditsResponse ret;

        ret.ok = json["success"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto owners = json["owners"].asArray();
        auto admins = json["admins"].asArray();
        auto helpers = json["helpers"].asArray();
        auto artists = json["artists"].asArray();
        auto contributors = json["contributors"].asArray();
        auto boosters = json["boosters"].asArray();

        if (!owners || !admins || !helpers || !artists || !contributors || !boosters) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        auto parseUsers = [](auto const& arr) {
            std::vector<CreditUserInfo> users;
            for (auto& userRes : arr) {
                CreditUserInfo user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();
                users.push_back(user);
            }
            return users;
        };

        ret.owners = parseUsers(owners.unwrap());
        ret.admins = parseUsers(admins.unwrap());
        ret.helpers = parseUsers(helpers.unwrap());
        ret.artists = parseUsers(artists.unwrap());
        ret.contributors = parseUsers(contributors.unwrap());
        ret.boosters = parseUsers(boosters.unwrap());

        return geode::Ok(ret);
    }
};

arc::Future<GetCreditsResponse> BackendManager::getCredits() {
    co_return co_await request<GetCreditsResponse>("GET", "/get_credits");
}

void BackendManager::performGetToken() {
    geode::queueInMainThread([] {
        auto accountData = argon::getGameAccountData();

        async::spawn([data = std::move(accountData)] -> arc::Future<> {
                auto res = co_await argon::startAuth(data);

                if (!res.ok()) {
                    geode::queueInMainThread([] {
                        Notification::create("[Level Grind] Failed to get Argon token!")->show();
                    });
                    co_return;
                }

                auto token = std::move(res).unwrap();
                Mod::get()->setSavedValue("argon_token", token);
                DataManager::getInstance().setUserToken(token);

                co_return;
            }
        );
    });
}


}
