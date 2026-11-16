#pragma once
#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class EventsManagePopup : public BasePopup {
public:
    static EventsManagePopup* create();

private:
    bool init() override;

    void addEventCard(int mode, float y);
    void onQueue(int mode);
    void loadDates();

    struct Card {
        TextInput* classic = nullptr;
        TextInput* plat = nullptr;
        CCLabelBMFont* dateLabel = nullptr;
    };
    Card m_cards[3];
    bool m_queueing = false;

    TaskHolder<web::WebResponse> m_listener;
    TaskHolder<web::WebResponse> m_datesListener;

    ~EventsManagePopup() {
        m_listener.cancel();
        m_datesListener.cancel();
    }
};

}