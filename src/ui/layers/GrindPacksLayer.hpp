#pragma once

#include <Geode/Geode.hpp>
#include "../BaseLayer.hpp"
#include "../../features/levels/LevelTypes.hpp"
#include "Geode/ui/ScrollLayer.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>
#include <cue/ListNode.hpp>
#include <Geode/binding/SetIDPopup.hpp>

using namespace geode::prelude;

namespace levelgrind {

class GrindPacksLayer : public BaseLayer, public SetIDPopupDelegate {
public:
    static GrindPacksLayer* create();

private:
    bool init() override;

    void onNextPage(CCObject* sender);
    void onPrevPage(CCObject* sender);
    void onPageButton(CCObject* sender);
    void onRefresh(CCObject* sender);

    void loadPage();
    arc::Future<> onLoadPacks();
    void startLoading();
    void stopLoading();
    void hideUIElements();
    void showUIElements();
    void updatePageButton();
    void populatePacks();

    void setIDPopupClosed(SetIDPopup* popup, int value) override;

    cue::ListNode* m_listNode = nullptr;
    ScrollLayer* m_scrollLayer = nullptr;

    CCMenuItemSpriteExtra* m_prevButton = nullptr;
    CCMenuItemSpriteExtra* m_nextButton = nullptr;
    CCMenuItemSpriteExtra* m_pageButton = nullptr;
    CCMenuItemSpriteExtra* m_refreshBtn = nullptr;
    CCLabelBMFont* m_pageButtonLabel = nullptr;
    CCLabelBMFont* m_packsLabel = nullptr;
    CCNode* m_circle = nullptr;

    std::vector<GrindPack> m_allPacks;
    int m_page = 0;
    int m_totalPages = 1;
    bool m_loading = false;
};

}