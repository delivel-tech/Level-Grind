#pragma once
#include "../BaseLayer.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/Geode.hpp>
#include "../../features/clans/ClanTypes.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class ClanViewerLayer : public BaseLayer {
public:
    static ClanViewerLayer* create();

private:
    bool init() override;

    arc::Future<> onLoadClans(Ref<LoadingSpinner> loadingRef, Ref<geode::NineSlice> mainPanelRef);

    void initUI(ViewClansResponse res, Ref<geode::NineSlice> mainPanel);
    void initClanUI(ViewClansResponse res, Ref<geode::NineSlice> mainPanel);

    enum MusicMode {
        In, Out
    };
    void changeMusic(MusicMode mode);

    void onBack(CCObject* sender) override;

    float m_prevMusicVolume = 1.f;
};

}