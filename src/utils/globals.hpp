#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include "ccTypes.h"
#include <Geode/Enums.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <string>
namespace levelgrind {

    // Filters body for get_levels request
    struct GetLevelsBody {
        std::vector<int> difficulties;
        std::vector<int> lengths;
        std::vector<int> demonDifficulties;
        std::vector<std::string> grindTypes;
        std::vector<int> versions;
        bool isNewerFirst = true;
        bool isRecentlyAdded = false;
    };

    // Body that is returned from get_levels
    struct GetLevelsResponse {
        bool ok;
        int count;
        std::vector<int> ids;
    };

    struct LevelSuggestion {
        int id;
        int points;
    };
    
    // Body returned from bootup_get
    struct BootupGetResponse {
        bool ok;
        std::vector<int> owners;
        std::vector<int> helpers;
        std::vector<int> admins;
        std::vector<int> artists;
        std::vector<int> boosters;
        std::vector<int> contributors;
        std::unordered_map<int, std::string> notes;
        std::unordered_set<int> levelsWithCoins;
        std::unordered_set<int> levelsWithoutCoins;
    };

    enum GrindPosition {
        User = 0,
        Helper = 1,
        Admin = 2,
        Owner = 3
    };

    struct PointInfo {
        int point;
        int coinPoint;
        int staffId;
        std::string staffUsername;
    };

    struct GetLevelInfoResponse {
        bool ok;
        bool isAdded;
        bool isLocked;
        bool star;
        bool moon;
        bool coin;
        bool demon;
        std::string addedBy;
        bool noteExists;
        std::string note;
        bool isDaily;
        bool isDailyPlat;
        bool isWeekly;
        bool isWeeklyPlat;
        bool hasPoints;
        int points;
        int coinPoints;
        std::vector<PointInfo> pointsInfo;
    };

    enum PointType {
        AcceptPoint = 1,
        RejectPoint = -1
    };

    enum CoinPointType {
        AcceptCoinPoint = 1,
        RejectCoinPoint = -1
    };

    struct ChangePointResponse {
        bool ok;
    };

    struct SyncLevelsResponse {
        bool ok;
        int deleted;
        int inserted;
        int coinUpdates;
    };

    struct ManageLevelBody {
        int id;
        std::string name;
        int difficulty;
        int length;
        int demonDifficulty;
        bool star;
        bool moon;
        bool coin;
        bool demon;
    };

    enum GuidePage {
        MainGuide = 0,
        PetGuide = 1,
        WeeklyAchievementsGuide = 2,
        NotesGuide = 3,
        OwnerRoleGuide = 4,
        AdminRoleGuide = 5,
        HelperRoleGuide = 6,
        ArtistRoleGuide = 7,
        ContribRoleGuide = 8,
        BoosterRoleGuide = 9,
        MainPage = 11
    };

    enum GuidePopupState {
        FromMainLayer = 0,
        FromOutside = 1
    };

    struct EventInfo {
        bool exists;
        int levelId;
        int id;
        double secondsLeft;
    };

    struct Events {
        bool ok;
        EventInfo classicEvent;
        EventInfo platEvent;
    };

    enum EventType {
        Daily = 0,
        Weekly = 1,
        Monthly = 2
    };

    enum CustomBrowserType {
        Search = 0, EventsHistory = 1
    };

    enum AchievementCellType {
        First = 0, Second = 1, Third = 2
    };

    enum AchievementType {
        Star = 0,
        Moon = 1,
        Coin = 2,
        Demon = 3
    };

    struct AchievementCellInfo {
        std::string username;
        int accountID;
        int icon;
        int colorFirst;
        int colorSecond;
        int glow;
        std::string title;
        int type;
        std::string info;
    };

    struct WeeklyAchResponse {
        bool ok;
        AchievementCellInfo cell1;
        AchievementCellInfo cell2;
        AchievementCellInfo cell3;
    };

    struct GrindPackLevels {
        int id1;
        int id2;
        int id3;
    };

    enum CustomDifficultyEnum {
        Auto = 1,
        Easy = 2,
        Normal = 3,
        Hard = 4,
        Harder = 5,
        Insane = 6,
        EasyDemon = 7,
        MediumDemon = 8,
        HardDemon = 9,
        InsaneDemon = 10,
        ExtremeDemon = 11
    };

    struct GrindPack {
        int id;
        std::string title;
        cocos2d::ccColor3B color;
        GrindPackLevels levels;
        CustomDifficultyEnum difficulty;
        bool isStar;
    };

    struct GetGrindPacksResponse {
        bool ok;
        std::vector<GrindPack> packs;
    };

    struct AnnouncementInfo {
        int id;
        std::string title;
        std::string content;
        std::string addedBy;
        std::string createdAt;
    };

    struct AnnouncementsResponse {
        std::vector<AnnouncementInfo> announcements;
        bool ok;
    };

    struct AddAnnouncementResponse {
        bool ok;
    };

    struct DeleteAnnouncementResponse {
        bool ok;
    };

    struct UserRoles {
        bool isOwner;
        bool isAdmin;
        bool isHelper;
        bool isArtist;
        bool isBooster;
        bool isContributor;
    };

    struct GetUserRolesResponse {
        bool ok;
        UserRoles roles;
        bool petExists;
        bool isPetBanned;
    };

    struct SetRolesBody {
        int accountID;
        std::string token;
        int targetAccountID;
        std::string targetUsername;
        int targetIcon;
        int targetColor1;
        int targetColor2;
        int targetColor3;
        bool isAdmin;
        bool isHelper;
        bool isArtist;
        bool isContributor;
        bool isBooster;
    };

    struct SetRolesResponse {
        bool ok;
    };

    struct WipePetResponse {
        bool ok;
    };

    struct BanPetResponse {
        bool ok;
    };

    struct UnbanPetResponse {
        bool ok;
    };

    enum Length {
        Short = 1,
        Medium = 2,
        Long = 3,
        XL = 4
    };

    struct LengthInfo {
        Length length;
        std::string lengthName;
        std::string lengthId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct VersionInfo {
        int version;
        std::string versionName;
        std::string versionId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct FilterInfo {
        std::string filterName;
        std::string top;
        std::string filterId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct DemonDifficultyInfo {
        int demonDifficulty;
        std::string sprite;
        std::string id;
        cocos2d::SEL_MenuHandler cb;
    };

    struct NewGrindPackBody {
        std::string title;
        bool star;
        int levelId1;
        int levelId2;
        int levelId3;
        int color1;
        int color2;
        int color3;
        int difficulty;
    };

    struct ReqAccessResponse {
        bool ok;
        int pos;
    };
}