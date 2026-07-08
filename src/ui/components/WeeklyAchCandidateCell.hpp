#pragma once

#include "Geode/cocos/menu_nodes/CCMenu.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchCandidateCell : public CCMenu {
public:
    static WeeklyAchCandidateCell* create(std::pair<bool, AchievementCellInfo> candidateInfo, AchievementCellType placement, GJUserScore* userScore);

    void createUI(AchievementCellInfo candidateInfo, AchievementCellType placement);

    GJUserScore* m_userScore = nullptr;
    int m_type;

private:
    bool init(std::pair<bool, AchievementCellInfo> candidateInfo, AchievementCellType placement, GJUserScore* userScore);
};

}