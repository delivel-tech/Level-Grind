#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include "BaseLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

class PetLayer : public BaseLayer {
public:
    static PetLayer* create(web::WebResponse response);
    std::string m_petName = "Grinding Pet";
    CCLabelBMFont* m_petLabel;
    static void goToPetLayer(web::WebResponse response);

    struct PetData {
        std::string petName;
        int petStars;
        int petMoons;
        int petLevel;
        int petRarity;
        bool a1bought;
        bool a2bought;
        bool a3bought;
        bool isBanned;
        std::string banReason;
    };

    enum PetStyle {
        StandardCube,
        OwnCube,
        OwnCubeWithColors
    };

    PetData m_petData;

    static PetStyle getStyleByLevel(int petLevel);
    static CCSprite* getPetSprByStyle(PetStyle style);
    int getUpgradeLvlCostByLevel(int petLevel);
    float getNextLevelPercentage(int petStars, int nextLvlCost);
    std::string getRarityFromInt(int rarity);
    std::string getPetAgeFromLevel(int petLevel);
    float getPetScale(int level);
    int getUpgradeRarityCostByCurrRarity(int rarity);

    bool maxLevel(int petLevel);

    void drawPet(PetStyle style, int petLevel);

    bool isLvlUpgraded = false;
    bool isRarityUpgraded = false;

private:
    bool init(web::WebResponse response);
    void onBack(CCObject* sender) override;
    void onSettingsBtn(CCObject* sender);
    void onInfoBtn(CCObject* sender);
    void onRenameBtn(CCObject* sender);
    void onUpgradeBtn(CCObject* sender);
    void onShopBtn(CCObject* sender);
    void onChooseABtn(CCObject* sender);

    PetData parsePetData(web::WebResponse res);
};