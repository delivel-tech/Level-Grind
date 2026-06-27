#pragma once

#include "Geode/ui/TextInput.hpp"
#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PetRenamePopup : public BasePopup {
public:
    static PetRenamePopup* create(std::string currPetName);

private:
    bool init(std::string currPetName);
    void onOKBtn(CCObject* sender);

    TextInput* m_input = nullptr;

protected:
    TaskHolder<web::WebResponse> m_listener;
    ~PetRenamePopup() { m_listener.cancel(); }
};

}