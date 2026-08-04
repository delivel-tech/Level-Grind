#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "../../features/levels/LevelTypes.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class AddPackPopup : public BasePopup {
public:
    static AddPackPopup* create();

private:
    bool init() override;

    arc::Future<> onAddClicked(NewGrindPackBody body);

    bool m_star = true;
    cocos2d::ccColor3B m_color;
};

}