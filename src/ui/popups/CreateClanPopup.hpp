#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <cue/RadioLogic.hpp>
#include "../BasePopup.hpp"
#include "../../utils/globals.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"
#include "ccTypes.h"

using namespace geode::prelude;

namespace levelgrind {

class CreateClanPopup : public BasePopup {
public:
    static CreateClanPopup* create(std::function<void()> onCreated);

private:
    bool init(std::function<void()> onCreated);

    void onCreate();

    std::function<void()> m_onCreated;

    TextInput* m_nameInput = nullptr;
    TextInput* m_descInput = nullptr;
    TextInput* m_statReqInput = nullptr;

    cue::RadioLogic<LGClanType> m_typeRadio;
    cue::RadioLogic<LGClanJoinType> m_joinRadio;

    ccColor3B m_color = {255, 255, 255};

    TaskHolder<web::WebResponse> m_listener;

    ~CreateClanPopup() { m_listener.cancel(); }
};

}
