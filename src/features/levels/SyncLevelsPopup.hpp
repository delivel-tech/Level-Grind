#include <Geode/Geode.hpp>
#include "../../shared/BasePopup.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class SyncLevelsPopup : public BasePopup {
public:
    static SyncLevelsPopup* create();

private:
    bool init() override;

    arc::Future<> onSyncClicked(int addThreshold, int deleteThreshold, int coinAddThreshold, int coinDeleteThreshold);
};

}