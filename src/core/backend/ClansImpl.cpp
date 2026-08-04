#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/SecurityManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

namespace {

geode::utils::web::WebRequest buildEncryptedClanRequest(matjson::Value const& body, int accountId) {
    geode::utils::web::WebRequest req;

    matjson::Value wrapper;
    wrapper["accountId"] = accountId;
    wrapper["payload"] = SecurityManager::getInstance().buildEncryptedPayload(body, accountId);

    req.bodyJSON(wrapper);
    return req;
}

LGClanData parseLGClanData(matjson::Value const& val) {
    LGClanData c;
    c.m_clanID = val["clanID"].asInt().unwrapOrDefault();
    c.m_ownerAccountID = val["ownerAccountID"].asInt().unwrapOrDefault();
    c.m_clanName = val["clanName"].asString().unwrapOrDefault();
    c.m_clanDescription = val["clanDescription"].asString().unwrapOrDefault();
    c.m_clanType = static_cast<LGClanType>(val["clanType"].asInt().unwrapOrDefault());
    c.m_clanRating = static_cast<LGClanRating>(val["clanRating"].asInt().unwrapOrDefault());

    auto visuals = val["clanVisuals"];
    c.m_clanVisuals.m_colorID = visuals["colorID"].asInt().unwrapOrDefault();

    auto progression = val["clanProgression"];
    c.m_clanProgression.m_clanLevel = progression["clanLevel"].asInt().unwrapOrDefault();
    c.m_clanProgression.m_clanStats = progression["clanStats"].asInt().unwrapOrDefault();

    c.m_maxMembers = val["maxMembers"].asInt().unwrapOrDefault();
    c.m_membersAmount = val["membersAmount"].asInt().unwrapOrDefault();
    c.m_clanJoinType = static_cast<LGClanJoinType>(val["clanJoinType"].asInt().unwrapOrDefault());
    c.m_statRequirementForRequest = val["statRequirementForRequest"].asInt().unwrapOrDefault();

    auto parseGoal = [](matjson::Value const& g) {
        LGClanStatGoal goal;
        goal.m_statGoal = g["statGoal"].asInt().unwrapOrDefault();
        goal.m_statGoalCompleted = g["statGoalCompleted"].asBool().unwrapOrDefault();
        return goal;
    };
    c.m_dailyStatGoal = parseGoal(val["dailyStatGoal"]);
    c.m_weeklyStatGoal = parseGoal(val["weeklyStatGoal"]);
    c.m_monthlyStatGoal = parseGoal(val["monthlyStatGoal"]);

    return c;
}

LGClanMember parseLGClanMember(matjson::Value const& val) {
    LGClanMember m;
    m.m_accountID = val["accountID"].asInt().unwrapOrDefault();
    m.m_username = val["username"].asString().unwrapOrDefault();
    m.m_role = static_cast<LGClanRole>(val["role"].asInt().unwrapOrDefault());
    m.m_stats = val["stats"].asInt().unwrapOrDefault();
    return m;
}

}

template <>
struct matjson::Serialize<ViewClansResponse> {
    static geode::Result<ViewClansResponse> fromJson(matjson::Value const& json) {
        ViewClansResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        ret.petStars = json["petStars"].asInt().unwrapOrDefault();
        ret.petMoons = json["petMoons"].asInt().unwrapOrDefault();
        ret.myClanID = json["myClanID"].asInt().unwrapOrDefault();

        auto clansArr = json["clans"].asArray();
        if (clansArr) {
            for (auto const& val : clansArr.unwrap()) {
                ret.clans.push_back(parseLGClanData(val));
            }
        }

        return geode::Ok(ret);
    }
};

arc::Future<ViewClansResponse> BackendManager::viewClans(bool shouldUpdateStars, int starsDelta, bool shouldUpdateMoons, int moonsDelta, int page, int limit) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["shouldUpdateStars"] = shouldUpdateStars;
    body["starsDelta"] = starsDelta;
    body["shouldUpdateMoons"] = shouldUpdateMoons;
    body["moonsDelta"] = moonsDelta;
    body["page"] = page;
    body["limit"] = limit;

    co_return co_await requestWithReq<ViewClansResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/view_clans");
}

