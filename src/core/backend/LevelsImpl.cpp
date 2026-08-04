#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<GetLevelsResponse> {
    static geode::Result<GetLevelsResponse> fromJson(matjson::Value const& json) {
        GetLevelsResponse ret;

        int totalCount = 0;
        if (json.contains("count")) {
            if (auto count = json["count"].asInt(); count) {
                totalCount = count.unwrap();
            }
        }

        std::vector<int> allIDs;
        if (json.contains("ids")) {
            auto arrRes = json["ids"].asArray();
            if (arrRes) {
                for (auto id : arrRes.unwrap()) {
                    if (auto idVal = id.asInt(); idVal) {
                        allIDs.push_back(idVal.unwrap());
                    }
                }
            }
        }

        ret.count = totalCount;
        ret.ids = allIDs;
        ret.ok = true;

        return geode::Ok(ret);
    }
};

arc::Future<GetLevelsResponse> BackendManager::getLevels(GetLevelsBody& body) {
    matjson::Value reqBody;

    if (!body.difficulties.empty()) reqBody["difficulties"] = body.difficulties;
    if (!body.lengths.empty()) reqBody["lengths"] = body.lengths;
    if (!body.demonDifficulties.empty()) reqBody["demonDifficulties"] = body.demonDifficulties;
    if (!body.grindTypes.empty()) reqBody["grindTypes"] = body.grindTypes;
    if (!body.versions.empty()) reqBody["versions"] = body.versions;
    reqBody["newerFirst"] = body.isNewerFirst;
    reqBody["recentlyAdded"] = body.isRecentlyAdded;

    co_return co_await request<GetLevelsResponse>("POST", "/get_levels", reqBody);
}

