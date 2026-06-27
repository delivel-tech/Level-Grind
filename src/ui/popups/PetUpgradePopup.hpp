#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "../BasePopup.hpp"
#include <Geode/Geode.hpp>

#include "../../managers/PetManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PetUpgradePopup : public BasePopup, public ::UploadPopupDelegate {
public:
    static PetUpgradePopup* create(PetManager::PetData petData);

    void onClosePopup(::UploadActionPopup* popup) override;

    PetManager::PetData m_petData;

    int getUpgradeRarityCostByCurrRarity(int rarity);
    int getUpgradeLvlCostByLevel(int petLevel);

private:
    bool init(PetManager::PetData petData);

    TaskHolder<web::WebResponse> m_listener;

    ~PetUpgradePopup() {
        m_listener.cancel();
    }

    void onUpgradeRarity(CCObject* sender);
    void onUpgradeLevel(CCObject* sender);

};

}