#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/Enums.hpp>
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
        RandomButtonGuide = 10,
        MainPage = 11
    };

    enum GuidePopupState {
        FromMainLayer = 0,
        FromOutside = 1
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

    struct ReqAccessResponse {
        bool ok;
        int pos;
    };
}