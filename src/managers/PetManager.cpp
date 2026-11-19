#include "PetManager.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/GameManager.hpp>
#include <fmt/format.h>
#include <cmath>
#include "Consent.hpp"
#include "Auth.hpp"
#include "APIClient.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>

using namespace geode::prelude;

namespace levelgrind {

static std::string romanNumeral(int n) {
    static constexpr std::pair<int, const char*> kParts[] = { { 10, "X" }, { 9, "IX" }, { 5, "V" }, { 4, "IV" }, { 1, "I" } };
    std::string out;
    for (auto const& [value, text] : kParts) {
        while (n >= value) {
            out += text;
            n -= value;
        }
    }
    return out;
}

int PetManager::getUpgradeRarityCostByCurrRarity(int rarity) {
    switch (rarity) {
        case 1: return 1500;
        case 2: return 4000;
        case 3: return 8000;
    }
    if (rarity < 4 || rarity >= kMaxRarity) return 0;
    return static_cast<int>(std::round(8000.0 * std::pow(1.5, rarity - 3) / 500.0) * 500.0);
}

bool PetManager::isMaxLevel(int petLevel) {
    return petLevel >= kMaxLevel;
}

CCSprite* PetManager::getPetSprByStyle(PetStyle style) {
    return getPetSprByStyle(style, getSelectedCubeIconId());
}

namespace {

int packColor(ccColor3B c) {
    return (c.r << 16) | (c.g << 8) | c.b;
}

ccColor3B unpackColor(int v) {
    return { static_cast<GLubyte>((v >> 16) & 255), static_cast<GLubyte>((v >> 8) & 255), static_cast<GLubyte>(v & 255) };
}

TaskHolder<web::WebResponse>& appearanceTask() {
    static TaskHolder<web::WebResponse> task;
    return task;
}

}

PetManager::Appearance PetManager::localAppearance() {
    auto mod = Mod::get();
    Appearance a;
    a.customColors = mod->getSavedValue<bool>("custom-colors-enabled");
    a.color1 = mod->getSavedValue<cocos2d::ccColor3B>("pet-color-main");
    a.color2 = mod->getSavedValue<cocos2d::ccColor3B>("pet-color-sec");
    a.customGlow = mod->getSavedValue<bool>("custom-glow-enabled");
    a.glow = mod->getSavedValue<cocos2d::ccColor3B>("pet-glow");
    a.customIcon = mod->getSavedValue<bool>("custom-icon-enabled");
    a.iconId = std::max(0, mod->getSavedValue<int>("pet-custom-icon-id"));
    return a;
}

void PetManager::storeLocalAppearance(Appearance const& a) {
    auto mod = Mod::get();
    mod->setSavedValue("custom-colors-enabled", a.customColors);
    mod->setSavedValue("pet-color-main", a.color1);
    mod->setSavedValue("pet-color-sec", a.color2);
    mod->setSavedValue("custom-glow-enabled", a.customGlow);
    mod->setSavedValue("pet-glow", a.glow);
    mod->setSavedValue("custom-icon-enabled", a.customIcon);
    mod->setSavedValue("pet-custom-icon-id", a.iconId > 0 ? a.iconId : -1);
}

bool PetManager::hasLocalCustomization() {
    auto a = localAppearance();
    return a.customColors || a.customGlow || a.customIcon;
}

PetManager::OwnerLook PetManager::localOwner() {
    auto gm = GameManager::sharedState();
    OwnerLook o;
    o.frame = gm->m_playerFrame;
    o.color1 = gm->colorForIdx(gm->m_playerColor);
    o.color2 = gm->colorForIdx(gm->m_playerColor2);
    o.glow = gm->m_playerGlow != 0;
    o.glowColor = gm->colorForIdx(gm->m_playerGlowColor);
    return o;
}

matjson::Value PetManager::appearanceToJson(Appearance const& a) {
    matjson::Value v;
    v["customColors"] = a.customColors;
    v["color1"] = packColor(a.color1);
    v["color2"] = packColor(a.color2);
    v["customGlow"] = a.customGlow;
    v["glow"] = packColor(a.glow);
    v["customIcon"] = a.customIcon;
    v["iconId"] = a.iconId;
    return v;
}

PetManager::Appearance PetManager::appearanceFromJson(matjson::Value const& v) {
    Appearance a;
    a.customColors = v["customColors"].asBool().unwrapOr(false);
    a.color1 = unpackColor(static_cast<int>(v["color1"].asInt().unwrapOr(packColor(a.color1))));
    a.color2 = unpackColor(static_cast<int>(v["color2"].asInt().unwrapOr(packColor(a.color2))));
    a.customGlow = v["customGlow"].asBool().unwrapOr(false);
    a.glow = unpackColor(static_cast<int>(v["glow"].asInt().unwrapOr(packColor(a.glow))));
    a.customIcon = v["customIcon"].asBool().unwrapOr(false);
    a.iconId = static_cast<int>(v["iconId"].asInt().unwrapOr(0));
    return a;
}

std::string PetManager::moodName(int mood) {
    if (mood >= 2) return "Happy";
    if (mood == 1) return "Okay";
    return "Hungry";
}

ccColor3B PetManager::moodColor(int mood) {
    if (mood >= 2) return { 120, 255, 120 };
    if (mood == 1) return { 255, 230, 120 };
    return { 255, 120, 120 };
}

CCSprite* PetManager::createMiniPet(int petLevel, matjson::Value const& appearance, int icon, int color1, int color2, int color3) {
    auto gm = GameManager::sharedState();
    OwnerLook owner;
    owner.frame = std::max(1, icon);
    owner.color1 = gm->colorForIdx(color1);
    owner.color2 = gm->colorForIdx(color2);
    owner.glow = color3 > 0;
    owner.glowColor = gm->colorForIdx(color3);
    Appearance a = appearance.isObject() ? appearanceFromJson(appearance) : Appearance{};
    return createPetSprite(PetManager::getInstance().getStyleByLevel(std::max(1, petLevel)), a, owner);
}

void PetManager::pushAppearance(geode::Function<void()> done) {
    if (!consent::granted()) {
        if (done) done();
        return;
    }
    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["appearance"] = appearanceToJson(localAppearance());
    appearanceTask().spawn(auth::post(fmt::format("{}/pet_appearance", APIClient::getInstance().baseUrl), std::move(body)), [done = std::move(done)](web::WebResponse) mutable {
        if (done) done();
    });
}

CCSprite* PetManager::createPetSprite(PetStyle style, Appearance const& a, OwnerLook const& owner) {
    int frame = style == PetStyle::StandardCube ? 2 : std::max(1, owner.frame);
    if (a.customIcon && a.iconId > 0) frame = a.iconId;

    auto helperSpr = CCSprite::create();
    auto playerSpr = SimplePlayer::create(frame);
    playerSpr->updatePlayerFrame(frame, IconType::Cube);
    if (a.customColors) {
        playerSpr->setColors(a.color1, a.color2);
    } else if (style == PetStyle::OwnCubeWithColors) {
        playerSpr->setColors(owner.color1, owner.color2);
    } else {
        playerSpr->setColors({ 111, 255, 0 }, { 0, 251, 255 });
    }
    if (a.customGlow) {
        playerSpr->setGlowOutline(a.glow);
    } else {
        playerSpr->disableGlowOutline();
    }
    helperSpr->addChild(playerSpr);
    helperSpr->setID("pet-sprite");
    return helperSpr;
}

CCSprite* PetManager::getPetSprByStyle(PetStyle style, int iconFrameId) {
    auto a = localAppearance();
    if (a.customIcon) a.iconId = iconFrameId > 0 ? iconFrameId : (style == PetStyle::StandardCube ? 2 : 0);
    return createPetSprite(style, a, localOwner());
}

int PetManager::getSelectedCubeIconId() {
    int savedIcon = Mod::get()->getSavedValue<int>("pet-custom-icon-id");
    if (savedIcon > 0) return savedIcon;
    return GameManager::sharedState()->m_playerFrame;
}

void PetManager::setSelectedCubeIconId(int iconFrameId) {
    if (iconFrameId > 0) {
        Mod::get()->setSavedValue("pet-custom-icon-id", iconFrameId);
    } else {
        Mod::get()->setSavedValue("pet-custom-icon-id", -1);
    }
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
    if (rarity > 4) rarityText = fmt::format("Mythic {}", romanNumeral(rarity - 4));

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
    const int TOTAL_COST = 20000;
    
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
    data.gp = json["gp"].asInt().unwrapOr(0);
    data.petLevel = json["petLevel"].asInt().unwrapOrDefault();
    Mod::get()->setSavedValue("last-pet-lvl", data.petLevel);
    data.petRarity = json["petRarity"].asInt().unwrapOrDefault();
    data.isBanned = json["isBanned"].asBool().unwrapOrDefault();
    data.mood = static_cast<int>(jsonUnwrapped["mood"].asInt().unwrapOr(1));
    data.streak = static_cast<int>(jsonUnwrapped["streak"].asInt().unwrapOr(0));
    data.feedCost = static_cast<int>(jsonUnwrapped["feedCost"].asInt().unwrapOr(25));
    if (jsonUnwrapped["appearance"].isObject()) {
        storeLocalAppearance(appearanceFromJson(jsonUnwrapped["appearance"]));
    } else if (hasLocalCustomization()) {
        this->pushAppearance();
    }
    if (data.isBanned) {
        data.banReason = json["banReason"].asString().unwrapOrDefault();
        Notification::create(fmt::format("Banned: {}", data.banReason), NotificationIcon::Error)->show();
        log::info("pet is banned: {}", data.banReason);
    }

    return data;
}
    
}