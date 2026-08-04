#include "../BackendManager.hpp"
#include "../DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<GetUserRolesResponse> {
    static geode::Result<GetUserRolesResponse> fromJson(matjson::Value const& json) {
        GetUserRolesResponse ret;

        ret.roles.isAdmin = json["isAdmin"].asBool().unwrapOrDefault();
        ret.roles.isOwner = json["isOwner"].asBool().unwrapOrDefault();
        ret.roles.isHelper = json["isHelper"].asBool().unwrapOrDefault();
        ret.roles.isArtist = json["isArtist"].asBool().unwrapOrDefault();
        ret.roles.isBooster = json["isBooster"].asBool().unwrapOrDefault();
        ret.roles.isContributor = json["isContributor"].asBool().unwrapOrDefault();
        ret.petExists = json["petExists"].asBool().unwrapOrDefault();
        ret.isPetBanned = json["isPetBanned"].asBool().unwrapOrDefault();

        ret.ok = true;
        return geode::Ok(ret);
    }
};

arc::Future<GetUserRolesResponse> BackendManager::getUserRoles(int accountID) {
    matjson::Value reqBody;
    reqBody["account_id"] = accountID;

    co_return co_await request<GetUserRolesResponse>("POST", "/check_helper_manage", reqBody);
}

template <>
struct matjson::Serialize<SetRolesResponse> {
    static geode::Result<SetRolesResponse> fromJson(matjson::Value const& json) {
        SetRolesResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<SetRolesResponse> BackendManager::setRoles(const SetRolesBody& body) {
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

    co_return co_await request<SetRolesResponse>("POST", "/set_roles", reqBody);
}

template <>
struct matjson::Serialize<ReqAccessResponse> {
    static geode::Result<ReqAccessResponse> fromJson(matjson::Value const& json) {
        ReqAccessResponse ret;
        ret.ok = true;
        ret.pos = json["pos"].asInt().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<ReqAccessResponse> BackendManager::requestStaffAccess() {
    matjson::Value body;
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["token"] = DataManager::getInstance().getUserToken();

    co_return co_await request<ReqAccessResponse>("POST", "/check_helper_new", body);
}


}
