#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class CreditsPopup : public BasePopup {
public:
    static CreditsPopup* create();

protected:
    bool init() override;

    cue::ListNode* m_list = nullptr;

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;

    ~CreditsPopup() {
        m_listener.cancel();
    }
};

}