#pragma once
#include "../BaseLayer.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include "../../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ClanViewerLayer : public BaseLayer {
public:
    static ClanViewerLayer* create();

private:
    bool init() override;

    void initUI(ViewClansResponse res, Ref<geode::NineSlice> mainPanel);
    void initClanUI(ViewClansResponse res, Ref<geode::NineSlice> mainPanel);

    enum MusicMode {
        In, Out
    };
    void changeMusic(MusicMode mode);

    void onBack(CCObject* sender) override;

    float m_prevMusicVolume = 1.f;

    TaskHolder<web::WebResponse> m_listener;
    ~ClanViewerLayer() {m_listener.cancel();}
};

}