#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LGAnnPopup : public Popup {
public:
    static LGAnnPopup* create();
    bool init() override;
};