#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementsManager : public Singleton<WeeklyAchievementsManager> {
    friend class Singleton<WeeklyAchievementsManager>;

private:
    WeeklyAchievementsManager() = default;

public:
    void saveConfigData(bool confirmed, AchievementCellType placement, AchievementCellInfo configData);
    std::pair<bool, AchievementCellInfo> loadConfigDataFromSaved(AchievementCellType placement);
    void wipeConfigData(AchievementCellType placement);
};

}