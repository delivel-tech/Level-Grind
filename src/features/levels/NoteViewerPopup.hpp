#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../../shared/BasePopup.hpp"
#include "LevelTypes.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/MDTextArea.hpp"

using namespace geode::prelude;

namespace levelgrind {

class NoteViewerPopup : public BasePopup {
public:
    static NoteViewerPopup* create(std::vector<NoteInfo> notes);

    std::vector<NoteInfo> m_notes;
    int m_currentPage = 0;

    CCMenuItemSpriteExtra* m_arrowLeft = nullptr;
    CCMenuItemSpriteExtra* m_arrowRight = nullptr;

    MDTextArea* m_currentTextArea = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCMenu* m_addedByMenu = nullptr;

private:
    bool init(std::vector<NoteInfo> notes);
    void addTop();

    void turnPage(int dir);
    void updatePage();
    void cleanUp();
};

}