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
    bool init() override;
    void drawUIFromData(PetManager::PetData data, NineSlice* mainPanel);
    void onBack(CCObject* sender) override;
    enum MusicMode {
        In, Out
    };
    void changeMusic(MusicMode mode);

    void drawPet(PetManager::PetStyle style, int petLevel);

    CCLabelBMFont* m_petLabel = nullptr;

    TaskHolder<web::WebResponse> m_listener;

    ~PetLayer() {m_listener.cancel();}
};

}