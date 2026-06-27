#include "PetCustomizePopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"

#include <UIBuilder.hpp>

namespace levelgrind {

PetCustomisePopup* PetCustomisePopup::create(PetManager::PetData petData) {
    auto ret = new PetCustomisePopup;
    if (ret && ret->init(petData)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PetCustomisePopup::init(PetManager::PetData petData) {
    if (!BasePopup::init({400, 280})) return false;

    m_rarity = static_cast<Rarity>(petData.petRarity);

    if (m_rarity == Rarity::Common) {
        this->onClose(nullptr);
    }

    auto iconsContainer = Build(NineSlice::create("GJ_square05.png"))
        .contentSize(300, 260)
        .pos(
            { m_mainLayer->getContentWidth() / 2.3f, m_mainLayer->getContentHeight() / 2 }
        )
        .parent(m_mainLayer)
        .id("icons-container")
        .collect();

    auto iconsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGrowCrossAxis(true))
        .contentSize(iconsContainer->getContentSize())
        .pos(iconsContainer->getPosition())
        .parent(m_mainLayer)
        .id("icons-menu")
        .collect();

    return true;
}

}