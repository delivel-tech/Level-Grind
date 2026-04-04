#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>

using namespace geode::prelude;

class ChooseAccessoriesPopup : public Popup {
public:
    static ChooseAccessoriesPopup* create() {
        auto ret = new ChooseAccessoriesPopup;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool m_a1Enabled = Mod::get()->getSavedValue<bool>("a-1-enabled");
    bool m_a2Enabled = Mod::get()->getSavedValue<bool>("a-2-enabled");
    bool m_a3Enabled = Mod::get()->getSavedValue<bool>("a-3-enabled");

    bool m_a1Bought = Mod::get()->getSavedValue<bool>("a-1-bought");
    bool m_a2Bought = Mod::get()->getSavedValue<bool>("a-2-bought");
    bool m_a3Bought = Mod::get()->getSavedValue<bool>("a-3-bought");

private:
    bool init() override {
        if (!Popup::init({ 250.f, 160.f })) return false;

        this->setTitle("Choose Accessories");

        addSideArt(
            m_mainLayer,
            SideArt::All,
            SideArtStyle::PopupGold,
            false
        );

        auto btnMenu = CCMenu::create();
        btnMenu->setLayout(ColumnLayout::create());
        btnMenu->setScale(0.8f);

        m_mainLayer->addChildAtPosition(
            btnMenu, Anchor::Center
        );

        if (m_a1Bought) {
            auto bunnyEarsToggler = CCMenuItemToggler::create(
                ButtonSprite::create("Bunny Ears"),
                ButtonSprite::create("Bunny Ears", "goldFont.fnt", "GJ_button_02.png"),
                this,
                menu_selector(ChooseAccessoriesPopup::onBunnyEarsToggler)
            );
            bunnyEarsToggler->setID("bunny-ears-toggler");
            btnMenu->addChild(bunnyEarsToggler);
            if (m_a1Enabled) {
                bunnyEarsToggler->toggle(true);
                Mod::get()->setSavedValue<bool>("a-1-enabled", true);
            }
        }

        if (m_a2Bought) {
            auto moonCrownToggler = CCMenuItemToggler::create(
                ButtonSprite::create("Moon Crown"),
                ButtonSprite::create("Moon Crown", "goldFont.fnt", "GJ_button_02.png"),
                this,
                menu_selector(ChooseAccessoriesPopup::onMoonCrownToggler)
            );
            moonCrownToggler->setID("moon-crown-toggler");
            btnMenu->addChild(moonCrownToggler);
            if (m_a2Enabled) {
                moonCrownToggler->toggle(true);
                Mod::get()->setSavedValue<bool>("a-2-enabled", true);
            }
        }

        if (m_a3Bought) {
            auto zoinkHeadphonesToggler = CCMenuItemToggler::create(
                ButtonSprite::create("Zoink Headphones"),
                ButtonSprite::create("Zoink Headphones", "goldFont.fnt", "GJ_button_02.png"),
                this,
                menu_selector(ChooseAccessoriesPopup::onZoinkHeadphonesToggler)
            );
            zoinkHeadphonesToggler->setID("zoink-headphones-toggler");
            btnMenu->addChild(zoinkHeadphonesToggler);
            if (m_a3Enabled) {
                zoinkHeadphonesToggler->toggle(true);
                Mod::get()->setSavedValue<bool>("a-3-enabled", true);
            }
        }

        btnMenu->updateLayout();

        return true;
    }

    void onBunnyEarsToggler(CCObject* sender) {
        auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	    bool isToggled = !toggler->isToggled();

	    if (isToggled) {
            Mod::get()->setSavedValue<bool>("a-1-enabled", true);
	    } else {
            Mod::get()->setSavedValue<bool>("a-1-enabled", false);
	    }
    }

    void onMoonCrownToggler(CCObject* sender) {
        auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	    bool isToggled = !toggler->isToggled();

	    if (isToggled) {
            Mod::get()->setSavedValue<bool>("a-2-enabled", true);
	    } else {
            Mod::get()->setSavedValue<bool>("a-2-enabled", false);
	    }
    }

    void onZoinkHeadphonesToggler(CCObject* sender) {
        auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	    bool isToggled = !toggler->isToggled();

	    if (isToggled) {
            Mod::get()->setSavedValue<bool>("a-3-enabled", true);
	    } else {
            Mod::get()->setSavedValue<bool>("a-3-enabled", false);
	    }
    }
};