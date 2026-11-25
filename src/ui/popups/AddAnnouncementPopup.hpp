#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../BasePopup.hpp"

#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"


using namespace geode::prelude;

namespace levelgrind {

class AddAnnouncementPopup : public BasePopup {
public:
    static AddAnnouncementPopup* create();

private:
    bool init() override;
    void onPreview();
    void onPost();
    void saveDraft();
    void loadDraft();
    TextInput* m_titleTextInput = nullptr;
    TextInput* m_body = nullptr;
    bool m_posting = false;

    TaskHolder<web::WebResponse> m_listener;

    ~AddAnnouncementPopup() {
        m_listener.cancel();
    }
};

}
