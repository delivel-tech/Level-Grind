#pragma once
#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"

#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

namespace levelgrind {

// singleton class for web requests to api

class APIClient : public Singleton<APIClient> {
    friend class Singleton<APIClient>;

private:
    APIClient() = default;

public:

    // get_levels methods
    web::WebFuture getLevels(GetLevelsBody& body);
    GetLevelsBody makeGetLevelsBody(
        std::vector<int> difficulties,
        std::vector<int> lengths,
        std::vector<int> demonDifficulties,
        std::vector<std::string> grindTypes,
        std::vector<int> versions,
        bool isNewerFirst,
        bool isRecentlyAdded
    );
    GetLevelsResponse getLevelsParse(web::WebResponse res);

    // bootup_get methods
    web::WebFuture bootupGet();
    BootupGetResponse bootupGetParse(web::WebResponse res);
    void performBootupGet(); // for $on_mod(Loaded)

    // handlers for usermanagepanel
    web::WebFuture getUserRoles(int accountID);
    GetUserRolesResponse getUserRolesParse(web::WebResponse res);

    web::WebFuture setRoles(const SetRolesBody& body);
    SetRolesResponse setRolesParse(web::WebResponse res);

    SetRolesBody makeSetRolesBody(
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
    );

    web::WebFuture wipePet(int accountID);
    WipePetResponse wipePetParse(web::WebResponse res);

    web::WebFuture banPet(int accountID, std::string reason);
    BanPetResponse banPetParse(web::WebResponse res);

    web::WebFuture unbanPet(int accountID);
    UnbanPetResponse unbanPetParse(web::WebResponse res);

    web::WebFuture getAnnouncements();
    AnnouncementsResponse getAnnouncementsParse(web::WebResponse res);

    web::WebFuture addAnnouncement(std::string title, std::string content);
    AddAnnouncementResponse addAnnouncementParse(web::WebResponse res);

    web::WebFuture deleteAnnouncement(int announcementId);
    DeleteAnnouncementResponse deleteAnnouncementParse(web::WebResponse res);

    // argon
    void performGetToken();

    std::string baseUrl = "https://api.delivel.tech";
};

}