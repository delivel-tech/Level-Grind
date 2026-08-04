#pragma once
#include <string>

namespace levelgrind {

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

}
