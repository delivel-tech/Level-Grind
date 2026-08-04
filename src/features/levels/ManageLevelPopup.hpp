#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "../../shared/BasePopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/utils/async.hpp"
#include <arc/future/Future.hpp>

#include "LevelTypes.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ManageLevelPopup : public BasePopup {
public:
    static ManageLevelPopup* create(GJGameLevel* level);

private:
    bool init(GJGameLevel* level);
    void formBody();

    arc::Future<> onLoadLevelInfo();
    arc::Future<> onAcceptClicked();
    arc::Future<> onRejectClicked();
    arc::Future<> onDeleteNotesClicked();
    arc::Future<> onUnlockClicked();
    arc::Future<> onLockClicked();
    arc::Future<> onDeleteClicked();
    arc::Future<> onAddOrReaddClicked(bool isReadd);
    arc::Future<> onCancelVoteClicked();

    ManageLevelBody m_body;

    LoadingSpinner* m_loadingSpinner = nullptr;
    GJGameLevel* m_level = nullptr;

    CCMenu* m_adminButtonsMenu = nullptr;
    CCMenu* m_helperButtonsMenu = nullptr;
    CCMenu* m_levelInfoMenu = nullptr;
};

}