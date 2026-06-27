#include "PetManager.hpp"
#include "Geode/ui/LoadingSpinner.hpp"

using namespace geode::prelude;

namespace levelgrind {

int PetManager::getUpgradeRarityCostByCurrRarity(int rarity) {
    switch (rarity) {
        case 1: return 500; break;
        case 2: return 2000; break;
        case 3: return 4000; break;
        default: return 999999; break;
    }
}

bool PetManager::isMaxLevel(int petLevel) {
    if (petLevel > 29) return true;
    return false;
}

CCSprite* PetManager::getPetSprByStyle(PetStyle style) {
    CCSprite* sprite = nullptr;
    if (style == PetStyle::StandardCube) {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(2);
        playerSpr->updatePlayerFrame(2, IconType::Cube);
        playerSpr->setColors({111, 255, 0}, {0, 251, 255});
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    } else if (style == PetStyle::OwnCube) {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(GameManager::sharedState()->m_playerFrame);
        playerSpr->updatePlayerFrame(GameManager::sharedState()->m_playerFrame, IconType::Cube);
        playerSpr->setColors({111, 255, 0}, {0, 251, 255});
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    } else {
        auto helperSpr = CCSprite::create();
        auto playerSpr = SimplePlayer::create(GameManager::sharedState()->m_playerFrame);
        playerSpr->updatePlayerFrame(GameManager::sharedState()->m_playerFrame, IconType::Cube);
        playerSpr->setColors(
            GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerColor),
            GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerColor2)
        );
        if (GameManager::sharedState()->m_playerGlow != 0) {
            playerSpr->setGlowOutline(
                GameManager::sharedState()->colorForIdx(GameManager::sharedState()->m_playerGlowColor)
            );
        }
        helperSpr->addChild(playerSpr);
        sprite = helperSpr;
    }

    sprite->setID("pet-sprite");
    return sprite;
}

PetManager::PetStyle PetManager::getStyleByLevel(int petLevel) {
    PetStyle style;

    if (petLevel < 5) {
        style = PetStyle::StandardCube;
    } else if (petLevel >= 5 && petLevel < 15) {
        style = PetStyle::OwnCube;
    } else {
        style = PetStyle::OwnCubeWithColors;
    }

    return style;
}

std::string PetManager::getRarityFromInt(int rarity) {
    std::string rarityText = "";
    switch (rarity) {
        case 1: rarityText = "Common"; break;
        case 2: rarityText = "Rare"; break;
        case 3: rarityText = "Legendary"; break;
        case 4: rarityText = "Mythic"; break;
    }

    return rarityText;
}

float PetManager::getPetScale(int level) {
    const float minScale = 0.9f;
    const float maxScale = 1.5f;
    const int maxLevel = 30;

    if (level <= 1) return minScale;
    if (level >= maxLevel) return maxScale;

    float t = (float)(level - 1) / (maxLevel - 1);
    return minScale * pow(maxScale / minScale, t);
}

std::string PetManager::getPetAgeFromLevel(int petLevel) {
    if (petLevel < 5) {
        return "Baby";
    } else if (petLevel >= 5 && petLevel < 10) {
        return "Toddler";
    } else if (petLevel >= 10 && petLevel < 15) {
        return "Teen";
    } else if (petLevel >= 15 && petLevel < 20) {
        return "Adult";
    } else if (petLevel >= 20 && petLevel < 25) {
        return "Elder";
    } else if (petLevel >= 25 && petLevel < 30) {
        return "Master";
    } else {
        return "Ascended";
    }
}

int PetManager::getUpgradeLvlCostByLevel(int petLevel) {
    const int MAX_LEVEL = 30;
    const int TOTAL_COST = 100000;
    
    auto baseCost = [&](int level) -> double {
        return pow(level, 2);
    };

    auto getTotalBase = [&]() -> double {
        double sum = 0;
        for (int i = 1; i <= MAX_LEVEL; i++) {
            sum += baseCost(i);
        }
        return sum;
    };

    static double scale = TOTAL_COST / getTotalBase();
    int nextLevel = petLevel + 1;

    return (int)(baseCost(nextLevel) * scale);
}

float PetManager::getNextLevelPercentage(int petStars, int nextLvlCost) {
    float percentage = ((float)petStars / nextLvlCost) * 100;
    if (percentage > 100) {
        percentage = 100;
    }

    return percentage;
}

PetManager::PetData PetManager::parsePetData(web::WebResponse res, LoadingSpinner* spinner) {
    PetData data;
    data.ok = true;
    if (!res.ok()) {
        log::error("invalid json in response");
        Notification::create("Failed to load pet data. Try again later.", NotificationIcon::Error)->show();
        data.ok = false;
        if (spinner) spinner->removeFromParent();
        return data;
    }

    auto jsonRes = res.json();

    if (!jsonRes) {
        log::error("invalid json in response");
        Notification::create("Failed to load pet data. Try again later.", NotificationIcon::Error)->show();
        data.ok = false;
        if (spinner) spinner->removeFromParent();
        return data;
    }

    auto jsonUnwrapped = jsonRes.unwrap();
    auto json = jsonUnwrapped["pet"];

    data.petName = json["petName"].asString().unwrapOrDefault();
    data.petStars = json["petStars"].asInt().unwrapOrDefault();
    data.petMoons = json["petMoons"].asInt().unwrapOrDefault();
    data.petLevel = json["petLevel"].asInt().unwrapOrDefault();
    Mod::get()->setSavedValue("last-pet-lvl", data.petLevel);
    data.petRarity = json["petRarity"].asInt().unwrapOrDefault();
    data.isBanned = json["isBanned"].asBool().unwrapOrDefault();
    if (data.isBanned) {
        data.banReason = json["banReason"].asString().unwrapOrDefault();
        Notification::create(fmt::format("Banned: {}", data.banReason), NotificationIcon::Error)->show();
        log::info("pet is banned: {}", data.banReason);
    }

    return data;
}
    
}