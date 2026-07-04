#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"

#include "../../managers/PetManager.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <cue/ListNode.hpp>
#include <vector>

using namespace geode::prelude;

namespace levelgrind {

class PetCustomisePopup : public BasePopup {
public:
    static PetCustomisePopup* create(PetManager::PetData petData);

private:
    bool init(PetManager::PetData petData);
    void onIconSelect(CCObject* sender);
    void onColorsToggle(CCObject* sender);
    void onGlowToggle(CCObject* sender);
    void onIconsToggle(CCObject* sender);
    void onPrimaryColorPicker(CCObject* sender);
    void onSecondaryColorPicker(CCObject* sender);
    void onGlowColorPicker(CCObject* sender);
    void refreshSelection();
    void refreshColorUI();
    void refreshUI();
    void refreshRarityUI();

    struct IconButtonEntry {
        CCMenuItemSpriteExtra* item = nullptr;
        SimplePlayer* player = nullptr;
        CCSprite* background = nullptr;
    };

    enum Rarity {
        Common = 1, Rare = 2, Legend = 3, Mythic = 4
    };

    Rarity m_rarity;
    int m_petLevel = 0;
    int m_selectedIconId = -1;
    std::vector<IconButtonEntry> m_iconButtons;
};

}