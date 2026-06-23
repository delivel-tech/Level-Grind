#include "LGLevelNotePopup.hpp"
#include <Geode/ui/MDTextArea.hpp>

LGLevelNotePopup* LGLevelNotePopup::create(std::string const& levelName, std::string const& levelNote) {
    auto ret = new LGLevelNotePopup;
    if (ret && ret->init(levelName, levelNote)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGLevelNotePopup::init(std::string const& levelName, std::string const& levelNote) {
    if (!Popup::init(440.f, 250.f)) return false;

    this->setTitle(levelName);

    auto mdTextArea = MDTextArea::create(levelNote, { 400.f, 150.f });
    m_mainLayer->addChildAtPosition(mdTextArea, Anchor::Center, { 0.f, -10.f });

    return true;
}
