#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class SyncLevelsPopup : public BasePopup {
public:
    static SyncLevelsPopup* create();

private:
    enum Threshold {
        Add = 0,
        Delete,
        Lock,
        CoinAdd,
        CoinDelete,
        ThresholdCount
    };

    bool init() override;

    TextInput* addThresholdRow(CCNode* panel, float y, Threshold which);
    void onSync();
    void runSync(std::vector<int> values);

    TextInput* m_inputs[ThresholdCount] = {};
    bool m_syncing = false;

    TaskHolder<web::WebResponse> m_listener;

    ~SyncLevelsPopup() {
        m_listener.cancel();
    }
};

}