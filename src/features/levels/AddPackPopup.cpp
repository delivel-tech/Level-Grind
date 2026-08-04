#include "AddPackPopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/ColorPickPopup.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/TextInput.hpp"

#include "../../core/BackendManager.hpp"

#include "../../utils/utils.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/general.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>

namespace levelgrind {

AddPackPopup* AddPackPopup::create() {
    auto ret = new AddPackPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AddPackPopup::init() {
    if (!BasePopup::init({ 280, 280 })) return false;

    setTitle("Grind Owner: Add Pack");

    auto contentMenu = Build(CCMenu::create())
        .parent(m_mainLayer)
        .center()
        .id("content-menu")
        .scale(0.9f)
        .height(160)
        .layout(ColumnLayout::create()->setGap(15)->setAxisReverse(true)->setCrossAxisReverse(true)->setGrowCrossAxis(true))
        .collect();

    auto titleTextInput = Build(TextInput::create(140, "Title", "chatFont.fnt"))
        .parent(contentMenu)
        .id("title-text-input")
        .collect();

    auto level1TextInput = Build(TextInput::create(140, "Level ID 1", "chatFont.fnt"))
        .parent(contentMenu)
        .with([](TextInput* input) {
            input->setCommonFilter(CommonFilter::Int);
        })
        .id("level-1-input")
        .collect();

    auto level2TextInput = Build(TextInput::create(140, "Level ID 2", "chatFont.fnt"))
        .parent(contentMenu)
        .with([](TextInput* input) {
            input->setCommonFilter(CommonFilter::Int);
        })
        .id("level-2-input")
        .collect();

    auto level3TextInput = Build(TextInput::create(140, "Level ID 3", "chatFont.fnt"))
        .parent(contentMenu)
        .with([](TextInput* input) {
            input->setCommonFilter(CommonFilter::Int);
        })
        .id("level-3-input")
        .collect();

    auto difficultyTextInput = Build(TextInput::create(140, "Difficulty", "chatFont.fnt"))
        .parent(contentMenu)
        .with([](TextInput* input) {
            input->setCommonFilter(CommonFilter::Int);
        })
        .id("difficulty-input")
        .collect();

    auto colorPreview = Build(ColorChannelSprite::create())
        .collect();

    auto colorPreviewBtn = CCMenuItemExt::createSpriteExtra(
        colorPreview,
        [this, colorPreview](CCObject* sender) {
            auto popup = geode::ColorPickPopup::create(m_color);
            if (!popup) return;

            if (colorPreview) {
                popup->setColorTarget(colorPreview);
            }

            popup->setCallback([this, colorPreview](cocos2d::ccColor4B const& color) {
                m_color = { color.r, color.g, color.b };
                colorPreview->setColor(m_color);
            });

            popup->show();
        }
    );
    contentMenu->addChild(colorPreviewBtn);
    colorPreviewBtn->setID("color-preview-btn");

    auto moonToggler = Build<CCMenuItemToggler>::createToggle(
        Build(CCSprite::createWithSpriteFrameName("GJ_bigMoon_001.png")).color({ 100, 100, 100 }).collect(),
        Build(CCSprite::createWithSpriteFrameName("GJ_bigMoon_001.png")).collect(),
        [this](CCMenuItemToggler* toggler) {
            bool isToggled = getNewTogglerState(toggler);

            if (isToggled) {
                m_star = false;
            } else {
                m_star = true;
            }
        }
    )
        .parent(contentMenu)
        .id("moon-toggler")
        .collect();

    auto addBtn = Build(ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this, titleTextInput, level1TextInput, level2TextInput, level3TextInput, difficultyTextInput] {
            NewGrindPackBody body {
                titleTextInput->getString(),
                this->m_star,
                numFromString<int>(level1TextInput->getString()).unwrap(),
                numFromString<int>(level2TextInput->getString()).unwrap(),
                numFromString<int>(level3TextInput->getString()).unwrap(),
                m_color.r, m_color.g, m_color.b,
                numFromString<int>(difficultyTextInput->getString()).unwrap()
            };

            async::spawn(this->onAddClicked(body));
        })
        .parent(m_buttonMenu)
        .pos(centerX(), 30)
        .id("add-btn")
        .collect();

    contentMenu->updateLayout();

    return true;
}

arc::Future<> AddPackPopup::onAddClicked(NewGrindPackBody body) {
    auto uPopup = UploadActionPopup::create(nullptr, "Adding pack...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    auto parsed = co_await BackendManager::getInstance().newGrindPack(body);

    if (!uPopupRef) co_return;

    if (!parsed.ok) {
        uPopupRef->showFailMessage("Failed! Try again later.");
        co_return;
    }

    uPopupRef->showSuccessMessage("Success! Pack added.");
    co_return;
}

}