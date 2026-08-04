#include "WeeklyAchTypes.hpp"
#include <Geode/Geode.hpp>
#include "../../shared/PodiumNineSlice.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementCell : public CCMenu {
public:
    static WeeklyAchievementCell* create(AchievementCellType cellType, AchievementCellInfo info);

    void buildAfterPodium();
    void buildPodium();
    void buildInfo();

    AchievementCellType m_cellType;
    AchievementCellInfo m_cellInfo;
    PodiumNineSlice* m_podium = nullptr;

private:
    bool init(AchievementCellType cellType, AchievementCellInfo info);
};

}