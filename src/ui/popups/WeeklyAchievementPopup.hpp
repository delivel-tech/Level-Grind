#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementPopup : public BasePopup {
public:
    static WeeklyAchievementPopup* create();

private:
    bool init() override;

    TaskHolder<web::WebResponse> m_listener;

    ~WeeklyAchievementPopup() {
        m_listener.cancel();
    }
};

}