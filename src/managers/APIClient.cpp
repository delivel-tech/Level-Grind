#include "APIClient.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <matjson.hpp>
#include <string>
#include <unordered_set>

#include <argon/argon.hpp>
#include <vector>

#include "DataManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

GetLevelsBody APIClient::makeGetLevelsBody(
    std::vector<int> difficulties,
    std::vector<int> lengths,
    std::vector<int> demonDifficulties,
    std::vector<std::string> grindTypes,
    std::vector<int> versions,
    bool isNewerFirst,
    bool isRecentlyAdded
) {
    GetLevelsBody ret;

    ret.difficulties = difficulties;
    ret.lengths = lengths;
    ret.demonDifficulties = demonDifficulties;
    ret.grindTypes = grindTypes;
    ret.versions = versions;
    ret.isNewerFirst = isNewerFirst;
    ret.isRecentlyAdded = isRecentlyAdded;

    return ret;
}

web::WebFuture APIClient::getLevels(levelgrind::GetLevelsBody& body) {
    auto req = web::WebRequest();

    matjson::Value reqBody;

    if (!body.difficulties.empty()) reqBody["difficulties"] = body.difficulties;
    if (!body.lengths.empty()) reqBody["lengths"] = body.lengths;
    if (!body.demonDifficulties.empty()) reqBody["demonDifficulties"] = body.demonDifficulties;
    if (!body.grindTypes.empty()) reqBody["grindTypes"] = body.grindTypes;
    if (!body.versions.empty()) reqBody["versions"] = body.versions;
    reqBody["newerFirst"] = body.isNewerFirst;
    reqBody["recentlyAdded"] = body.isRecentlyAdded;

    req.bodyJSON(reqBody);

    return req.post(fmt::format("{}{}", baseUrl, "/get_levels"));
}

GetLevelsResponse APIClient::getLevelsParse(web::WebResponse res) {
    GetLevelsResponse ret;

    if (!res.ok()) {
        ret.ok = false;
        log::error("bad web req, code: {}", res.code());
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        ret.ok = false;
        log::error("bad web req, code: {}", res.code());
        return ret;
    }

    auto json = jsonRes.unwrap();

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

    return ret;
}

web::WebFuture APIClient::bootupGet() {
    auto req = web::WebRequest();
    return req.get(fmt::format("{}{}", baseUrl, "/bootup_get"));
}

