#include "NoteViewerPopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDTextArea.hpp"

#include <Geode/binding/ProfilePage.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

namespace levelgrind {

NoteViewerPopup* NoteViewerPopup::create(std::vector<NoteInfo> notes) {
    auto ret = new NoteViewerPopup;
    if (ret && ret->init(notes)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NoteViewerPopup::init(std::vector<NoteInfo> notes) {
    if (!BasePopup::init({380, 260})) return false;

    m_notes = notes;

    addTop();

    m_arrowLeft = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .intoMenuItem([this] {this->turnPage(-1);})
        .tag(-1)
        .parent(m_buttonMenu)
        .id("arrow-left")
        .pos({
            -20, centerY()
        })
        .collect();

    m_arrowRight = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .flipX(true)
        .intoMenuItem([this] {this->turnPage(1);})
        .tag(1)
        .parent(m_buttonMenu)
        .id("arrow-right")
        .pos({
            m_buttonMenu->getContentWidth() + 20, centerY()
        })
        .collect();

    this->updatePage();

    return true;
}

void NoteViewerPopup::turnPage(int dir) {
    if (!m_arrowLeft || !m_arrowRight) return;

    m_currentPage += dir;

    updatePage();
}

void NoteViewerPopup::updatePage() {
    if (!m_arrowLeft || !m_arrowRight) return;
    if (m_notes.empty()) return;

    auto const& currentNote = m_notes.at(m_currentPage);

    this->cleanUp();

    m_currentTextArea = Build(MDTextArea::create(currentNote.note, {280, 150}))
        .parent(m_buttonMenu)
        .id("text-area")
        .pos({centerX(), centerY() - 20})
        .collect();

    m_pageLabel = Build(CCLabelBMFont::create(
        fmt::format("Note {} of {}", m_currentPage + 1, m_notes.size()).c_str(),
        "bigFont.fnt"
    ))
        .scale(0.4f)
        .parent(m_mainLayer)
        .id("page-label")
        .pos({centerX(), 15})
        .collect();

    m_addedByMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(2)->setAutoScale(false))
        .parent(m_buttonMenu)
        .id("added-by-menu")
        .pos({centerX(), 27})
        .collect();

    auto addedByLabel = Build(CCLabelBMFont::create("By: ", "goldFont.fnt"))
        .scale(0.5f)
        .parent(m_addedByMenu)
        .id("added-by-label")
        .collect();

    auto usernameBtn = Build(CCLabelBMFont::create(
        currentNote.senderUsername.c_str(), "goldFont.fnt"
    ))
        .scale(0.5f)
        .parent(m_addedByMenu)
        .id("username-btn")
        .collect();

    m_addedByMenu->updateLayout();

    if (!m_arrowLeft || !m_arrowRight) return;

    if (m_currentPage <= 0) {
        m_arrowLeft->setEnabled(false);
        m_arrowLeft->setColor({100, 100, 100});
    } else {
        m_arrowLeft->setEnabled(true);
        m_arrowLeft->setColor({255,255, 255});
    }

    if (m_currentPage >= m_notes.size() - 1) {
        m_arrowRight->setEnabled(false);
        m_arrowRight->setColor({100, 100, 100});
    } else {
        m_arrowRight->setEnabled(true);
        m_arrowRight->setColor({255,255, 255});
    }

    return;
}

void NoteViewerPopup::cleanUp() {
    if (m_currentTextArea) {
        m_currentTextArea->removeFromParent();
        m_currentTextArea = nullptr;
    }
    if (m_pageLabel) {
        m_pageLabel->removeFromParent();
        m_pageLabel = nullptr;
    }
    if (m_addedByMenu) {
        m_addedByMenu->removeAllChildrenWithCleanup(true);
        m_addedByMenu->removeFromParent();
        m_addedByMenu = nullptr;
    }
    return;
}

void NoteViewerPopup::addTop() {
    auto grindLogo = Build(CCSprite::create("lg-logo.png"_spr))
        .pos(
            m_mainLayer->getContentWidth() / 2.f,
            m_mainLayer->getContentHeight() / 1.2f + 17.f
        )
        .parent(m_mainLayer)
        .scale(0.8f)
        .id("grind-logo")
        .collect();

    auto viewerLabel = Build(CCLabelBMFont::create("Notes Viewer", "bigFont.fnt"))
        .pos(
            m_mainLayer->getContentWidth() / 2.f,
            m_mainLayer->getContentHeight() / 1.2f - 13.f
        )
        .parent(m_mainLayer)
        .scale(0.5f)
        .id("viewer-label")
        .collect();

    return;
}

}