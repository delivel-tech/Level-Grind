#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LGLevelNotePopup : public Popup {
public:
    static LGLevelNotePopup* create(std::string const& levelName, std::string const& levelNote);

private:
    bool init(std::string const& levelName, std::string const& levelNote);
};