template <>
struct matjson::Serialize<GetClanResponse> {
    static geode::Result<GetClanResponse> fromJson(matjson::Value const& json) {
        GetClanResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto clanJson = json["clan"];
        ret.clan.m_data = parseLGClanData(clanJson["data"]);

        auto membersArr = clanJson["members"].asArray();
        if (membersArr) {
            for (auto const& val : membersArr.unwrap()) {
                ret.clan.m_members.push_back(parseLGClanMember(val));
            }
        }

        if (clanJson.contains("joinRequests")) {
            ret.canManage = true;

            auto reqArr = clanJson["joinRequests"].asArray();
            if (reqArr) {
                for (auto const& val : reqArr.unwrap()) {
                    ClanJoinRequestInfo info;
                    info.m_accountID = val["accountID"].asInt().unwrapOrDefault();
                    info.m_username = val["username"].asString().unwrapOrDefault();
                    ret.joinRequests.push_back(info);
                }
            }

            auto invArr = clanJson["invites"].asArray();
            if (invArr) {
                for (auto const& val : invArr.unwrap()) {
                    ClanInviteInfo info;
                    info.m_accountID = val["accountID"].asInt().unwrapOrDefault();
                    info.m_username = val["username"].asString().unwrapOrDefault();
                    info.m_invitedBy = val["invitedBy"].asInt().unwrapOrDefault();
                    ret.invites.push_back(info);
                }
            }
        }

        return geode::Ok(ret);
    }
};

arc::Future<GetClanResponse> BackendManager::getClan(int clanID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;

    co_return co_await requestWithReq<GetClanResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/get_clan");
}

template <>
struct matjson::Serialize<ClanDataResponse> {
    static geode::Result<ClanDataResponse> fromJson(matjson::Value const& json) {
        ClanDataResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) {
            ret.error = json["error"].asString().unwrapOrDefault();
            return geode::Ok(ret);
        }
        ret.clan = parseLGClanData(json["clan"]);
        return geode::Ok(ret);
    }
};

arc::Future<ClanDataResponse> BackendManager::createClan(std::string name, std::string description, LGClanType type, int colorID, LGClanJoinType joinType, int statRequirement) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanName"] = name;
    body["clanDescription"] = description;
    body["clanType"] = static_cast<int>(type);
    body["colorID"] = colorID;
    body["clanJoinType"] = static_cast<int>(joinType);
    body["statRequirementForRequest"] = statRequirement;

    co_return co_await requestWithReq<ClanDataResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/create_clan");
}

arc::Future<ClanDataResponse> BackendManager::editClan(int clanID, std::string name, std::string description, int colorID, LGClanJoinType joinType, int statRequirement) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["clanName"] = name;
    body["clanDescription"] = description;
    body["colorID"] = colorID;
    body["clanJoinType"] = static_cast<int>(joinType);
    body["statRequirementForRequest"] = statRequirement;

    co_return co_await requestWithReq<ClanDataResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/edit_clan");
}

template <>
struct matjson::Serialize<ClanActionResponse> {
    static geode::Result<ClanActionResponse> fromJson(matjson::Value const& json) {
        ClanActionResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) {
            ret.error = json["error"].asString().unwrapOrDefault();
        }
        if (json.contains("clanDisbanded")) {
            ret.disbanded = json["clanDisbanded"].asBool().unwrapOrDefault();
        }

        return geode::Ok(ret);
    }
};

arc::Future<ClanActionResponse> BackendManager::joinClan(int clanID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/join_clan");
}

arc::Future<ClanActionResponse> BackendManager::requestJoinClan(int clanID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/request_join_clan");
}

arc::Future<ClanActionResponse> BackendManager::respondJoinRequest(int clanID, int targetAccountID, bool accept) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["targetAccountId"] = targetAccountID;
    body["accept"] = accept;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/respond_join_request");
}

arc::Future<ClanActionResponse> BackendManager::inviteToClan(int clanID, int targetAccountID, std::string targetUsername) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["targetAccountId"] = targetAccountID;
    body["targetUsername"] = targetUsername;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/invite_to_clan");
}

