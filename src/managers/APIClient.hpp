#pragma once
#include "Auth.hpp"
#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"

#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <string>

using namespace geode::prelude;

namespace levelgrind {

class APIClient : public Singleton<APIClient> {
    friend class Singleton<APIClient>;

private:
    APIClient() = default;

public:
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
    matjson::Value getLevelsJson(GetLevelsBody& body);

    web::WebFuture bootupGet();
    BootupGetResponse bootupGetParse(web::WebResponse res);
    void performBootupGet();

    web::WebFuture getUserRoles(int accountID);
    GetUserRolesResponse getUserRolesParse(web::WebResponse res);

    auth::Future setRoles(const SetRolesBody& body);
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

    auth::Future wipePet(int accountID);
    WipePetResponse wipePetParse(web::WebResponse res);

    auth::Future banPet(int accountID, std::string reason);
    BanPetResponse banPetParse(web::WebResponse res);

    auth::Future unbanPet(int accountID);
    UnbanPetResponse unbanPetParse(web::WebResponse res);

    web::WebFuture getAnnouncements();
    AnnouncementsResponse getAnnouncementsParse(web::WebResponse res);

    auth::Future addAnnouncement(std::string title, std::string content);
    AddAnnouncementResponse addAnnouncementParse(web::WebResponse res);

    auth::Future deleteAnnouncement(int announcementId);
    DeleteAnnouncementResponse deleteAnnouncementParse(web::WebResponse res);

    auth::Future requestStaffAccess();
    ReqAccessResponse requestStaffAccessParse(web::WebResponse res);

    auth::Future getLevelInfo(int levelID);
    GetLevelInfoResponse getLevelInfoParse(web::WebResponse res);

    auth::Future changePoint(PointType type, int coinType, ManageLevelBody levelBody);
    ChangePointResponse changePointParse(web::WebResponse res);

    auth::Future syncLevels(int addThreshold, int deleteThreshold, int coinAddThreshold, int coinDeleteThreshold, int lockThreshold);
    SyncLevelsResponse syncLevelsParse(web::WebResponse res);

    auth::Future newlevel(ManageLevelBody body);
    bool newLevelParse(web::WebResponse res);

    web::WebFuture getGrindPacks();
    GetGrindPacksResponse getGrindPacksParse(web::WebResponse res);

    auth::Future setEvents(int mode, int classicID, int platID);
    bool setEventsParse(web::WebResponse res);

    auth::Future moveEvents(int mode, int days, int hours, int sinceDays, bool dryRun);
    MoveEventsResponse moveEventsParse(web::WebResponse res);

    web::WebFuture getEvents(int mode);
    Events getEventsParse(web::WebResponse res);

    auth::Future syncPet();

    web::WebFuture health();
    bool healthParse(web::WebResponse res);

    auth::Future newGrindPack(NewGrindPackBody body);
    bool newGrindPackParse(web::WebResponse res);

    auth::Future newWeeklyAch(AchievementCellInfo info1, AchievementCellInfo info2, AchievementCellInfo info3);
    bool newWeeklyAchParse(web::WebResponse res);

    web::WebFuture getWeeklyAch();
    WeeklyAchResponse getWeeklyAchParse(web::WebResponse res);

    auth::Future deleteLevel(int levelId);

    auth::Future lockLevel(int levelId, std::string levelName);
    auth::Future unlockLevel(int levelId, std::string levelName);
    auth::Future deleteNotes(int levelId, std::string levelName);

    web::WebFuture getIndicators(int levelID);
    Indicators getIndicatorsParse(web::WebResponse res);

    auth::Future deleteGrindPack(int ID);

    auth::Future cancelVote(int levelID);
    CancelVoteResponse cancelVoteParse(web::WebResponse res);

    auth::Future updateCreditsProfile();
    UpdateCreditsProfileResponse updateCreditsProfileParse(web::WebResponse res);

    static std::string errorMessage(web::WebResponse& res, std::string fallback);
    static int ratingFromLevel(GJGameLevel* level);
    static int coinsFromLevel(GJGameLevel* level);

    void performGetToken();

    std::string baseUrl = "https://api.delivel.tech/v2";
};

}