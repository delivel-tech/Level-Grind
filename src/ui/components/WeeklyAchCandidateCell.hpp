#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchCandidateCell : public CCNode {
public:
    static constexpr CCSize kSize { 150.f, 196.f };

    static WeeklyAchCandidateCell* create(AchievementCellType placement, GJUserScore* userScore);

    bool isFilled() const { return m_filled; }

    GJUserScore* m_userScore = nullptr;
    int m_type = 0;

private:
    bool init(AchievementCellType placement, GJUserScore* userScore);

    void rebuild();
    void buildHeader();
    void buildEmpty();
    void buildFilled();

    void selectType(int type);
    void saveFromUI();

    AchievementCellType m_placement = AchievementCellType::First;
    bool m_filled = false;
    AchievementCellInfo m_info {};

    CCNode* m_content = nullptr;
    TextInput* m_titleInput = nullptr;
    TextInput* m_infoInput = nullptr;
    std::vector<CCSprite*> m_typeSprites;
};

}