#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/LevelCell.hpp>
#include "../BasePopup.hpp"
#include "../../utils/globals.hpp"
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
    CCLayer* m_levelContainer = nullptr;
    LevelCell* m_levelCell = nullptr;

    CCLabelBMFont* m_timer = nullptr;
    NineSlice* m_eventContainer = nullptr;
    LevelCell* m_classicCell = nullptr;
    LevelCell* m_platCell = nullptr;
    LoadingSpinner* m_spinner = nullptr;

    CCMenuItemSpriteExtra* m_arrowLeft = nullptr;
    CCMenuItemSpriteExtra* m_arrowRight = nullptr;

private:
    bool init(EventType type);
    void update(float dt) override;

    std::string m_pendingKey;
    int m_classicLevelId = -1;
    int m_platLevelId = -1;
    double m_secondsLeft = 0.f;

    int m_pendingClassicId = -1;
    int m_pendingPlatId = -1;

    TaskHolder<web::WebResponse> m_listener;

    ~EventPopup() {
        m_listener.cancel();
    }
};
    
}