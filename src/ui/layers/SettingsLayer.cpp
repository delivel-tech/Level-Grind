#include "SettingsLayer.hpp"
#include "cue/ListNode.hpp"

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

    replaceBgToClassic();

    for (size_t i = 0; i < 4; i++) {
        
    }

    return true;
}

}