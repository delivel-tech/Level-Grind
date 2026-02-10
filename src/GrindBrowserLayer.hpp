#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>

using namespace geode::prelude;

class GrindBrowserLayer : public LevelBrowserLayer {
public:
    static GrindBrowserLayer* create(const char* title, GJSearchObject* search);
    static const char* browserTitle;
};