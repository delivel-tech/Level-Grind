#include <Geode/Geode.hpp>
#include "../custom/LevelGrindLayer.hpp"

using namespace geode::prelude;

class LGDiffSelector : public Popup {
public:
    static LGDiffSelector* create(LevelGrindLayer* currentLayer);

private:
    bool init(LevelGrindLayer* currentLayer);

    void onFourBtn(CCObject* sender);
    void onFiveBtn(CCObject* sender);

    void onSixBtn(CCObject* sender);
    void onSevenBtn(CCObject* sender);

    void onEightBtn(CCObject* sender);
    void onNineBtn(CCObject* sender);

    LevelGrindLayer* m_currentLayer;
};