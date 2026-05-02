#include <Geode/Geode.hpp>
#include <Geode/binding/LevelCell.hpp>
#include "../BasePopup.hpp"
#include "../../utils/globals.hpp"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"

using namespace geode::prelude;

namespace levelgrind {

class EventPopup : public BasePopup {
public:
    static EventPopup* create(EventType type);
    
    EventType m_type;

    CCSprite* m_title = nullptr;
    CCLayer* m_levelContainer = nullptr;
    LevelCell* m_levelCell = nullptr;

private:
    bool init(EventType type);
};
    
}