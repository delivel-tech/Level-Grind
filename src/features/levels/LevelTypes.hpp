#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include "ccTypes.h"
#include <string>
#include <vector>

namespace levelgrind {

struct GetLevelsBody {
    std::vector<int> difficulties;
    std::vector<int> lengths;
    std::vector<int> demonDifficulties;
    std::vector<std::string> grindTypes;
    std::vector<int> versions;
    bool isNewerFirst = true;
    bool isRecentlyAdded = false;
};

struct GetLevelsResponse {
    bool ok;
    int count;
    std::vector<int> ids;
};

struct LevelSuggestion {
    int id;
    int points;
};

struct NoteInfo {
    int levelID;
    std::string note;
    std::string senderUsername;
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
    int id = 0;
    std::string name = "";
    int difficulty = 0;
    int length = 0;
    int demonDifficulty = 0;
    bool star = false;
    bool moon = false;
    int coin = 0;
    bool demon = false;
};

struct CancelVoteResponse {
    bool ok;
};

struct Indicators {
    bool ok;
    bool added;
    bool coin;
    bool event;
    bool pack;
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

struct NewLevelResponse {
    bool ok;
};

struct NewGrindPackResponse {
    bool ok;
};

struct DeleteLevelResponse {
    bool ok;
};

struct LockLevelResponse {
    bool ok;
};

struct UnlockLevelResponse {
    bool ok;
};

struct DeleteNotesResponse {
    bool ok;
};

struct DeleteGrindPackResponse {
    bool ok;
};

}
