#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include <string>

namespace levelgrind {

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

enum CustomBrowserType {
    Search = 0, EventsHistory = 1
};

}
