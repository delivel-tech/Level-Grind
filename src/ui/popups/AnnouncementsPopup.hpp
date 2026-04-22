#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/MDTextArea.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class AnnouncementsPopup : public BasePopup {
public:
    static AnnouncementsPopup* create();

private:
    bool init() override;

    TaskHolder<web::WebResponse> m_listener;

    LoadingSpinner* m_spinner = nullptr;

    std::vector<geode::MDTextArea*> m_textAreas;
    cue::ListNode* m_listNode = nullptr;;
    geode::Scrollbar* m_scrollBar = nullptr;

    std::vector<CCMenuItemSpriteExtra> m_deleteButtons;

    int m_currentAnnouncementID;
    int m_newestAnnouncementID;

    ~AnnouncementsPopup() {
        m_listener.cancel();
    }
};

}