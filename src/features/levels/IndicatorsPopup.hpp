#include <Geode/Geode.hpp>
#include "../../shared/BasePopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class IndicatorsPopup : public BasePopup {
public:
    static IndicatorsPopup* create(int levelID);

    int m_levelID;

    LoadingSpinner* m_loadingSpinner = nullptr;
    LoadingSpinner* m_statusSpinner = nullptr;
    CCMenu* m_indicatorsMenu = nullptr;
    CCMenu* m_statusMenu = nullptr;

    NineSlice* m_menuBG = nullptr;

private:
    bool init(int levelID);

    arc::Future<> onLoadIndicators(int levelID);
};

}