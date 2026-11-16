#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJLevelList.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <array>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace levelgrind::lists {

struct ListLevel {
    int id = 0;
    int stars = 0;
    bool moon = false;
    bool demon = false;
    int coins = 0;
    bool loaded = true;
};

struct GrindList {
    int id = 0;
    std::string name;
    std::string description;
    std::string author;
    int accountId = 0;
    int difficulty = -1;
    int diamonds = 0;
    int levelsRequired = 0;
    bool featured = false;
    int likes = 0;
    int downloads = 0;
    int uploadDate = 0;
    std::vector<ListLevel> levels;
    int totalStars = 0;
    int totalMoons = 0;
    long long addedAt = 0;

    int beaten = 0;
    int left = 0;
    bool finished = false;
};

enum class Sort { Value = 0, AlmostDone = 1, Diamonds = 2, Newest = 3 };

struct Filters {
    bool classic = false, platformer = false, demons = false, coins = false;
    int minDiamonds = 0;
    int maxRequired = 0;
    int maxLeft = 0;
    bool hideFinished = false;
    bool featuredOnly = false;
    bool unratedOnly = false;
    std::array<bool, 11> diffs{};
    Sort sort = Sort::Value;

    static Filters load();
    void save() const;
    matjson::Value serverBody() const;
};

bool listsMode();
void setListsMode(bool on);

web::WebFuture fetchLists(Filters const& f);

std::vector<GrindList> parseLists(web::WebResponse& res, Filters const& f, std::string& error);

GJLevelList* makeLevelList(GrindList const& list);

enum Tag { TagStar = 1, TagMoon = 2, TagCoin = 4, TagDemon = 8 };

}
