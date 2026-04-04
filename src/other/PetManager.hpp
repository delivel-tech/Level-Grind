#include "Geode/loader/Mod.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class PetManager {
public:
    static PetManager* get() {
        static PetManager instance;
        return &instance;
    }

    PetManager(const PetManager&) = delete;
    PetManager& operator=(const PetManager&) = delete;
    PetManager(PetManager&&) = delete;
    PetManager& operator=(PetManager&&) = delete;

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

private:
    PetManager() = default;
    ~PetManager() = default;
};