#include "LGDiffSelector.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Layout.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>

using namespace geode::prelude;

LGDiffSelector* LGDiffSelector::create(LevelGrindLayer* currentLayer) {
    auto ret = new LGDiffSelector();
    if (ret && ret->init(currentLayer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGDiffSelector::init(LevelGrindLayer* currentLayer) {
    if (!Popup::init(240.f, 220.f)) {
        return false;
    }

    m_currentLayer = currentLayer;

    setTitle("Select Difficulties");

    auto hardLabel = CCLabelBMFont::create("Hard Difficulty", "bigFont.fnt");
    m_mainLayer->addChildAtPosition(hardLabel, Anchor::Center, { 0.f, 60.f });
    hardLabel->setScale(0.5f);

    hardLabel->setID("hard-label");

    auto fourStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("4", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("4", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onFourBtn)
    );
    fourStToggler->setID("four-st-toggler");
    fourStToggler->toggle(m_currentLayer->isSplitDifficultySelected(4));

    auto fiveStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("5", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("5", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onFiveBtn)
    );
    fiveStToggler->setID("five-st-toggler");
    fiveStToggler->toggle(m_currentLayer->isSplitDifficultySelected(5));

    auto hardMenu = CCMenu::create();
    hardMenu->setLayout(RowLayout::create()->setGap(15.f));
    hardMenu->setID("hard-menu");

    hardMenu->setScale(0.7f);

    hardMenu->addChild(fourStToggler);
    hardMenu->addChild(fiveStToggler);

    hardMenu->updateLayout();

    m_mainLayer->addChildAtPosition(hardMenu, Anchor::Center, { 0.f, 35.f });

    auto harderLabel = CCLabelBMFont::create("Harder Difficulty", "bigFont.fnt");
    m_mainLayer->addChildAtPosition(harderLabel, Anchor::Center, { 0.f, 10.f });
    harderLabel->setScale(0.5f);
    harderLabel->setID("harder-label");

    auto sixStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("6", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("6", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onSixBtn)
    );
    sixStToggler->setID("six-st-toggler");
    sixStToggler->toggle(m_currentLayer->isSplitDifficultySelected(6));

    auto sevenStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("7", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("7", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onSevenBtn)
    );
    sevenStToggler->setID("seven-st-toggler");
    sevenStToggler->toggle(m_currentLayer->isSplitDifficultySelected(7));

    auto harderMenu = CCMenu::create();
    harderMenu->setLayout(RowLayout::create()->setGap(15.f));
    harderMenu->setID("harder-menu");

    harderMenu->setScale(0.7f);
    harderMenu->addChild(sixStToggler);
    harderMenu->addChild(sevenStToggler);
    harderMenu->updateLayout();

    m_mainLayer->addChildAtPosition(harderMenu, Anchor::Center, { 0.f, -15.f });

    auto insaneLabel = CCLabelBMFont::create("Insane Difficulty", "bigFont.fnt");
    m_mainLayer->addChildAtPosition(insaneLabel, Anchor::Center, { 0.f, -40.f });
    insaneLabel->setScale(0.5f);
    insaneLabel->setID("insane-label");

    auto eightStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("8", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("8", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onEightBtn)
    );
    eightStToggler->setID("eight-st-toggler");
    eightStToggler->toggle(m_currentLayer->isSplitDifficultySelected(8));

    auto nineStToggler = CCMenuItemToggler::create(
        ButtonSprite::create("9", "goldFont.fnt", "GJ_button_04.png"),
        ButtonSprite::create("9", "goldFont.fnt", "GJ_button_02.png"),
        this,
        menu_selector(LGDiffSelector::onNineBtn)
    );
    nineStToggler->setID("nine-st-toggler");
    nineStToggler->toggle(m_currentLayer->isSplitDifficultySelected(9));

    auto insaneMenu = CCMenu::create();
    insaneMenu->setLayout(RowLayout::create()->setGap(15.f));
    insaneMenu->setID("insane-menu");

    insaneMenu->setScale(0.7f);
    insaneMenu->addChild(eightStToggler);
    insaneMenu->addChild(nineStToggler);
    insaneMenu->updateLayout();

    m_mainLayer->addChildAtPosition(insaneMenu, Anchor::Center, { 0.f, -65.f });
    m_mainLayer->updateLayout();

    return true;
}

void LGDiffSelector::onFourBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(4, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}

void LGDiffSelector::onFiveBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(5, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}

void LGDiffSelector::onSixBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(6, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}

void LGDiffSelector::onSevenBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(7, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}

void LGDiffSelector::onEightBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(8, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}

void LGDiffSelector::onNineBtn(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler || !m_currentLayer) return;

    m_currentLayer->setSplitDifficultySelected(9, !toggler->isToggled());
    m_currentLayer->refreshSplitDifficultyFilters();
}