BootupGetResponse APIClient::bootupGetParse(web::WebResponse res) {
    BootupGetResponse ret;

    if (!res.ok()) {
        ret.ok = false;
        log::error("bad web req, code: {}", res.code());
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();

    bool isOK = json["ok"].asBool().unwrapOrDefault();
    if (!isOK) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto staffs = json["staff"];

    auto owners = staffs["owners"].asArray();
    auto admins = staffs["admins"].asArray();
    auto helpers = staffs["helpers"].asArray();
    auto contributors = staffs["contributors"].asArray();
    auto artists = staffs["artists"].asArray();
    auto boosters = staffs["boosters"].asArray();

    if (!owners || !admins || !helpers || !contributors || !artists || !boosters) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
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
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
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
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    std::unordered_map<int, std::string> notesMap;

    for (auto const& val : notes.unwrap()) {
        auto levelID = val["levelID"].asInt();
        auto note = val["note"].asString();

        if (!levelID || !note) {
            continue;
        }

        notesMap[levelID.unwrap()] = note.unwrap();
    }

    ret.notes = notesMap;
    ret.ok = true;

    return ret;
}

web::WebFuture APIClient::getAnnouncements() {
    return web::WebRequest().get(fmt::format("{}{}", baseUrl, "/get_announcements"));
}

AnnouncementsResponse APIClient::getAnnouncementsParse(web::WebResponse res) {
    AnnouncementsResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();

    bool isOK = json["ok"].asBool().unwrapOrDefault();
    
    if (!isOK) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto ann = json["announcements"].asArray();

    if (!ann) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto announcements = ann.unwrap();

    for (auto const& val : announcements) {
        AnnouncementInfo currentAnn;

        currentAnn.content = val["content"].asString().unwrapOrDefault();
        currentAnn.id = val["id"].asInt().unwrapOrDefault();
        currentAnn.title = val["title"].asString().unwrapOrDefault();
        currentAnn.addedBy = val["added_by"].asString().unwrapOrDefault();
        currentAnn.createdAt = val["created_at"].asString().unwrapOrDefault();

        ret.announcements.push_back(currentAnn);
    }

    ret.ok = true;

    return ret;
}

void APIClient::performBootupGet() {
    async::spawn(
        bootupGet(),
        [this](web::WebResponse res) {
            auto parsed = bootupGetParse(res);
            DataManager::getInstance().setSharedData(parsed);
        }
    );
}

web::WebFuture APIClient::addAnnouncement(std::string title, std::string content) {
    auto req = web::WebRequest();
    matjson::Value reqBody;

    reqBody["account_id"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["title"] = title;
    reqBody["content"] = content;
    reqBody["added_by"] = std::string(GJAccountManager::sharedState()->m_username);

    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/new_announcement"));
}

web::WebFuture APIClient::getLevelInfo(int levelID) {
    web::WebRequest req = web::WebRequest();
    matjson::Value reqBody;

    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["levelId"] = levelID;

    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/get_level_info_staff"));
}

GetLevelInfoResponse APIClient::getLevelInfoParse(web::WebResponse res) {
    GetLevelInfoResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();

    #define GET_JSON_BOOL(x) json[x].asBool().unwrapOrDefault()

    ret.ok = GET_JSON_BOOL("ok");

    if (!ret.ok) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
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

    #undef GET_JSON_BOOL

    return ret;
}

AddAnnouncementResponse APIClient::addAnnouncementParse(web::WebResponse res) {
    AddAnnouncementResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::deleteAnnouncement(int announcementId) {
    auto req = web::WebRequest();
    matjson::Value reqBody;

    reqBody["account_id"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["announcement_id"] = announcementId;

    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/delete_announcement"));
}

DeleteAnnouncementResponse APIClient::deleteAnnouncementParse(web::WebResponse res) {
    DeleteAnnouncementResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::getUserRoles(int accountID) {
    auto req = web::WebRequest();
    matjson::Value reqBody;
    reqBody["account_id"] = accountID;
    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/check_helper_manage"));
}

GetUserRolesResponse APIClient::getUserRolesParse(web::WebResponse res) {
    GetUserRolesResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();

    ret.roles.isAdmin = json["isAdmin"].asBool().unwrapOrDefault();
    ret.roles.isOwner = json["isOwner"].asBool().unwrapOrDefault();
    ret.roles.isHelper = json["isHelper"].asBool().unwrapOrDefault();
    ret.roles.isArtist = json["isArtist"].asBool().unwrapOrDefault();
    ret.roles.isBooster = json["isBooster"].asBool().unwrapOrDefault();
    ret.roles.isContributor = json["isContributor"].asBool().unwrapOrDefault();
    ret.petExists = json["petExists"].asBool().unwrapOrDefault();
    ret.isPetBanned = json["isPetBanned"].asBool().unwrapOrDefault();

    ret.ok = true;
    return ret;
}

SetRolesBody APIClient::makeSetRolesBody(
    int accountID,
    std::string token,
    int targetAccountID,
    std::string targetUsername,
    int targetIcon,
    int targetColor1,
    int targetColor2,
    int targetColor3,
    bool isAdmin,
    bool isHelper,
    bool isArtist,
    bool isContributor,
    bool isBooster
) {
    SetRolesBody ret;

    ret.accountID = accountID;
    ret.token = token;
    ret.targetAccountID = targetAccountID;
    ret.targetUsername = targetUsername;
    ret.targetIcon = targetIcon;
    ret.targetColor1 = targetColor1;
    ret.targetColor2 = targetColor2;
    ret.targetColor3 = targetColor3;
    ret.isAdmin = isAdmin;
    ret.isHelper = isHelper;
    ret.isArtist = isArtist;
    ret.isContributor = isContributor;
    ret.isBooster = isBooster;

    return ret;
}

web::WebFuture APIClient::setRoles(const SetRolesBody& body) {
    auto req = web::WebRequest();
    matjson::Value reqBody;

    reqBody["account_id"] = body.accountID;
    reqBody["token"] = body.token;
    reqBody["target_account_id"] = body.targetAccountID;
    reqBody["targetUsername"] = body.targetUsername;
    reqBody["targetIcon"] = body.targetIcon;
    reqBody["targetColor1"] = body.targetColor1;
    reqBody["targetColor2"] = body.targetColor2;
    reqBody["targetColor3"] = body.targetColor3;
    reqBody["is_admin"] = body.isAdmin;
    reqBody["is_helper"] = body.isHelper;
    reqBody["is_artist"] = body.isArtist;
    reqBody["is_contributor"] = body.isContributor;
    reqBody["is_booster"] = body.isBooster;

    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/set_roles"));
}

SetRolesResponse APIClient::setRolesParse(web::WebResponse res) {
    SetRolesResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::wipePet(int accountID) {
    auto req = web::WebRequest();
    matjson::Value reqBody;
    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountID"] = accountID;
    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/wipe_pet_data"));
}

WipePetResponse APIClient::wipePetParse(web::WebResponse res) {
    WipePetResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::banPet(int accountID, std::string reason) {
    auto req = web::WebRequest();
    matjson::Value reqBody;
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountId"] = accountID;
    reqBody["banReason"] = reason;
    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/ban_pet"));
}

BanPetResponse APIClient::banPetParse(web::WebResponse res) {
    BanPetResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::unbanPet(int accountID) {
    auto req = web::WebRequest();
    matjson::Value reqBody;
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountId"] = accountID;
    req.bodyJSON(reqBody);
    return req.post(fmt::format("{}{}", baseUrl, "/unban_pet"));
}

UnbanPetResponse APIClient::unbanPetParse(web::WebResponse res) {
    UnbanPetResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = json["ok"].asBool().unwrapOrDefault();
    return ret;
}

web::WebFuture APIClient::requestStaffAccess() {
    matjson::Value body;
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["token"] = DataManager::getInstance().getUserToken();

    web::WebRequest req;
    req.bodyJSON(body);

    return req.post(fmt::format("{}{}", baseUrl, "/check_helper_new"));
}

ReqAccessResponse APIClient::requestStaffAccessParse(web::WebResponse res) {
    ReqAccessResponse ret;

    if (!res.ok()) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto jsonRes = res.json();
    if (!jsonRes) {
        log::error("bad web req, code: {}", res.code());
        ret.ok = false;
        return ret;
    }

    auto json = jsonRes.unwrap();
    ret.ok = true;
    ret.pos = json["pos"].asInt().unwrapOrDefault();
    return ret;
}

void APIClient::performGetToken() {
    async::spawn(
        argon::startAuth(),
        [](Result<std::string> res) {
            if (!res.ok()) return;
			auto token = std::move(res).unwrap();
			Mod::get()->setSavedValue("argon_token", token);
            DataManager::getInstance().setUserToken(token);
        }
    );
}

}