#pragma once

#include <Geode/Geode.hpp>
#include "../BaseLayer.hpp"
#include "../../managers/PetManager.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/NineSlice.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PetLayer : public BaseLayer {
public:
    static PetLayer* create();

private:
    enum MusicMode {
        In, Out
    };

    bool init() override;
    void onBack(CCObject* sender) override;
    void changeMusic(MusicMode mode);

    void loadData(bool firstLoad);
    void reload();
    void drawUIFromData(PetManager::PetData data, cocos2d::CCNode* mainPanel);
    void drawPet(PetManager::PetStyle style, int petLevel, int mood);
    void celebrate(bool rarity, PetManager::PetData const& data);
    void feed();

    int m_lastLevel = -1;
    int m_lastRarity = -1;
    bool m_feeding = false;

    NineSlice* m_mainPanel = nullptr;
    cocos2d::CCNode* m_content = nullptr;
    CCLabelBMFont* m_petLabel = nullptr;

    TaskHolder<web::WebResponse> m_feedTask;
    TaskHolder<web::WebResponse> m_listener;
};

}