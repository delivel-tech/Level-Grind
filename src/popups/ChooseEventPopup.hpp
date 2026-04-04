#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>

#include "EventLevelPopup.hpp"

using namespace geode::prelude;

class ChooseEventPopup : public Popup {
public:
    static ChooseEventPopup* create() {
        auto ret = new ChooseEventPopup;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init() override {
        if (!Popup::init({ 250.f, 160.f })) return false;

        this->setTitle("Choose Event");

        addSideArt(
            m_mainLayer,
            SideArt::All,
            SideArtStyle::PopupGold,
            false
        );

        auto btnMenu = CCMenu::create();
        btnMenu->setLayout(ColumnLayout::create());

        m_mainLayer->addChildAtPosition(
            btnMenu, Anchor::Center
        );

        auto btnDaily = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Daily Levels"),
            [](CCObject* sender) {
                EventLevelPopup::create(EventLevelPopup::GrindEventType::Daily)->show();
            }
        );

        auto btnWeekly = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Weekly Levels"),
            [](CCObject* sender) {
                EventLevelPopup::create(EventLevelPopup::GrindEventType::Weekly)->show();
            }
        );

        btnMenu->addChild(btnDaily);
        btnMenu->addChild(btnWeekly);

        btnMenu->updateLayout();

        return true;
    }
};
