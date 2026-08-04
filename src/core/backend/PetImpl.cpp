#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/PetManager.hpp"
#include "../../managers/SecurityManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<SyncPetResponse> {
    static geode::Result<SyncPetResponse> fromJson(matjson::Value const& value) {
        SyncPetResponse data;
        data.ok = true;

        auto json = value["pet"];

        data.petName = json["petName"].asString().unwrapOrDefault();
        data.petStars = json["petStars"].asInt().unwrapOrDefault();
        data.petMoons = json["petMoons"].asInt().unwrapOrDefault();
        data.petLevel = json["petLevel"].asInt().unwrapOrDefault();
        data.petRarity = json["petRarity"].asInt().unwrapOrDefault();
        data.isBanned = json["isBanned"].asBool().unwrapOrDefault();
        if (data.isBanned) {
            data.banReason = json["banReason"].asString().unwrapOrDefault();
        }

        return geode::Ok(data);
    }
};

arc::Future<SyncPetResponse> BackendManager::syncPet() {
    int accountId = GJAccountManager::get()->m_accountID;

    matjson::Value body;
    body["accountId"] = accountId;
    body["username"] = GJAccountManager::get()->m_username.c_str();
    body["token"] = DataManager::getInstance().getUserToken();

    if (PetManager::getInstance().shouldUpdatePetStars()) {
        body["shouldUpdateStars"] = true;
        body["starsDelta"] = PetManager::getInstance().getPetStarsDelta();
    }

    if (PetManager::getInstance().shouldUpdatePetMoons()) {
        body["shouldUpdateMoons"] = true;
        body["moonsDelta"] = PetManager::getInstance().getPetMoonsDelta();
    }

    matjson::Value wrapper;
    wrapper["accountId"] = accountId;
    wrapper["payload"] = SecurityManager::getInstance().buildEncryptedPayload(body, accountId);

    co_return co_await request<SyncPetResponse>("POST", "/sync_pet", wrapper);
}

template <>
struct matjson::Serialize<WipePetResponse> {
    static geode::Result<WipePetResponse> fromJson(matjson::Value const& json) {
        WipePetResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<WipePetResponse> BackendManager::wipePet(int accountID) {
    matjson::Value reqBody;
    reqBody["accountID"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountID"] = accountID;

    co_return co_await request<WipePetResponse>("POST", "/wipe_pet_data", reqBody);
}

template <>
struct matjson::Serialize<BanPetResponse> {
    static geode::Result<BanPetResponse> fromJson(matjson::Value const& json) {
        BanPetResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<BanPetResponse> BackendManager::banPet(int accountID, std::string reason) {
    matjson::Value reqBody;
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountId"] = accountID;
    reqBody["banReason"] = reason;

    co_return co_await request<BanPetResponse>("POST", "/ban_pet", reqBody);
}

template <>
struct matjson::Serialize<UnbanPetResponse> {
    static geode::Result<UnbanPetResponse> fromJson(matjson::Value const& json) {
        UnbanPetResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<UnbanPetResponse> BackendManager::unbanPet(int accountID) {
    matjson::Value reqBody;
    reqBody["accountId"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["targetAccountId"] = accountID;

    co_return co_await request<UnbanPetResponse>("POST", "/unban_pet", reqBody);
}


}
