#pragma once

#include <Geode/Geode.hpp>
#include "../utils/singleton.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PetManager : public Singleton<PetManager> {
    friend class Singleton<PetManager>;

private:
    PetManager() = default;

public:
    struct PetData {
        bool ok;
        std::string petName;
        int petStars;
        int petMoons;
        int petLevel;
        int petRarity;
        bool isBanned;
        std::string banReason;
    };

    enum PetStyle {
        StandardCube,
        OwnCube,
        OwnCubeWithColors
    };

    int getUpgradeRarityCostByCurrRarity(int rarity);
    PetData parsePetData(web::WebResponse res, LoadingSpinner* spinner);
    float getNextLevelPercentage(int petStars, int nextLvlCost);
    int getUpgradeLvlCostByLevel(int petLevel);
    std::string getPetAgeFromLevel(int petLevel);
    bool isMaxLevel(int petLevel);
    float getPetScale(int level);
    std::string getRarityFromInt(int rarity);
    CCSprite* getPetSprByStyle(PetStyle style);
    CCSprite* getPetSprByStyle(PetStyle style, int iconFrameId);
    int getSelectedCubeIconId();
    void setSelectedCubeIconId(int iconFrameId);
    PetStyle getStyleByLevel(int petLevel);

    int addStarsToPetStarsDelta(int value) {
        auto mod = Mod::get();
        int currStars = mod->getSavedValue<int>("pet-stars-delta");
        mod->setSavedValue("pet-stars-delta", currStars += value);

        return mod->getSavedValue<int>("pet-stars-delta"); // returning new value
    }

    int removeStarsFromPetStarsDelta(int value) {
        auto mod = Mod::get();
        int currStars = mod->getSavedValue<int>("pet-stars-delta");
        mod->setSavedValue("pet-stars-delta", currStars -= value);

        return mod->getSavedValue<int>("pet-stars-delta"); // returning new value
    }

    int addMoonsToPetStarsDelta(int value) {
        auto mod = Mod::get();
        int currMoons = mod->getSavedValue<int>("pet-moons-delta");
        mod->setSavedValue("pet-moons-delta", currMoons += value);

        return mod->getSavedValue<int>("pet-moons-delta"); // returning new value
    }

    int removeMoonsFromPetStarsDelta(int value) {
        auto mod = Mod::get();
        int currMoons = mod->getSavedValue<int>("pet-moons-delta");
        mod->setSavedValue("pet-moons-delta", currMoons -= value);

        return mod->getSavedValue<int>("pet-moons-delta"); // returning new value
    }

    int getPetStarsDelta() {
        auto mod = Mod::get();

        return mod->getSavedValue<int>("pet-stars-delta");
    }

    int getPetMoonsDelta() {
        auto mod = Mod::get();

        return mod->getSavedValue<int>("pet-moons-delta");
    }

    void updatePetDeltasOnCompletion(int stars, int moons) {
        if (isStatValid(stars)) {
            addStarsToPetStarsDelta(stars);
        }
        if (isStatValid(moons)) {
            addMoonsToPetStarsDelta(moons);
        }
    }

    bool isStatValid(int value) {
        if (value > 0 && value < 11) {
            return true;
        }
        return false;
    }

    int resetPetStarsDelta() {
        auto mod = Mod::get();

        mod->setSavedValue("pet-stars-delta", 0);
        return getPetStarsDelta();
    }

    int resetPetMoonsDelta() {
        auto mod = Mod::get();

        mod->setSavedValue("pet-moons-delta", 0);
        return getPetMoonsDelta();
    }

    void resetPetDeltas() {
        resetPetStarsDelta();
        resetPetMoonsDelta();
    }

    bool shouldUpdatePetStars() {
        int starsDelta = getPetStarsDelta();
        if (starsDelta > 0) {
            return true;
        }
        return false;
    }

    bool shouldUpdatePetMoons() {
        int moonsDelta = getPetMoonsDelta();
        if (moonsDelta > 0) {
            return true;
        }
        return false;
    }

    bool m_isRarityUpgraded = false;
    bool m_isLevelUpgraded = false;

    bool isItemBought = false;
};

}