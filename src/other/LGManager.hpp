#pragma once
#include "Geode/loader/Log.hpp"
#include <Geode/Geode.hpp>
#include <string>
#include <argon/argon.hpp>
#include <unordered_map>

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

    struct Staff {
        std::vector<int> helpers;
        std::vector<int> admins;
        std::vector<int> artists;
        std::vector<int> boosters;
        std::vector<int> contributors;
    };

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

    Staff& getStaff() {
        return staffs;
    }

    const Staff& getStaff() const {
        return staffs;
    }

    std::unordered_map<int, std::string>& getNotes() {
        return notes;
    }

    void fetchBootup() {
        web::WebRequest req;

        LGManager::get()->getNotes().clear();
        LGManager::get()->getStaff().admins.clear();
        LGManager::get()->getStaff().helpers.clear();
        LGManager::get()->getStaff().contributors.clear();
        LGManager::get()->getStaff().artists.clear();
        LGManager::get()->getStaff().boosters.clear();

        async::spawn(
            req.get("https://delivel.tech/grindapi/bootup_get"),
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

                auto& staff = LGManager::get()->getStaff();

                auto staffs = json["staff"];

                auto admins = staffs["admins"].asArray();
                auto helpers = staffs["helpers"].asArray();
                auto contributors = staffs["contributors"].asArray();
                auto artists = staffs["artists"].asArray();
                auto boosters = staffs["boosters"].asArray();

                for (auto const& val : admins.unwrap()) {
                    auto accountId = val["accountId"].asInt();
                    if (!accountId) continue;
                    staff.admins.push_back(accountId.unwrapOrDefault());
                }

                for (auto const& val : helpers.unwrap()) {
                    auto accountId = val["accountId"].asInt();
                    if (!accountId) continue;
                    staff.helpers.push_back(accountId.unwrapOrDefault());
                }

                for (auto const& val : contributors.unwrap()) {
                    auto accountId = val["accountId"].asInt();
                    if (!accountId) continue;
                    staff.contributors.push_back(accountId.unwrapOrDefault());
                }

                for (auto const& val : artists.unwrap()) {
                    auto accountId = val["accountId"].asInt();
                    if (!accountId) continue;
                    staff.artists.push_back(accountId.unwrapOrDefault());
                }

                for (auto const& val : boosters.unwrap()) {
                    auto accountId = val["accountId"].asInt();
                    if (!accountId) continue;
                    staff.boosters.push_back(accountId.unwrapOrDefault());
                }

                auto notes = json["notes"].asArray();

                if (!notes) {
                    log::warn("bootup_get JSON does not contain notes array");
                    return;
                }

                for (auto const& val : notes.unwrap()) {
                    auto levelID = val["levelID"].asInt();
                    auto note = val["note"].asString();

                    if (!levelID || !note) {
                        continue;
                    }

                    auto& notesMap = LGManager::get()->getNotes();
                    notesMap[levelID.unwrap()] = note.unwrap();
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

    Staff staffs;
    std::unordered_map<int, std::string> notes;
};