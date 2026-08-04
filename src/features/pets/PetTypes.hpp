#pragma once
#include <string>

namespace levelgrind {

struct SyncPetResponse {
    bool ok;
    std::string petName;
    int petStars;
    int petMoons;
    int petLevel;
    int petRarity;
    bool isBanned;
    std::string banReason;
};

struct WipePetResponse {
    bool ok;
};

struct BanPetResponse {
    bool ok;
};

struct UnbanPetResponse {
    bool ok;
};

struct UpgradePetRarityResponse {
    bool ok;
};

struct UpgradePetLevelResponse {
    bool ok;
};

struct RenamePetResponse {
    bool ok;
};

}
