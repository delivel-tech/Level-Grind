#pragma once

#include "Geode/ui/TextInput.hpp"
#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include <arc/future/Future.hpp>

using namespace geode::prelude;

namespace levelgrind {

class PetRenamePopup : public BasePopup {
public:
    static PetRenamePopup* create(std::string currPetName);

private:
    bool init(std::string currPetName);
    void onOKBtn(CCObject* sender);
    arc::Future<> onRename(std::string petName);

    TextInput* m_input = nullptr;
};

}