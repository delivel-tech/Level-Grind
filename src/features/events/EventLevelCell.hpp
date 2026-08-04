#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <cue/LoadingCircle.hpp>

using namespace geode::prelude;

namespace levelgrind {

class EventLevelCell : public CCLayer {
public:
    static EventLevelCell* create();
    ~EventLevelCell();

    void loadLevel(int levelID);
    void reloadFull();
    void reload(bool fromFullReload = false);

protected:
    static constexpr float CELL_WIDTH = 380.f;
    static constexpr float CELL_HEIGHT = 116.f;
    static constexpr float LOAD_TIMEOUT = 10.f;

    bool init() override;
    void update(float dt) override;
    void createCell();

    void showLoading();
    void hideLoading();
    void removeLoadedElements();

    void levelLoaded(Result<GJGameLevel*, int> result);
    void fixLevelCell(LevelCell* cell);

    Ref<CCNode> m_loadedContainer;
    Ref<cue::LoadingCircle> m_loadingCircle;
    Ref<LevelCell> m_levelCell;
    Ref<CCLabelBMFont> m_errorLabel;

    Ref<GJGameLevel> m_level;

    int m_pendingLevelID = -1;
    std::string m_pendingKey;
    float m_pendingElapsed = 0.f;
};

}
