#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class FindPlayerPopup : public BasePopup {
public:
    static FindPlayerPopup* create();

private:
    bool init();
    void find();

    TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    bool m_busy = false;
    TaskHolder<web::WebResponse> m_task;

    ~FindPlayerPopup() { m_task.cancel(); }
};

}
