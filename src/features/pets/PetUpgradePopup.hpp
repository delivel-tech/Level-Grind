#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "../../shared/BasePopup.hpp"
#include <Geode/Geode.hpp>

#include "PetManager.hpp"
#include "PetTypes.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class PetUpgradePopup : public BasePopup, public ::UploadPopupDelegate {
public:
    static PetUpgradePopup* create(SyncPetResponse petData);

    void onClosePopup(::UploadActionPopup* popup) override;

    SyncPetResponse m_petData;

    int getUpgradeRarityCostByCurrRarity(int rarity);
    int getUpgradeLvlCostByLevel(int petLevel);

private:
    bool init(SyncPetResponse petData);

    void onUpgradeRarity(CCObject* sender);
    void onUpgradeLevel(CCObject* sender);

    arc::Future<> onUpgradeRarityClicked(int upgradeCost);
    arc::Future<> onUpgradeLevelClicked(int upgradeCost);
};

}