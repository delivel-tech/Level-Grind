#pragma once

#include <Geode/Geode.hpp>
#include "../../shared/BaseLayer.hpp"
#include "PetManager.hpp"
#include "PetTypes.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class PetLayer : public BaseLayer {
public:
    static PetLayer* create();

private:
    bool init() override;
    arc::Future<> onLoadPetData(Ref<LoadingSpinner> loadingRef, Ref<geode::NineSlice> mainPanelRef);
    void drawUIFromData(SyncPetResponse data, NineSlice* mainPanel);
    void onBack(CCObject* sender) override;
    enum MusicMode {
        In, Out
    };
    void changeMusic(MusicMode mode);

    void drawPet(PetManager::PetStyle style, int petLevel);

    CCLabelBMFont* m_petLabel = nullptr;
};

}