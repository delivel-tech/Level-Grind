#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/platform/windows.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>

using namespace geode::prelude;

#define makeCb(func) menu_selector(func)

namespace levelgrind {

template <typename T>
void safeErase(std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end()) {
        vec.erase(it);
    }
}

template <typename T>
void safeAdd(std::vector<T>& vec, const T& value) {
    if (std::find(vec.begin(), vec.end(), value) == vec.end()) {
        vec.push_back(value);
    }
}

template <typename T>
inline void inlineSafeAdd(std::vector<T>& vec, const T& value) {
    if (std::find(vec.begin(), vec.end(), value) == vec.end()) {
        vec.push_back(value);
    }
}

inline bool getNewTogglerState(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return false;

    return !toggler->isToggled();
}

template <typename T>
void processValueOnToggler(CCObject* sender, std::vector<T>& vec, const T& value) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        safeAdd(vec, value);
    } else {
        safeErase(vec, value);
    }
}

}