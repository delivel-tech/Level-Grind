#pragma once

#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LGDiscordPopup : public Popup {
public:
    static LGDiscordPopup* create();

private:
    bool init() override;

    void closePopup(CCObject* sender);
    void onJoin(CCObject* sender);
};