template <>
struct matjson::Serialize<GetMyClanInvitesResponse> {
    static geode::Result<GetMyClanInvitesResponse> fromJson(matjson::Value const& json) {
        GetMyClanInvitesResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto arr = json["invites"].asArray();
        if (arr) {
            for (auto const& val : arr.unwrap()) {
                MyClanInviteInfo info;
                info.m_clanID = val["clanID"].asInt().unwrapOrDefault();
                info.m_clanName = val["clanName"].asString().unwrapOrDefault();
                info.m_invitedBy = val["invitedBy"].asInt().unwrapOrDefault();
                ret.invites.push_back(info);
            }
        }

        return geode::Ok(ret);
    }
};

arc::Future<GetMyClanInvitesResponse> BackendManager::getMyClanInvites() {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();

    co_return co_await requestWithReq<GetMyClanInvitesResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/get_my_clan_invites");
}

arc::Future<ClanActionResponse> BackendManager::respondClanInvite(int clanID, bool accept) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["accept"] = accept;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/respond_clan_invite");
}

arc::Future<ClanActionResponse> BackendManager::setClanRating(int clanID, LGClanRating rating) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["rating"] = static_cast<int>(rating);

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/set_clan_rating");
}

arc::Future<ClanActionResponse> BackendManager::leaveClan() {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/leave_clan");
}

arc::Future<ClanActionResponse> BackendManager::transferLeadership(int clanID, int targetAccountID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["targetAccountId"] = targetAccountID;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/transfer_leadership");
}

arc::Future<ClanActionResponse> BackendManager::kickClanMember(int clanID, int targetAccountID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["targetAccountId"] = targetAccountID;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/kick_clan_member");
}

arc::Future<ClanActionResponse> BackendManager::setClanMemberRole(int clanID, int targetAccountID, LGClanRole role) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["targetAccountId"] = targetAccountID;
    body["role"] = static_cast<int>(role);

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/set_clan_member_role");
}

arc::Future<ClanActionResponse> BackendManager::disbandClan(int clanID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/disband_clan");
}

arc::Future<ClanActionResponse> BackendManager::sendClanMessage(int clanID, std::string message) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["message"] = message;

    co_return co_await requestWithReq<ClanActionResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/send_clan_message");
}

template <>
struct matjson::Serialize<GetClanMessagesResponse> {
    static geode::Result<GetClanMessagesResponse> fromJson(matjson::Value const& json) {
        GetClanMessagesResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto arr = json["messages"].asArray();
        if (arr) {
            for (auto const& val : arr.unwrap()) {
                ClanMessageInfo m;
                m.m_accountID = val["accountID"].asInt().unwrapOrDefault();
                m.m_username = val["username"].asString().unwrapOrDefault();
                m.m_message = val["message"].asString().unwrapOrDefault();
                m.m_createdAt = val["createdAt"].asString().unwrapOrDefault();
                ret.messages.push_back(m);
            }
        }

        return geode::Ok(ret);
    }
};

arc::Future<GetClanMessagesResponse> BackendManager::getClanMessages(int clanID) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;

    co_return co_await requestWithReq<GetClanMessagesResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/get_clan_messages");
}

template <>
struct matjson::Serialize<ClaimClanGoalResponse> {
    static geode::Result<ClaimClanGoalResponse> fromJson(matjson::Value const& json) {
        ClaimClanGoalResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) {
            ret.error = json["error"].asString().unwrapOrDefault();
            return geode::Ok(ret);
        }

        ret.rewardStars = json["rewardStars"].asInt().unwrapOrDefault();
        ret.rewardMoons = json["rewardMoons"].asInt().unwrapOrDefault();

        return geode::Ok(ret);
    }
};

arc::Future<ClaimClanGoalResponse> BackendManager::claimClanGoal(int clanID, int goalType) {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["token"] = DataManager::getInstance().getUserToken();
    body["clanId"] = clanID;
    body["goalType"] = goalType;

    co_return co_await requestWithReq<ClaimClanGoalResponse>(buildEncryptedClanRequest(body, accountId), "POST", "/claim_clan_goal");
}


}
