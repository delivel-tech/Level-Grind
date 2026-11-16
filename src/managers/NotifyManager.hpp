#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind::notify {

enum class Type { ClanMessage, ClanTask, ClanRequest, Events, WeeklyAch, NewLevels, NewPacks };

struct TypeInfo {
    Type type;
    const char* key;
    const char* name;
    const char* desc;
};
std::vector<TypeInfo> const& types();

bool enabled(Type type);
void setEnabled(Type type, bool on);

void show(std::string const& title, std::string const& text, const char* iconFrame, bool modSprite = false);

void showAchievement(std::string const& title, std::string const& text, int gp);

void start();

}
