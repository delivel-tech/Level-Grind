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

    web::WebFuture requestStaffAccess();
    ReqAccessResponse requestStaffAccessParse(web::WebResponse res);

    web::WebFuture getLevelInfo(int levelID);
    GetLevelInfoResponse getLevelInfoParse(web::WebResponse res);

    web::WebFuture changePoint(PointType type, int coinType, ManageLevelBody levelBody);
    ChangePointResponse changePointParse(web::WebResponse res);

    web::WebFuture syncLevels(int addThreshold, int deleteThreshold, int coinAddThreshold);
    SyncLevelsResponse syncLevelsParse(web::WebResponse res);

    web::WebFuture newlevel(ManageLevelBody body);
    bool newLevelParse(web::WebResponse res);

    web::WebFuture getGrindPacks();
    GetGrindPacksResponse getGrindPacksParse(web::WebResponse res);

    web::WebFuture setEvents(int mode, int classicID, int platID);
    bool setEventsParse(web::WebResponse res);

    web::WebFuture getEvents(int mode);
    Events getEventsParse(web::WebResponse res);

    web::WebFuture syncPet();

    web::WebFuture health();
    bool healthParse(web::WebResponse res);

    web::WebFuture newGrindPack(NewGrindPackBody body);
    bool newGrindPackParse(web::WebResponse res);

    web::WebFuture newWeeklyAch(AchievementCellInfo info1, AchievementCellInfo info2, AchievementCellInfo info3);
    bool newWeeklyAchParse(web::WebResponse res);

    web::WebFuture getWeeklyAch();
    WeeklyAchResponse getWeeklyAchParse(web::WebResponse res);

    web::WebFuture deleteLevel(int levelId);

    web::WebFuture lockLevel(int levelId, std::string levelName);
    web::WebFuture unlockLevel(int levelId, std::string levelName);
    web::WebFuture deleteNotes(int levelId, std::string levelName);

    web::WebFuture getIndicators(int levelID);
    Indicators getIndicatorsParse(web::WebResponse res);

    web::WebFuture deleteGrindPack(int ID);

    // argon
    void performGetToken();

    std::string baseUrl = "https://api.delivel.tech";
};

}