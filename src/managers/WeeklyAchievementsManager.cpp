#include "WeeklyAchievementsManager.hpp"
#include <fmt/format.h>
#include <utility>

namespace levelgrind {

void WeeklyAchievementsManager::saveConfigData(bool confirmed, AchievementCellType placement, AchievementCellInfo configData) {
    auto getPlacementID = [](AchievementCellType placement) {
        return static_cast<int>(placement);
    };

    auto mod = Mod::get();

    #define MAKE_ID(str, placement) fmt::format(str, getPlacementID(placement))

    mod->setSavedValue(MAKE_ID("ach-{}-confirmed", placement), confirmed);
    mod->setSavedValue(MAKE_ID("ach-{}-accountID", placement), configData.accountID);
    mod->setSavedValue(MAKE_ID("ach-{}-username", placement), configData.username);
    mod->setSavedValue(MAKE_ID("ach-{}-icon", placement), configData.icon);
    mod->setSavedValue(MAKE_ID("ach-{}-color-first", placement), configData.colorFirst);
    mod->setSavedValue(MAKE_ID("ach-{}-color-second", placement), configData.colorSecond);
    mod->setSavedValue(MAKE_ID("ach-{}-glow", placement), configData.glow);
    mod->setSavedValue(MAKE_ID("ach-{}-type", placement), configData.type);
    mod->setSavedValue(MAKE_ID("ach-{}-title", placement), configData.title);
    mod->setSavedValue(MAKE_ID("ach-{}-info", placement), configData.info);
    
    #undef MAKE_ID
}

std::pair<bool, AchievementCellInfo> WeeklyAchievementsManager::loadConfigDataFromSaved(AchievementCellType placement) {
    AchievementCellInfo info;

    auto getPlacementID = [](AchievementCellType placement) {
        return static_cast<int>(placement);
    };

    auto mod = Mod::get();

    #define MAKE_ID(str, placement) fmt::format(str, getPlacementID(placement))

    bool confirmed = mod->getSavedValue<bool>(MAKE_ID("ach-{}-confirmed", placement));
    info.accountID = mod->getSavedValue<int>(MAKE_ID("ach-{}-accountID", placement));
    info.username = mod->getSavedValue<std::string>(MAKE_ID("ach-{}-username", placement));
    info.title = mod->getSavedValue<std::string>(MAKE_ID("ach-{}-title", placement));
    info.type = mod->getSavedValue<int>(MAKE_ID("ach-{}-type", placement));
    info.info = mod->getSavedValue<std::string>(MAKE_ID("ach-{}-info", placement));
    info.icon = mod->getSavedValue<int>(MAKE_ID("ach-{}-icon", placement));
    info.colorFirst = mod->getSavedValue<int>(MAKE_ID("ach-{}-color-first", placement));
    info.colorSecond = mod->getSavedValue<int>(MAKE_ID("ach-{}-color-second", placement));
    info.glow = mod->getSavedValue<int>(MAKE_ID("ach-{}-glow", placement));

    #undef MAKE_ID
    return std::make_pair(confirmed, info);
}

void WeeklyAchievementsManager::wipeConfigData(AchievementCellType placement) {
    auto getPlacementID = [](AchievementCellType placement) {
        return static_cast<int>(placement);
    };

    auto mod = Mod::get();

    #define MAKE_ID(str, placement) fmt::format(str, getPlacementID(placement))

    mod->setSavedValue(MAKE_ID("ach-{}-confirmed", placement), false);
    mod->setSavedValue(MAKE_ID("ach-{}-accountID", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-username", placement), std::string(""));
    mod->setSavedValue(MAKE_ID("ach-{}-icon", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-color-first", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-color-second", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-glow", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-type", placement), 0);
    mod->setSavedValue(MAKE_ID("ach-{}-title", placement), std::string(""));
    mod->setSavedValue(MAKE_ID("ach-{}-info", placement), std::string(""));
    
    #undef MAKE_ID
}

}