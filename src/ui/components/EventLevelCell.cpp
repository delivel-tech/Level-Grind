#include "EventLevelCell.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include "Geode/platform/windows.hpp"
#include "Geode/ui/Notification.hpp"
#include <UIBuilder.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

EventLevelCell* EventLevelCell::create() {
    auto ret = new EventLevelCell;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

EventLevelCell::~EventLevelCell() {}

bool EventLevelCell::init() {
    if (!CCLayer::init()) return false;

    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint({ 0.f, 0.f });
    this->setContentSize({ CELL_WIDTH, CELL_HEIGHT });
    this->scheduleUpdate();

    m_loadedContainer = Build(CCNode::create())
        .anchorPoint({ 0.f, 0.f })
        .contentSize({ CELL_WIDTH, CELL_HEIGHT })
        .visible(false)
        .id("loaded-container")
        .parent(this)
        .collect();

    m_loadingCircle = Build(cue::LoadingCircle::create(true))
        .id("loading-circle")
        .collect();
    m_loadingCircle->addToLayer(this);

    return true;
}

void EventLevelCell::showLoading() {
    if (m_errorLabel) {
        m_errorLabel->removeFromParent();
        m_errorLabel = nullptr;
    }
    if (m_loadingCircle) m_loadingCircle->fadeIn();
    if (m_loadedContainer) m_loadedContainer->setVisible(false);
}

void EventLevelCell::hideLoading() {
    if (m_loadingCircle) m_loadingCircle->fadeOut();
}

void EventLevelCell::removeLoadedElements() {
    if (m_loadedContainer) {
        m_loadedContainer->removeAllChildrenWithCleanup(true);
        m_loadedContainer->setVisible(false);
    }
    if (m_errorLabel) {
        m_errorLabel->removeFromParent();
        m_errorLabel = nullptr;
    }
    m_levelCell = nullptr;
    m_level = nullptr;
}

void EventLevelCell::createCell() {
    m_levelCell = LevelCell::create(CELL_WIDTH, CELL_HEIGHT);
    m_levelCell->setAnchorPoint({ 0.f, 0.f });
    m_levelCell->setPosition({ 0.f, 0.f });
    m_levelCell->setContentSize({ CELL_WIDTH, CELL_HEIGHT });
    m_levelCell->setID("level-cell");
    m_levelCell->loadFromLevel(m_level);
    m_loadedContainer->addChild(m_levelCell);

    fixLevelCell(m_levelCell);

    auto cvoltonID = m_levelCell->m_mainLayer->getChildByIDRecursive("cvolton.betterinfo/level-id-label");
    if (cvoltonID != nullptr) {
        cvoltonID->setVisible(false);
    }

    m_loadedContainer->setVisible(true);
}

void EventLevelCell::fixLevelCell(LevelCell* cell) {
    if (!cell || !cell->m_mainMenu) return;

    cell->m_mainMenu->setPosition({ 0, 0 });

    auto viewButton    = cell->m_mainMenu->getChildByID("view-button");
    auto creatorButton = cell->m_mainMenu->getChildByID("creator-name");

    if (auto levelName = cell->m_mainLayer->getChildByID("level-name")) levelName->setPosition({50, 88});
    if (auto songName = cell->m_mainLayer->getChildByID("song-name")) songName->setPosition({52, 46});
    if (auto creatorName = cell->m_mainMenu->getChildByID("creator-name")) creatorName->setPosition({50, 65});

    if (creatorButton) {
        creatorButton->setPosition({ 50, 65 });
        creatorButton->setAnchorPoint({ 0.f, 0.5f });
    }

    if (viewButton) {
        if (auto castedViewBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(viewButton)) {
            auto spr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
            spr->setScale(0.7f);
            castedViewBtn->setSprite(spr);
        }
        viewButton->setPosition({
            cell->getContentSize().width - 40,
            cell->getContentSize().height / 2.f
        });
    }

    std::vector<std::string> ids {
        "length-icon", "downloads-icon", "likes-icon", "length-label", "downloads-label", "likes-label",
        "orbs-icon", "orbs-label"
    };

    for (const auto& id : ids) {
        if (auto element = cell->m_mainLayer->getChildByID(id)) {
            element->setPositionY(26);
        }
    }

    auto children = this->m_levelCell->getChildren();

    for (const auto& child : CCArrayExt(children)) {
        typeinfo_cast<CCNode*>(child)->setPositionY(
            typeinfo_cast<CCNode*>(child)->getPositionY() - 3
        );
    }
}

void EventLevelCell::loadLevel(int levelID) {
    if (levelID <= 0) return;

    removeLoadedElements();
    showLoading();

    auto searchObj = GJSearchObject::create(SearchType::Search, numToString(levelID));
    m_pendingKey = std::string(searchObj->getKey());
    m_pendingLevelID = levelID;
    m_pendingElapsed = 0.f;

    auto glm = GameLevelManager::sharedState();
    if (!glm) return;

    auto stored = glm->getStoredOnlineLevels(m_pendingKey.c_str());
    if (stored && stored->count() > 0) {
        auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
        if (lvl && lvl->m_levelID == levelID) {
            m_pendingKey.clear();
            m_pendingLevelID = -1;
            levelLoaded(Ok(lvl));
            return;
        }
    }

    glm->getOnlineLevels(searchObj);
}

void EventLevelCell::reload(bool fromFullReload) {
    int id = m_level ? m_level->m_levelID : m_pendingLevelID;
    if (id <= 0) return;

    if (!fromFullReload) showLoading();
    loadLevel(id);
}

void EventLevelCell::reloadFull() {
    reload(true);
}

void EventLevelCell::update(float dt) {
    if (m_levelCell) {
        if (auto separator = m_levelCell->getChildByIDRecursive("cdc.level_thumbnails/separator")) separator->setPosition({262.8f, 0.3f});
        if (auto clNode = m_levelCell->getChildByIDRecursive("cdc.level_thumbnails/clipping-node")) clNode->setPosition({386, 0.3f});

        if (auto levelTagsMenu = m_levelCell->m_mainLayer->getChildByID("level-tags")) {
            if (levelTagsMenu) levelTagsMenu->removeFromParent();
            fixLevelCell(m_levelCell);
        }
    }

    if (m_pendingKey.empty()) return;

    m_pendingElapsed += dt;

    auto glm = GameLevelManager::sharedState();
    if (!glm) return;

    auto stored = glm->getStoredOnlineLevels(m_pendingKey.c_str());
    if (stored && stored->count() > 0) {
        for (unsigned int i = 0; i < stored->count(); i++) {
            auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
            if (lvl && lvl->m_levelID == m_pendingLevelID) {
                m_pendingKey.clear();
                m_pendingLevelID = -1;
                levelLoaded(Ok(lvl));
                return;
            }
        }
    }

    if (m_pendingElapsed >= LOAD_TIMEOUT) {
        int failedId = m_pendingLevelID;
        m_pendingKey.clear();
        m_pendingLevelID = -1;
        levelLoaded(Err(failedId));
    }
}

void EventLevelCell::levelLoaded(Result<GJGameLevel*, int> result) {
    hideLoading();

    if (result.isErr()) {
        Notification::create(
            fmt::format("Failed to load level (id {})", result.unwrapErr()),
            NotificationIcon::Error
        )->show();

        m_errorLabel = Build(CCLabelBMFont::create("Failed to load level", "bigFont.fnt"))
            .scale(0.5f)
            .pos({ CELL_WIDTH / 2.f, CELL_HEIGHT / 2.f })
            .id("error-label")
            .parent(this)
            .collect();
        return;
    }

    m_level = result.unwrap();
    createCell();
}

}
