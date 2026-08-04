#pragma once
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>
#include "../../shared/BasePopup.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class CreditsPopup : public BasePopup {
public:
    static CreditsPopup* create();

protected:
    bool init() override;

    arc::Future<> onLoadCredits(Ref<LoadingSpinner> loadingRef);

    cue::ListNode* m_list = nullptr;
};

}