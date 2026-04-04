#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>
#include "../custom/PetShopLayer.hpp"

using namespace geode::prelude;

class BuyAccessoryPopup : public Popup {
public:
    static BuyAccessoryPopup* create(PetShopLayer::AccessoryInfo* info, PetLayer::PetData petData);

    PetLayer::PetData m_petData;
    PetShopLayer::AccessoryInfo* m_accessoryInfo = nullptr;
    int m_price = 0;

private:
    bool init(PetShopLayer::AccessoryInfo* info, PetLayer::PetData petData);
    void onBuyBtn(CCObject* sender);

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;

    ~BuyAccessoryPopup() {
        m_listener.cancel();
    }
};