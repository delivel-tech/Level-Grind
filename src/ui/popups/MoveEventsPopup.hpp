#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <cue/RadioLogic.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"
#include "../../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class MoveEventsPopup : public BasePopup {
public:
    static MoveEventsPopup* create(geode::Function<void()> onMoved);

private:
    bool init(geode::Function<void()> onMoved);

    TextInput* addStepper(CCNode* panel, CCMenu* menu, const char* label, float y, int initial, int min, int max);
    void onMove();
    void runMove(int mode, int days, int hours, int sinceDays);
    static std::string summary(MoveEventsResponse const& res, int days, int hours);

    geode::Function<void()> m_onMoved;
    cue::RadioLogic<int> m_modeRadio;
    cue::RadioLogic<int> m_dirRadio;
    TextInput* m_daysInput = nullptr;
    TextInput* m_hoursInput = nullptr;
    TextInput* m_sinceInput = nullptr;
    bool m_busy = false;

    TaskHolder<web::WebResponse> m_listener;

    ~MoveEventsPopup() {
        m_listener.cancel();
    }
};

}