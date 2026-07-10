#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"

using namespace geode::prelude;

namespace levelgrind {

class AddNotePopup : public BasePopup {
public:
    static AddNotePopup* create(int levelId, std::string levelName);

private:
    bool init(int levelId, std::string levelName);

    TextInput* m_input = nullptr;
    std::string m_levelName;
    int m_levelId;

    void onAddBtn(CCObject* sender);
    void onPreview(CCObject* sender);

protected:
    TaskHolder<web::WebResponse> m_listener;
    ~AddNotePopup() { m_listener.cancel(); }
};

}

