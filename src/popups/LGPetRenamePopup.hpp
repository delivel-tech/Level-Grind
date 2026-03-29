#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/TextInput.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LGPetRenamePopup : public Popup {
public:
    static LGPetRenamePopup* create(std::string currPetName);

private:
    bool init(std::string currPetName);
    void onOKBtn(CCObject* sender);

    TextInput* m_input = nullptr;

protected:
    async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    ~LGPetRenamePopup() { m_listener.cancel(); }
};