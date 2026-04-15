#include "SettingsLayer.hpp"
#include "Geode/ui/General.hpp"

using namespace geode::prelude;

namespace levelgrind {

SettingsLayer* SettingsLayer::create() {
    auto ret = new SettingsLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SettingsLayer::init() {
    if (!BaseLayer::init()) return false;

    m_bg->removeFromParent();

    auto newBg = createLayerBG();
    newBg->setColor({ 0, 102, 255 });
    addChild(newBg, -1);

    return true;
}

}