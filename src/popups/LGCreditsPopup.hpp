#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LGCreditsPopup : public Popup {
public:
    static LGCreditsPopup* create();

private:
    ScrollLayer* m_scrollLayer = nullptr;
    LoadingSpinner* m_spinner = nullptr;

    void onInfo(CCObject* sender);
    void onHeaderInfo(CCObject* sender);
    void onAccountClicked(CCObject* sender);

    bool init() override;
};