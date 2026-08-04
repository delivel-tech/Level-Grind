#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<AnnouncementsResponse> {
    static geode::Result<AnnouncementsResponse> fromJson(matjson::Value const& json) {
        AnnouncementsResponse ret;

        ret.ok = json["ok"].asBool().unwrapOrDefault();
        if (!ret.ok) return geode::Ok(ret);

        auto ann = json["announcements"].asArray();
        if (!ann) {
            ret.ok = false;
            return geode::Ok(ret);
        }

        for (auto const& val : ann.unwrap()) {
            AnnouncementInfo currentAnn;

            currentAnn.content = val["content"].asString().unwrapOrDefault();
            currentAnn.id = val["id"].asInt().unwrapOrDefault();
            currentAnn.title = val["title"].asString().unwrapOrDefault();
            currentAnn.addedBy = val["added_by"].asString().unwrapOrDefault();
            currentAnn.createdAt = val["created_at"].asString().unwrapOrDefault();

            ret.announcements.push_back(currentAnn);
        }

        return geode::Ok(ret);
    }
};

arc::Future<AnnouncementsResponse> BackendManager::getAnnouncements() {
    co_return co_await request<AnnouncementsResponse>("GET", "/get_announcements");
}

template <>
struct matjson::Serialize<AddAnnouncementResponse> {
    static geode::Result<AddAnnouncementResponse> fromJson(matjson::Value const& json) {
        AddAnnouncementResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<AddAnnouncementResponse> BackendManager::addAnnouncement(std::string title, std::string content) {
    matjson::Value reqBody;

    reqBody["account_id"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["title"] = title;
    reqBody["content"] = content;
    reqBody["added_by"] = std::string(GJAccountManager::sharedState()->m_username);

    co_return co_await request<AddAnnouncementResponse>("POST", "/new_announcement", reqBody);
}

template <>
struct matjson::Serialize<DeleteAnnouncementResponse> {
    static geode::Result<DeleteAnnouncementResponse> fromJson(matjson::Value const& json) {
        DeleteAnnouncementResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<DeleteAnnouncementResponse> BackendManager::deleteAnnouncement(int announcementId) {
    matjson::Value reqBody;

    reqBody["account_id"] = GJAccountManager::sharedState()->m_accountID;
    reqBody["token"] = DataManager::getInstance().getUserToken();
    reqBody["announcement_id"] = announcementId;

    co_return co_await request<DeleteAnnouncementResponse>("POST", "/delete_announcement", reqBody);
}


}
