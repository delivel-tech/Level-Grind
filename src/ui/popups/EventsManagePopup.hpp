#pragma once
#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class EventsManagePopup : public BasePopup {
public:
    static EventsManagePopup* create();

private:
    bool init() override;

    TaskHolder<web::WebResponse> m_listener;
};

}