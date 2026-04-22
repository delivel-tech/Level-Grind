#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class WeeklyAchievementPopup : public Popup {
public:
    static WeeklyAchievementPopup* create();

private:
    bool init() override;
};

}