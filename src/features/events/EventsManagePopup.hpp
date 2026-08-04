#pragma once
#include <Geode/Geode.hpp>
#include "../../shared/BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

struct EventSection {
    int mode;
    std::string name;
    std::string menuId;
};

class EventsManagePopup : public BasePopup {
public:
    static EventsManagePopup* create();

private:
    bool init() override;

    arc::Future<> onQueueClicked(EventSection section, int classicId, int platId);
    arc::Future<> onLoadEventDates();
};

}