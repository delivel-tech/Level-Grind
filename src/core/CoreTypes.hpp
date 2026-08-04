#pragma once
#include "../features/levels/LevelTypes.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace levelgrind {

struct BootupGetResponse {
    bool ok;
    std::vector<int> owners;
    std::vector<int> helpers;
    std::vector<int> admins;
    std::vector<int> artists;
    std::vector<int> boosters;
    std::vector<int> contributors;
    std::unordered_multimap<int, NoteInfo> notes;
    std::unordered_set<int> levelsWithCoins;
    std::unordered_set<int> levelsWithoutCoins;
};

enum GuidePage {
    MainGuide = 0,
    PetGuide = 1,
    WeeklyAchievementsGuide = 2,
    NotesGuide = 3,
    OwnerRoleGuide = 4,
    AdminRoleGuide = 5,
    HelperRoleGuide = 6,
    ArtistRoleGuide = 7,
    ContribRoleGuide = 8,
    BoosterRoleGuide = 9,
    MainPage = 11,
    EventsGuide = 12,
    GrindPacksGuide = 13,
    HelperSuggestionsGuide = 14,
    LevelIndicatorsGuide = 15,
    CreditsInfo = 16
};

enum GuidePopupState {
    FromMainLayer = 0,
    FromOutside = 1
};

}
