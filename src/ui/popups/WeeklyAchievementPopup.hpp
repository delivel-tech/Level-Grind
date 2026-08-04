#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementPopup : public BasePopup {
public:
    static WeeklyAchievementPopup* create();

private:
    bool init() override;

    arc::Future<> onLoadWeeklyAch();
};

}