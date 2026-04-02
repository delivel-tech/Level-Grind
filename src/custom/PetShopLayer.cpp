#include "PetShopLayer.hpp"
#include "BaseLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "PetLayer.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <cue/ListNode.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

PetShopLayer* PetShopLayer::create(PetLayer::PetData petData) {
    auto ret = new PetShopLayer;
    if (ret && ret->init(petData)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void PetShopLayer::open() {
    auto scene = CCScene::create();
    scene->addChild(this);

    auto transition = CCTransitionFade::create(0.5f, scene);

    CCDirector::sharedDirector()->pushScene(transition);
}

bool PetShopLayer::init(PetLayer::PetData petData) {
    if (!BaseLayer::init()) return false;
    if (!initAccessoriesInfo(petData)) return false;

    if (!Mod::get()->getSavedValue<bool>("disable-star-particles")) {
        auto grindParticles = CCParticleSnow::create();
        auto texture = CCTextureCache::sharedTextureCache()->addImage(
            "GJ_bigStar_noShadow.png"_spr, true);
        grindParticles->m_fStartSpin = 0.f;
        grindParticles->m_fEndSpin = 180.f;
        grindParticles->m_fStartSize = 6.f;
        grindParticles->m_fEndSize = 3.f;
        grindParticles->setTexture(texture);

        this->addChild(grindParticles);
    }

    if (!initAccessories()) return false;

    return true;
}

bool PetShopLayer::initAccessoriesInfo(PetLayer::PetData petData) {
    m_accesories.resize(3);

    for (auto& accesory : m_accesories) {
        accesory.currPetStyle = PetLayer::getStyleByLevel(petData.petLevel);
        accesory.discountPercentage = this->getDiscountByRarity(petData.petRarity);
    }

    m_a1 = &m_accesories[0];
    m_a2 = &m_accesories[1];
    m_a3 = &m_accesories[2];

    m_a1->isBought = petData.a1bought;
    m_a2->isBought = petData.a2bought;
    m_a3->isBought = petData.a3bought;

    m_a1->originalCost = 600;
    m_a2->originalCost = 1000;
    m_a3->originalCost = 2100;

    m_a1->aid = 1;
    m_a2->aid = 2;
    m_a3->aid = 3;

    m_a1->accessoryName = "Bunny Ears";
    m_a2->accessoryName = "Test 2";
    m_a3->accessoryName = "Test 3";

    return true;
}

CCMenu* PetShopLayer::makeHeader(const char* title) {
    auto headerCell = CCMenu::create();
    headerCell->ignoreAnchorPointForPosition(false);
    headerCell->setContentSize({ CELL_SIZE.width, 24.f });

    auto headerLabel = CCLabelBMFont::create(title, "goldFont.fnt");
    headerLabel->setScale(0.5f);
    headerLabel->setPosition({ CELL_SIZE.width / 2.f, 12.f });
    headerCell->addChild(headerLabel);

    return headerCell;
}

CCMenu* PetShopLayer::makeAccessoryCell(PetShopLayer::AccessoryInfo* info) {
    auto accessoryCell = CCMenu::create();
    accessoryCell->ignoreAnchorPointForPosition(false);
    accessoryCell->setContentSize(ACCESSORY_CELL_SIZE);

    auto petSpr = PetLayer::getPetSprByStyle(
        info->currPetStyle
    );

    accessoryCell->addChildAtPosition(
        petSpr, Anchor::Left, { 30.f, 0.f }
    );

    auto aNameLabel = CCLabelBMFont::create(info->accessoryName.c_str(), "goldFont.fnt");
    aNameLabel->setAnchorPoint({ 0.f, 0.5f });
    aNameLabel->setScale(0.8f);
    
    accessoryCell->addChildAtPosition(
        aNameLabel, Anchor::Left, { 55.f, 0.f }
    );

    auto getBuyFunByAID = [info]() {
        if (info->aid == 1) {
            return menu_selector(PetShopLayer::onBuy1Btn);
        } else if (info->aid == 2) {
            return menu_selector(PetShopLayer::onBuy2Btn);
        } else {
            return menu_selector(PetShopLayer::onBuy3Btn);
        }
    };

    if (!info->isBought) {
        auto buySpr = ButtonSprite::create("Buy");
        buySpr->setScale(0.7f);
        auto buyBtn = CCMenuItemSpriteExtra::create(
            buySpr,
            this,
            getBuyFunByAID()
        );
        buyBtn->setID(fmt::format("buy-btn-{}", info->aid));
        accessoryCell->addChildAtPosition(
            buyBtn, Anchor::Right, { -35.f, 0.f }
        );

        auto priceLabel = CCLabelBMFont::create(numToString(getPriceWithDiscount(info)).c_str(), "goldFont.fnt");
        priceLabel->setID(fmt::format("price-label-{}", info->aid));
        priceLabel->setAnchorPoint({ 1.f, 0.5f });
        priceLabel->setScale(0.7f);
        accessoryCell->addChildAtPosition(
            priceLabel, Anchor::Right, { -65.f, 0.f }
        );
    } else {
        auto boughtSpr = ButtonSprite::create("Bought", "goldFont.fnt", "GJ_button_02.png");
        boughtSpr->setScale(0.7f);
        auto boughtBtn = CCMenuItemSpriteExtra::create(
            boughtSpr,
            this,
            nullptr
        );
        boughtBtn->setID(fmt::format("bought-btn-{}", info->aid));
        boughtBtn->setEnabled(false);
        boughtBtn->setColor({
            128, 128, 128
        });
        accessoryCell->addChildAtPosition(
            boughtBtn, Anchor::Right, { -50.f, 0.f }
        );
    }

    return accessoryCell;
}

void PetShopLayer::onBuy1Btn(CCObject* sender) {

}

void PetShopLayer::onBuy2Btn(CCObject* sender) {
    
}

void PetShopLayer::onBuy3Btn(CCObject* sender) {
    
}

bool PetShopLayer::initAccessories() {
    m_list = cue::ListNode::create({ 356.f, 220.f }, cue::Brown, cue::ListBorderStyle::SlimLevels);
    m_list->setPosition(CCDirector::sharedDirector()->getWinSize() / 2.f);
    m_list->setAutoUpdate(true);

    this->addChild(m_list);

    m_list->addCell(makeHeader(
        "Buy Accessories"
    ));

    m_list->addCell(makeAccessoryCell(m_a1));
    m_list->addCell(makeAccessoryCell(m_a2));
    m_list->addCell(makeAccessoryCell(m_a3));

    m_list->updateLayout();

    return true;
}

int PetShopLayer::getDiscountByRarity(int rarity) {
    switch (rarity) {
        case 2: return 30; break;
        case 3: return 60; break;
        case 4: return 90; break;
        default: return 0;
    }
}

int PetShopLayer::getPriceWithDiscount(PetShopLayer::AccessoryInfo* accessory) {
    int newCost = int(accessory->originalCost - ((accessory->originalCost / 100) * accessory->discountPercentage));
    return newCost;
}