#include "DifficultySelectorPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"

#include <Geode/binding/CCMenuItemToggler.hpp>
#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

DifficultySelectorPopup* DifficultySelectorPopup::create(MainLayer* owner) {
    auto ret = new DifficultySelectorPopup;
    if (ret && ret->init(owner)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DifficultySelectorPopup::init(MainLayer* owner) {
    if (!BasePopup::init(240.f, 220.f)) return false;

    m_owner = owner;

    setTitle("Select Difficulties");

    auto hardLabel = Build(CCLabelBMFont::create("Hard Difficulty", "bigFont.fnt"))
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() + 60.f })
        .scale(0.5f)
        .id("hard-label")
        .collect();

    auto hardMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .id("hard-menu")
        .scale(0.7f)
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() + 35.f })
        .collect();

    auto fourStToggler = Build(CCMenuItemToggler::create(
        ButtonSprite::create("4", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("4", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onFourBtn)
    ))
        .id("four-st-toggler")
        .parent(hardMenu)
        .collect();

    CCMenuItemToggler* fiveStToggler = nullptr;

    Build(CCMenuItemToggler::create(
        ButtonSprite::create("5", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("5", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onFiveBtn)
    ))
        .id("five-st-toggler")
        .store(fiveStToggler)
        .parent(hardMenu)
        .intoParent()
        .updateLayout();

    fourStToggler->toggle(m_owner->isSplitDifficultySelected(4));
    fiveStToggler->toggle(m_owner->isSplitDifficultySelected(5));

    auto harderLabel = Build(CCLabelBMFont::create("Harder Difficulty", "bigFont.fnt"))
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() + 10.f })
        .scale(0.5f)
        .id("harder-label")
        .collect();

    auto harderMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .id("harder-menu")
        .scale(0.7f)
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() - 15.f })
        .collect();

    auto sixStToggler = Build(CCMenuItemToggler::create(
        ButtonSprite::create("6", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("6", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onSixBtn)
    ))
        .id("six-st-toggler")
        .parent(harderMenu)
        .collect();

    CCMenuItemToggler* sevenStToggler = nullptr;

    Build(CCMenuItemToggler::create(
        ButtonSprite::create("7", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("7", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onSevenBtn)
    ))
        .id("seven-st-toggler")
        .store(sevenStToggler)
        .parent(harderMenu)
        .intoParent()
        .updateLayout();

    sixStToggler->toggle(m_owner->isSplitDifficultySelected(6));
    sevenStToggler->toggle(m_owner->isSplitDifficultySelected(7));

    auto insaneLabel = Build(CCLabelBMFont::create("Insane Difficulty", "bigFont.fnt"))
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() - 40.f })
        .scale(0.5f)
        .id("insane-label")
        .collect();

    auto insaneMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .id("insane-menu")
        .scale(0.7f)
        .parent(m_mainLayer)
        .pos({ centerX(), centerY() - 65.f })
        .collect();

    auto eightStToggler = Build(CCMenuItemToggler::create(
        ButtonSprite::create("8", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("8", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onEightBtn)
    ))
        .id("eight-st-toggler")
        .parent(insaneMenu)
        .collect();

    CCMenuItemToggler* nineStToggler = nullptr;

    Build(CCMenuItemToggler::create(
        ButtonSprite::create("9", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("9", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(DifficultySelectorPopup::onNineBtn)
    ))
        .id("nine-st-toggler")
        .store(nineStToggler)
        .parent(insaneMenu)
        .intoParent()
        .updateLayout();

    eightStToggler->toggle(m_owner->isSplitDifficultySelected(8));
    nineStToggler->toggle(m_owner->isSplitDifficultySelected(9));

    return true;
}

void DifficultySelectorPopup::onFourBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(4, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

void DifficultySelectorPopup::onFiveBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(5, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

void DifficultySelectorPopup::onSixBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(6, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

void DifficultySelectorPopup::onSevenBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(7, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

void DifficultySelectorPopup::onEightBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(8, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

void DifficultySelectorPopup::onNineBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_owner) return;

    m_owner->setSplitDifficultySelected(9, !toggler->isToggled());
    m_owner->refreshSplitDifficultyFilters();
}

}