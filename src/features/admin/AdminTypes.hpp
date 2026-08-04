#pragma once
#include <string>

namespace levelgrind {

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

struct ReqAccessResponse {
    bool ok;
    int pos;
};

}
