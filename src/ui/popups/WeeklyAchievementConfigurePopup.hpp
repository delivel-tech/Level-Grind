#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementConfigurePopup : public BasePopup {
public:
    static WeeklyAchievementConfigurePopup* create(GJUserScore* userScore);

    GJUserScore* m_userScore = nullptr;
    bool m_triedSetting = false;

private:
    bool init(GJUserScore* userScore);

    TaskHolder<web::WebResponse> m_listener;

    ~WeeklyAchievementConfigurePopup() {
        m_listener.cancel();
    }
};

}