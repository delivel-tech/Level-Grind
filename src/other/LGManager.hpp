#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <argon/argon.hpp>

using namespace geode::prelude;

class LGManager {
public:
    static LGManager* get() {
        static LGManager instance;
        return &instance;
    }

    LGManager(const LGManager&) = delete;
    LGManager& operator=(const LGManager&) = delete;
    LGManager(LGManager&&) = delete;
    LGManager& operator=(LGManager&&) = delete;

    void setArgonToken(const std::string& token) {
        m_argonToken = token;
    }

    std::string getArgonToken() const {
        if (!m_argonToken.empty()) {
            return m_argonToken;
        } else {
            return Mod::get()->getSavedValue<std::string>("argon_token");
        }
    }

    void authArgon() {
        async::spawn(
            argon::startAuth(),
            [](Result<std::string> res) {
                if (!res.ok()) return;
			    auto token = std::move(res).unwrap();
			    Mod::get()->setSavedValue("argon_token", token);
                LGManager::get()->setArgonToken(token);
            }
        );
    }

    std::unordered_map<int, int>& getStaff() {
        return staffs;
    }

    const std::unordered_map<int, int>& getStaff() const {
        return staffs;
    }

    void fetchStaff() {
        web::WebRequest req;

        async::spawn(
            req.get("https://delivel.tech/grindapi/get_staff"),
            [](web::WebResponse res) {
                if (!res.ok()) {
                    log::warn("get_staff returned non-ok status: {}", res.code());
                    return;
                }

                auto jsonRes = res.json();
                if (!jsonRes) {
                    log::warn("Failed to parse get_staff JSON");
                    return;
                }

                auto json = jsonRes.unwrap();

                bool isOK = json["ok"].asBool().unwrapOrDefault();
                if (!isOK) {
                    log::warn("Server returned ok=false for get_staff");
                    return;
                }

                auto staff = json["staff"].asArray();

                if (!staff) {
                    log::warn("get_staff JSON does not contain staff array");
                    return;
                }

                for (auto const& val : staff.unwrap()) {
                    auto accountID = val["accountID"].asInt();
                    auto role = val["role"].asInt();

                    if (!accountID || !role) {
                        continue;
                    }

                    auto& staffMap = LGManager::get()->getStaff();
                    staffMap[accountID.unwrap()] = role.unwrap();
                }
            }
        );
    }

    bool isAdmin() {
        return Mod::get()->getSavedValue<bool>("isAdmin");
    }

    void setAdmin(bool isAdmin) {
        Mod::get()->setSavedValue("isAdmin", isAdmin);
    }

    bool isHelper() {
        return Mod::get()->getSavedValue<bool>("isHelper");
    }

    void setHelper(bool isHelper) {
        Mod::get()->setSavedValue("isHelper", isHelper);
    }
    
    bool isDelivel() {
        if (GJAccountManager::sharedState()->m_accountID == 13678537) {
            return true;
        } else {
            return false;
        }
    }

private:
    LGManager() = default;
    ~LGManager() = default;

    std::string m_argonToken;

    std::unordered_map<int, int> staffs; // accountId, role (1 owner, 2 admin)
};