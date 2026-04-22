// admins / owners only Popup
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../BasePopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class AddAnnouncementPopup : public BasePopup {
public:
    static AddAnnouncementPopup* create();

private:
    bool init() override;

    TextInput* m_titleTextInput = nullptr;
    TextInput* m_contentTextInput = nullptr;
    CCMenuItemSpriteExtra* m_previewBtn = nullptr;
    CCMenuItemSpriteExtra* m_addBtn = nullptr;

    CCMenu* m_buttonsMenu = nullptr;

    TaskHolder<web::WebResponse> m_listener;

    ~AddAnnouncementPopup() {
        m_listener.cancel();
    }
};

}