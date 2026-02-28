#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>

using namespace geode::prelude;

class LGLevelInfoLayer : public LevelInfoLayer {
public:
    static LGLevelInfoLayer* create(GJGameLevel *level, bool challenge);

    bool init(GJGameLevel* level, bool challenge);
};