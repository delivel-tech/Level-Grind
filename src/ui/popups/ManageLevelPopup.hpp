#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "../BasePopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

#include "../../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ManageLevelPopup : public BasePopup {
public:
    static ManageLevelPopup* create(GJGameLevel* level);

private:
    bool init(GJGameLevel* level);
    void formBody();

    ManageLevelBody m_body;

    LoadingSpinner* m_loadingSpinner = nullptr;
    GJGameLevel* m_level = nullptr;

    CCMenu* m_adminButtonsMenu = nullptr;
    CCMenu* m_helperButtonsMenu = nullptr;
    CCMenu* m_levelInfoMenu = nullptr;

    TaskHolder<web::WebResponse> m_listener;

    ~ManageLevelPopup() {
        m_listener.cancel();
    }
};

}