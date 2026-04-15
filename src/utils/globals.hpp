#pragma once
#include <string>
namespace levelgrind {

    // Filters body for get_levels request
    struct GetLevelsBody {
        std::vector<int> difficulties;
        std::vector<int> lengths;
        std::vector<int> demonDifficulties;
        std::vector<std::string> grindTypes;
        std::vector<int> versions;
        bool isNewerFirst;
        bool isRecentlyAdded;
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
        Helper = 1,
        Admin = 2,
        Owner = 3
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
}