#pragma once

#include <Geode/Geode.hpp>
#include "../utils/singleton.hpp"

#include "SecurityManager.hpp"

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
        int accountId = GJAccountManager::get()->m_accountID;
        int curr = SecurityManager::getInstance().getEncryptedInt("pet-stars-delta", accountId);
        curr += value;
        SecurityManager::getInstance().setEncryptedInt("pet-stars-delta", curr, accountId);
        return curr;
    }

    int removeStarsFromPetStarsDelta(int value) {
        int accountId = GJAccountManager::get()->m_accountID;
        int curr = SecurityManager::getInstance().getEncryptedInt("pet-stars-delta", accountId);
        curr -= value;
        SecurityManager::getInstance().setEncryptedInt("pet-stars-delta", curr, accountId);
        return curr;
    }

    int addMoonsToPetStarsDelta(int value) {
        int accountId = GJAccountManager::get()->m_accountID;
        int curr = SecurityManager::getInstance().getEncryptedInt("pet-moons-delta", accountId);
        curr += value;
        SecurityManager::getInstance().setEncryptedInt("pet-moons-delta", curr, accountId);
        return curr;
    }

    int removeMoonsFromPetStarsDelta(int value) {
        int accountId = GJAccountManager::get()->m_accountID;
        int curr = SecurityManager::getInstance().getEncryptedInt("pet-moons-delta", accountId);
        curr -= value;
        SecurityManager::getInstance().setEncryptedInt("pet-moons-delta", curr, accountId);
        return curr;
    }

    int getPetStarsDelta() {
        int accountId = GJAccountManager::get()->m_accountID;
        return SecurityManager::getInstance().getEncryptedInt("pet-stars-delta", accountId);
    }

    int getPetMoonsDelta() {
        int accountId = GJAccountManager::get()->m_accountID;
        return SecurityManager::getInstance().getEncryptedInt("pet-moons-delta", accountId);
    }

    int resetPetStarsDelta() {
        int accountId = GJAccountManager::get()->m_accountID;
        SecurityManager::getInstance().setEncryptedInt("pet-stars-delta", 0, accountId);
        return 0;
    }

    int resetPetMoonsDelta() {
        int accountId = GJAccountManager::get()->m_accountID;
        SecurityManager::getInstance().setEncryptedInt("pet-moons-delta", 0, accountId);
        return 0;
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