template <>
struct matjson::Serialize<NewLevelResponse> {
    static geode::Result<NewLevelResponse> fromJson(matjson::Value const& json) {
        NewLevelResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<NewLevelResponse> BackendManager::newlevel(ManageLevelBody body) {
    matjson::Value reqBody;

    reqBody["id"] = body.id;
    reqBody["name"] = body.name;
    reqBody["difficulty"] = body.difficulty;
    reqBody["length"] = body.length;

    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["account_id"] = GJAccountManager::get()->m_accountID;

    if (body.difficulty == 10) {
        reqBody["demon_difficulty"] = body.demonDifficulty;
    }

    if (body.star) reqBody["star"] = body.star;
    if (body.moon) reqBody["moon"] = body.moon;
    if (body.coin && body.coin >= 0) reqBody["coin"] = body.coin;
    if (body.demon) reqBody["demon"] = body.demon;

    reqBody["added_by"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<NewLevelResponse>("POST", "/new_level", reqBody);
}

template <>
struct matjson::Serialize<DeleteLevelResponse> {
    static geode::Result<DeleteLevelResponse> fromJson(matjson::Value const& json) {
        DeleteLevelResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<DeleteLevelResponse> BackendManager::deleteLevel(int levelId) {
    matjson::Value body;

    body["token"] = DataManager::getInstance().getUserToken();
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["id"] = levelId;

    co_return co_await request<DeleteLevelResponse>("POST", "/delete_level", body);
}

template <>
struct matjson::Serialize<LockLevelResponse> {
    static geode::Result<LockLevelResponse> fromJson(matjson::Value const& json) {
        LockLevelResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<LockLevelResponse> BackendManager::lockLevel(int levelId, std::string levelName) {
    matjson::Value body;

    body["token"] = DataManager::getInstance().getUserToken();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["levelId"] = levelId;
    body["levelName"] = levelName;
    body["bannedBy"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<LockLevelResponse>("POST", "/ban_level", body);
}

template <>
struct matjson::Serialize<UnlockLevelResponse> {
    static geode::Result<UnlockLevelResponse> fromJson(matjson::Value const& json) {
        UnlockLevelResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<UnlockLevelResponse> BackendManager::unlockLevel(int levelId, std::string levelName) {
    matjson::Value body;

    body["token"] = DataManager::getInstance().getUserToken();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["levelId"] = levelId;
    body["levelName"] = levelName;
    body["unbannedBy"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<UnlockLevelResponse>("POST", "/unban_level", body);
}

template <>
struct matjson::Serialize<DeleteNotesResponse> {
    static geode::Result<DeleteNotesResponse> fromJson(matjson::Value const& json) {
        DeleteNotesResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<DeleteNotesResponse> BackendManager::deleteNotes(int levelId, std::string levelName) {
    matjson::Value reqBody;

    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["levelID"] = levelId;
    reqBody["levelName"] = levelName;
    reqBody["deletedBy"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<DeleteNotesResponse>("POST", "/delete_note", reqBody);
}

template <>
struct matjson::Serialize<AddNoteResponse> {
    static geode::Result<AddNoteResponse> fromJson(matjson::Value const& json) {
        AddNoteResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<AddNoteResponse> BackendManager::addNote(int levelId, std::string levelName, std::string note) {
    matjson::Value reqBody;

    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["levelID"] = levelId;
    reqBody["levelName"] = levelName;
    reqBody["note"] = note;
    reqBody["addedBy"] = GJAccountManager::sharedState()->m_username.c_str();

    co_return co_await request<AddNoteResponse>("POST", "/new_note", reqBody);
}

template <>
struct matjson::Serialize<GetSuggestionsResponse> {
    static geode::Result<GetSuggestionsResponse> fromJson(matjson::Value const& json) {
        GetSuggestionsResponse ret;

        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto suggestions = json["suggestions"].asArray();
        if (!suggestions) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        for (auto const& val : suggestions.unwrap()) {
            LevelSuggestion suggestion;
            suggestion.id = val["levelID"].asInt().unwrapOrDefault();
            suggestion.points = val["points"].asInt().unwrapOrDefault();
            ret.suggestions.push_back(suggestion);
        }

        return geode::Ok(ret);
    }
};

arc::Future<GetSuggestionsResponse> BackendManager::getSuggestions(int mode) {
    co_return co_await request<GetSuggestionsResponse>("GET", fmt::format("/get_suggestions?mode={}", mode));
}

template <>
struct matjson::Serialize<GetGrindPacksResponse> {
    static geode::Result<GetGrindPacksResponse> fromJson(matjson::Value const& json) {
        GetGrindPacksResponse ret;

        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto packs = json["grindPacks"].asArray();
        if (!packs) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        std::vector<GrindPack> arr;

        for (const auto& val : packs.unwrap()) {
            GrindPack pack;

            pack.id = val["id"].asInt().unwrapOrDefault();
            pack.title = val["title"].asString().unwrapOrDefault();
            pack.difficulty = static_cast<CustomDifficultyEnum>(val["difficulty"].asInt().unwrapOrDefault());
            pack.levels = {
                val["levelId1"].as<int>().unwrapOrDefault(),
                val["levelId2"].as<int>().unwrapOrDefault(),
                val["levelId3"].as<int>().unwrapOrDefault()
            };
            pack.color = {
                static_cast<GLubyte>(val["color1"].as<int>().unwrapOrDefault()),
                static_cast<GLubyte>(val["color2"].as<int>().unwrapOrDefault()),
                static_cast<GLubyte>(val["color3"].as<int>().unwrapOrDefault()),
            };
            pack.isStar = val["isStar"].asBool().unwrapOrDefault();

            arr.push_back(pack);
        }

        ret.packs = arr;

        return geode::Ok(ret);
    }
};

arc::Future<GetGrindPacksResponse> BackendManager::getGrindPacks() {
    co_return co_await request<GetGrindPacksResponse>("GET", "/get_grind_packs");
}

template <>
struct matjson::Serialize<NewGrindPackResponse> {
    static geode::Result<NewGrindPackResponse> fromJson(matjson::Value const& json) {
        NewGrindPackResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<NewGrindPackResponse> BackendManager::newGrindPack(NewGrindPackBody body) {
    matjson::Value reqBody;

    reqBody["accountID"] = GJAccountManager::get()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["title"] = body.title;
    reqBody["isStar"] = body.star;
    reqBody["levelId1"] = body.levelId1;
    reqBody["levelId2"] = body.levelId2;
    reqBody["levelId3"] = body.levelId3;
    reqBody["color1"] = body.color1;
    reqBody["color2"] = body.color2;
    reqBody["color3"] = body.color3;
    reqBody["difficulty"] = body.difficulty;
    reqBody["username"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<NewGrindPackResponse>("POST", "/new_grind_pack", reqBody);
}

template <>
struct matjson::Serialize<DeleteGrindPackResponse> {
    static geode::Result<DeleteGrindPackResponse> fromJson(matjson::Value const& json) {
        DeleteGrindPackResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<DeleteGrindPackResponse> BackendManager::deleteGrindPack(int ID) {
    matjson::Value body;

    body["accountID"] = GJAccountManager::get()->m_accountID;
    body["token"] = DataManager::getInstance().getUserToken();
    body["id"] = ID;

    co_return co_await request<DeleteGrindPackResponse>("POST", "/delete_grind_pack", body);
}

template <>
struct matjson::Serialize<GetLevelInfoResponse> {
    static geode::Result<GetLevelInfoResponse> fromJson(matjson::Value const& json) {
        GetLevelInfoResponse ret;

        #define GET_JSON_BOOL(x) json[x].asBool().unwrapOrDefault()

        ret.ok = GET_JSON_BOOL("ok");

        if (!ret.ok) {
            return geode::Ok(ret);
        }

        ret.isAdded = GET_JSON_BOOL("isAdded");
        ret.isLocked = GET_JSON_BOOL("isLocked");
        ret.star = GET_JSON_BOOL("star");
        ret.moon = GET_JSON_BOOL("moon");
        ret.coin = GET_JSON_BOOL("coin");
        ret.demon = GET_JSON_BOOL("demon");
        ret.addedBy = json["added_by"].asString().unwrapOrDefault();
        ret.noteExists = GET_JSON_BOOL("noteExists");
        ret.note = json["noteContent"].asString().unwrapOrDefault();
        ret.isDaily = GET_JSON_BOOL("isDaily");
        ret.isDailyPlat = GET_JSON_BOOL("isDailyPlat");
        ret.isWeekly = GET_JSON_BOOL("isWeekly");
        ret.isWeeklyPlat = GET_JSON_BOOL("isWeeklyPlat");
        ret.demon = GET_JSON_BOOL("hasPoints");

        #undef GET_JSON_BOOL

        ret.points = json["points"].asInt().unwrapOrDefault();
        ret.coinPoints = json["coinPoints"].asInt().unwrapOrDefault();

        ret.pointsInfo = [&] -> std::vector<PointInfo> {
            std::vector<PointInfo> toReturn;
            auto pointsInfo = json["pointsInfo"].asArray();
            if (!pointsInfo) {
                ret.ok = false;
                return toReturn;
            }

            for (const auto& val : pointsInfo.unwrap()) {
                PointInfo info;
                info.point = val["point"].asInt().unwrapOrDefault();
                info.coinPoint = val["coinPoint"].asInt().unwrapOrDefault();
                info.staffId = val["staffId"].asInt().unwrapOrDefault();
                info.staffUsername = val["staffUsername"].asString().unwrapOrDefault();
                toReturn.push_back(info);
            }

            return toReturn;
        }();

        return geode::Ok(ret);
    }
};

arc::Future<GetLevelInfoResponse> BackendManager::getLevelInfo(int levelID) {
    matjson::Value reqBody;

    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["levelId"] = levelID;

    co_return co_await request<GetLevelInfoResponse>("POST", "/get_level_info_staff", reqBody);
}

template <>
struct matjson::Serialize<ChangePointResponse> {
    static geode::Result<ChangePointResponse> fromJson(matjson::Value const& json) {
        ChangePointResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<ChangePointResponse> BackendManager::changePoint(PointType type, int coinType, ManageLevelBody levelBody) {
    matjson::Value reqBody;

    reqBody["point"] = static_cast<int>(type);
    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["username"] = GJAccountManager::sharedState()->m_username.c_str();
    reqBody["levelID"] = levelBody.id;
    reqBody["levelName"] = levelBody.name;
    reqBody["coinPoint"] = coinType;
    reqBody["length"] = levelBody.length;
    reqBody["difficulty"] = levelBody.difficulty;
    reqBody["demonDifficulty"] = levelBody.demonDifficulty;
    reqBody["star"] = levelBody.star;
    reqBody["moon"] = levelBody.moon;
    reqBody["demon"] = levelBody.demon;

    co_return co_await request<ChangePointResponse>("POST", "/change_point", reqBody);
}

template <>
struct matjson::Serialize<SyncLevelsResponse> {
    static geode::Result<SyncLevelsResponse> fromJson(matjson::Value const& json) {
        SyncLevelsResponse ret;

        ret.ok = json["ok"].asBool().unwrapOrDefault();
        ret.deleted = json["deleted"].asInt().unwrapOrDefault();
        ret.inserted = json["inserted"].asInt().unwrapOrDefault();
        ret.coinUpdates = json["coinUpdates"].asInt().unwrapOrDefault();

        return geode::Ok(ret);
    }
};

arc::Future<SyncLevelsResponse> BackendManager::syncLevels(int addThreshold, int deleteThreshold, int coinAddThreshold, int coinDeleteThreshold) {
    matjson::Value reqBody;

    reqBody["accountID"] = GJAccountManager::get()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["addThreshold"] = addThreshold;
    reqBody["deleteThreshold"] = deleteThreshold;
    reqBody["coinAddThreshold"] = coinAddThreshold;
    reqBody["coinDeleteThreshold"] = coinDeleteThreshold;

    co_return co_await request<SyncLevelsResponse>("POST", "/sync_levels", reqBody, std::chrono::seconds{30});
}

template <>
struct matjson::Serialize<Indicators> {
    static geode::Result<Indicators> fromJson(matjson::Value const& json) {
        Indicators indicators;

        auto indicatorsJson = json["indicators"];

        indicators.ok = json["ok"].asBool().unwrapOrDefault();

        indicators.added = indicatorsJson["added"].asBool().unwrapOrDefault();
        indicators.coin = indicatorsJson["coin"].asBool().unwrapOrDefault();
        indicators.event = indicatorsJson["event"].asBool().unwrapOrDefault();
        indicators.pack = indicatorsJson["pack"].asBool().unwrapOrDefault();

        return geode::Ok(indicators);
    }
};

arc::Future<Indicators> BackendManager::getIndicators(int levelID) {
    co_return co_await request<Indicators>("GET", fmt::format("/get_indicators?id={}", levelID));
}

template <>
struct matjson::Serialize<CancelVoteResponse> {
    static geode::Result<CancelVoteResponse> fromJson(matjson::Value const& json) {
        CancelVoteResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<CancelVoteResponse> BackendManager::cancelVote(int levelID) {
    matjson::Value reqBody;

    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["levelID"] = levelID;

    co_return co_await request<CancelVoteResponse>("POST", "/cancel_vote", reqBody);
}


}
