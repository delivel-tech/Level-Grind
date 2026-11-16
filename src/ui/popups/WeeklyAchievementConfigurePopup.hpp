#include <Geode/Geode.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchCandidateCell;

class WeeklyAchievementConfigurePopup : public BasePopup {
public:
    static WeeklyAchievementConfigurePopup* create(GJUserScore* userScore);

    GJUserScore* m_userScore = nullptr;

private:
    bool init(GJUserScore* userScore);

    void onSet();
    void sendSet();

    WeeklyAchCandidateCell* m_cells[3] = {};
    bool m_setting = false;

    TaskHolder<web::WebResponse> m_listener;

    ~WeeklyAchievementConfigurePopup() {
        m_listener.cancel();
    }
};

}