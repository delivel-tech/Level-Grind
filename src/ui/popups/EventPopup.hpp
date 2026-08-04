#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../BasePopup.hpp"
#include "../../features/events/EventTypes.hpp"
#include "../components/EventLevelCell.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class EventPopup : public BasePopup {
public:
    static EventPopup* create(EventType type);

    EventType m_type;

    CCSprite* m_title = nullptr;

    CCLabelBMFont* m_timer = nullptr;
    NineSlice* m_eventContainer = nullptr;
    EventLevelCell* m_classicCell = nullptr;
    EventLevelCell* m_platCell = nullptr;
    LoadingSpinner* m_spinner = nullptr;

    CCMenuItemSpriteExtra* m_arrowLeft = nullptr;
    CCMenuItemSpriteExtra* m_arrowRight = nullptr;

private:
    bool init(EventType type);
    void update(float dt) override;

    double m_secondsLeft = 0.f;

    TaskHolder<web::WebResponse> m_listener;

    ~EventPopup() {
        m_listener.cancel();
    }
};

}