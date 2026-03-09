#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class BaseLayer : public CCLayer {
public:
    static BaseLayer* create();
    bool init() override;

private:
    void keyBackClicked() override;
    void onBack(CCObject* sender);
};