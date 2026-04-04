#pragma once
#include "BaseLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "PetLayer.hpp"
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include <string>

using namespace geode::prelude;

class PetShopLayer : public BaseLayer {
public:
    static PetShopLayer* create(PetLayer::PetData petData);
    void open();

    PetLayer::PetData m_petData;

    struct AccessoryInfo {
        int aid;
        int originalCost;
        int discountPercentage;
        PetLayer::PetStyle currPetStyle;
        std::string accessoryName;
        std::string fileName;
        bool isBought;
        CCSize aPosition;
        float aScale;
    };

    std::vector<AccessoryInfo> m_accesories;
    AccessoryInfo* m_a1 = nullptr;
    AccessoryInfo* m_a2 = nullptr;
    AccessoryInfo* m_a3 = nullptr;

    cue::ListNode* m_list = nullptr;

    static int getPriceWithDiscount(AccessoryInfo* accessory);

private:
    bool init(PetLayer::PetData petData);
    bool initAccessoriesInfo(PetLayer::PetData petData);
    
    bool initAccessories();

    CCMenu* makeHeader(const char* title);
    CCMenu* makeAccessoryCell(AccessoryInfo* info);

    int getDiscountByRarity(int rarity);

    void onBuy1Btn(CCObject* sender);
    void onBuy2Btn(CCObject* sender);
    void onBuy3Btn(CCObject* sender);

    static constexpr CCSize CELL_SIZE = {356.f, 28.f};
    static constexpr CCSize ACCESSORY_CELL_SIZE = { 356.f, 56.f };
};