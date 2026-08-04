#include "../BackendManager.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

template <>
struct matjson::Serialize<SetEventsResponse> {
    static geode::Result<SetEventsResponse> fromJson(matjson::Value const& json) {
        SetEventsResponse ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();
        return geode::Ok(ret);
    }
};

arc::Future<SetEventsResponse> BackendManager::setEvents(int mode, int classicID, int platID) {
    matjson::Value body;

    body["accountID"] = GJAccountManager::get()->m_accountID;
    body["token"] = DataManager::getInstance().getUserToken();
    body["mode"] = mode;
    body["classicId"] = classicID;
    body["platformerId"] = platID;
    body["addedBy"] = GJAccountManager::get()->m_username.c_str();

    co_return co_await request<SetEventsResponse>("POST", "/set_events", body);
}

template <>
struct matjson::Serialize<Events> {
    static geode::Result<Events> fromJson(matjson::Value const& json) {
        Events ret;
        ret.ok = json["ok"].asBool().unwrapOrDefault();

        if (!ret.ok) {
            return geode::Ok(ret);
        }

        matjson::Value events = json["events"];

        bool classicAdded = false;
        bool platAdded = false;

        for (const auto& event : events) {
            int type = event["type"].asInt().unwrapOrDefault();
            if (type == 0) {
                if (classicAdded) continue;
                ret.classicEvent.exists = true;
                ret.classicEvent.id = event["id"].asInt().unwrapOrDefault();
                ret.classicEvent.levelId = event["levelId"].asInt().unwrapOrDefault();
                classicAdded = true;
                ret.classicEvent.secondsLeft = event["secondsLeft"].asDouble().unwrapOrDefault();
            }
            if (type == 1) {
                if (platAdded) continue;
                ret.platEvent.exists = true;
                ret.platEvent.id = event["id"].asInt().unwrapOrDefault();
                ret.platEvent.levelId = event["levelId"].asInt().unwrapOrDefault();
                platAdded = true;
                ret.platEvent.secondsLeft = event["secondsLeft"].asDouble().unwrapOrDefault();
            }
        }

        return geode::Ok(ret);
    }
};

arc::Future<Events> BackendManager::getEvents(int mode) {
    co_return co_await request<Events>("GET", fmt::format("/get_events_new?mode={}", mode));
}


}
