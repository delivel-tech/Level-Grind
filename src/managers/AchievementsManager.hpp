#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind::achievements {

struct Achievement {
    std::string id;
    std::string category;
    std::string title;
    std::string description;
    long long target = 1;
    long long progress = 0;
    int gp = 0;
    bool unlocked = false;
};

struct Category {
    const char* id;
    const char* name;
    const char* icon;
    bool modSprite = false;
};
std::vector<Category> const& categories();
Category const* category(std::string const& id);

void check(geode::Function<void(std::vector<Achievement> const&)> onDone = nullptr);

}
