#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>

#include "../custom/PetLayer.hpp"

using namespace geode::prelude;

class LGPetUpgradePopup : public Popup, public ::UploadPopupDelegate {
public:
    static LGPetUpgradePopup* create(PetLayer::PetData petData);

    void onClosePopup(::UploadActionPopup* popup) override;

    PetLayer::PetData m_petData;

    int getUpgradeRarityCostByCurrRarity(int rarity);
    int getUpgradeLvlCostByLevel(int petLevel);

private:
    bool init(PetLayer::PetData petData);

    async::TaskHolder<web::WebResponse> m_listener;

    ~LGPetUpgradePopup() {
        m_listener.cancel();
    }

    void onUpgradeRarity(CCObject* sender);
    void onUpgradeLevel(CCObject* sender);

};