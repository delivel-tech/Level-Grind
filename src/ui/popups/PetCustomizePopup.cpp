#include "PetCustomizePopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/platform/windows.hpp"
#include "Geode/ui/ColorPickPopup.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Scrollbar.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <UIBuilder.hpp>
#include <cue/ListNode.hpp>

#include "../../utils/utils.hpp"

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

    setTitle("Customize Pet");

    m_rarity = static_cast<Rarity>(petData.petRarity);

    if (m_rarity == Rarity::Common) {
        this->onClose(nullptr);
        return true;
    }

    auto columnContainer = Build(NineSlice::create("GJ_square05.png"))
        .contentSize({ 70, 200 })
        .parent(m_mainLayer)
        .pos(
            358, 125
        )
        .id("column-container")
        .collect();

    auto columnMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()->setAutoScale(false)->setAxisReverse(true))
        .id("column-menu")
        .parent(columnContainer)
        .center()
        .collect();

    auto colorsTitle = Build(CCLabelBMFont::create("Colors", "bigFont.fnt"))
        .scale(0.4f)
        .parent(columnMenu)
        .id("colors-title")
        .collect();

    auto colorsSelectorToggler = Build<CCMenuItemToggler>::createToggle(
        Build(CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png")).scale(0.8f).collect(),
        Build(CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png")).scale(0.8f).collect(),
        [] (CCMenuItemToggler* toggler) {
            bool newState = getNewTogglerState(toggler);
            Mod::get()->setSavedValue("custom-colors-enabled", newState);
        }
    )
        .parent(columnMenu)
        .collect();

    if (Mod::get()->getSavedValue<bool>("custom-colors-enabled")) colorsSelectorToggler->toggle(true);

    auto colorMainPreview = Build(ColorChannelSprite::create())
        .scale(0.6f)
        .id("color-preview")
        .color(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-color-main"))
        .intoMenuItem([](CCMenuItemSpriteExtra* btn) {
            auto popup = geode::ColorPickPopup::create(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-color-main"));
            if (!popup) {
                return;
            }

            if (auto preview = typeinfo_cast<ColorChannelSprite*>(btn->getChildByID("color-preview"))) {
                popup->setColorTarget(preview);
            }

            popup->setCallback([](cocos2d::ccColor4B const& color) {
                Mod::get()->setSavedValue("pet-color-main", cocos2d::ccColor3B { color.r, color.g, color.b });
            });

            popup->show();
        })
        .parent(columnMenu)
        .collect();

    auto colorSecondaryPreview = Build(ColorChannelSprite::create())
        .scale(0.6f)
        .id("color-preview")
        .color(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-color-sec"))
        .intoMenuItem([](CCMenuItemSpriteExtra* btn) {
            auto popup = geode::ColorPickPopup::create(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-color-sec"));
            if (!popup) {
                return;
            }

            if (auto preview = typeinfo_cast<ColorChannelSprite*>(btn->getChildByID("color-preview"))) {
                popup->setColorTarget(preview);
            }

            popup->setCallback([](cocos2d::ccColor4B const& color) {
                Mod::get()->setSavedValue("pet-color-sec", cocos2d::ccColor3B { color.r, color.g, color.b });
            });

            popup->show();
        })
        .parent(columnMenu)
        .collect();

    auto glowTitle = Build(CCLabelBMFont::create("Glow", "bigFont.fnt"))
        .scale(0.4f)
        .parent(columnMenu)
        .id("colors-title")
        .collect();

    if (m_rarity == Rarity::Common || m_rarity == Rarity::Rare) {
        auto lockBtn = Build(CCSprite::createWithSpriteFrameName("GJ_lock_001.png"))
            .intoMenuItem([] {
                FLAlertLayer::create(
                    "Locked!",
                    "You need <co>Legendary</c> to choose your icon.",
                    "OK"
                )->show();
            })
            .parent(columnMenu)
            .collect();
    } else {
        auto glowSelectorToggler = Build<CCMenuItemToggler>::createToggle(
            Build(CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png")).scale(0.8f).collect(),
            Build(CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png")).scale(0.8f).collect(),
            [] (CCMenuItemToggler* toggler) {
                bool newState = getNewTogglerState(toggler);
                Mod::get()->setSavedValue("custom-glow-enabled", newState);
            }
        )
        .parent(columnMenu)
        .collect();

        if (Mod::get()->getSavedValue<bool>("custom-glow-enabled")) glowSelectorToggler->toggle(true);

        auto colorGlowPreview = Build(ColorChannelSprite::create())
            .scale(0.6f)
            .id("color-preview")
            .color(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-glow"))
            .intoMenuItem([](CCMenuItemSpriteExtra* btn) {
                auto popup = geode::ColorPickPopup::create(Mod::get()->getSavedValue<cocos2d::ccColor3B>("pet-glow"));
                if (!popup) {
                    return;
                }

                if (auto preview = typeinfo_cast<ColorChannelSprite*>(btn->getChildByID("color-preview"))) {
                    popup->setColorTarget(preview);
                }

                popup->setCallback([](cocos2d::ccColor4B const& color) {
                    Mod::get()->setSavedValue("pet-glow", cocos2d::ccColor3B { color.r, color.g, color.b });
                });

                popup->show();
            })
            .parent(columnMenu)
            .collect();
    }

    columnMenu->updateLayout();

    m_selectedIconId = PetManager::getInstance().getSelectedCubeIconId();

    auto listNode = Build(cue::ListNode::create({300.f, 230.f}))
        .anchorPoint({0.5f, 0.5f})
        .pos(centerX() - 40, centerY() - 13)
        .parent(m_mainLayer)
        .id("icons-list")
        .collect();

    auto scrollBar = Build(Scrollbar::create(listNode->getScrollLayer()))
        .parent(m_mainLayer)
        .pos(listNode->getPositionX() + (listNode->getContentWidth() / 2) + 6,
        (listNode->getContentHeight() / 2) + 10)
        .scale(0.9f)
        .id("scroll-bar")
        .collect();

    if (m_rarity != Rarity::Mythic) {
        auto lockBtnMenu = Build<CCMenu>::create()
            .parent(listNode)
            .center()
            .collect();
        auto lockBtn = Build(CCSprite::createWithSpriteFrameName("GJ_lock_001.png"))
            .intoMenuItem([] {
                FLAlertLayer::create(
                    "Locked!",
                    "You need <cr>Mythic</c> to choose your icon.",
                    "OK"
                )->show();
            })
            .parent(lockBtnMenu)
            .collect();
        return true;
    }

    auto iconSelectorToggler = Build<CCMenuItemToggler>::createToggle(
        Build(CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png")).scale(0.8f).collect(),
        Build(CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png")).scale(0.8f).collect(),
        [] (CCMenuItemToggler* toggler) {
            bool newState = getNewTogglerState(toggler);
            Mod::get()->setSavedValue("custom-icon-enabled", newState);
        }
    )
        .parent(m_buttonMenu)
        .pos(
            50, m_buttonMenu->getContentHeight() - 22
        )
        .collect();

    if (Mod::get()->getSavedValue<bool>("custom-icon-enabled")) iconSelectorToggler->toggle(true);

    float const itemSize = 40.f;
    int const columns = 6;
    int const maxFrames = 485;

    CCMenu* currentRow = nullptr;
    int rowIndex = 0;
    for (int frameId = 1; frameId <= maxFrames; ++frameId) {
        if ((frameId - 1) % columns == 0) {
            currentRow = Build(CCMenu::create())
                .layout(RowLayout::create()->setGap(4)->setGrowCrossAxis(true))
                .contentSize({listNode->getContentWidth() - 12.f, itemSize})
                .id(fmt::format("icons-row-{}", rowIndex))
                .collect();
            currentRow->updateLayout();
            listNode->addCell(currentRow);
            rowIndex += 1;
        }

        auto itemNode = CCNode::create();
        itemNode->setContentSize({itemSize, itemSize});

        auto player = SimplePlayer::create(frameId);
        player->updatePlayerFrame(frameId, IconType::Cube);
        player->setScale(0.8f);
        player->setPosition({itemSize / 2.f, itemSize / 2.f});
        itemNode->addChild(player);

        auto item = CCMenuItemSpriteExtra::create(itemNode, this, menu_selector(PetCustomisePopup::onIconSelect));
        item->setTag(frameId);
        item->setAnchorPoint({0.5f, 0.5f});

        currentRow->addChild(item);
        currentRow->updateLayout();
        m_iconButtons.push_back({item, player, nullptr});
    }

    if (currentRow) currentRow->updateLayout();
    listNode->getScrollLayer()->m_contentLayer->updateLayout();
    listNode->updateLayout();
    listNode->scrollToTop();
    refreshSelection();

    return true;
}

void PetCustomisePopup::onIconSelect(CCObject* sender) {
    auto item = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    if (!item) return;

    m_selectedIconId = item->getTag();
    PetManager::getInstance().setSelectedCubeIconId(m_selectedIconId);
    refreshSelection();
}

void PetCustomisePopup::refreshSelection() {
    for (auto& entry : m_iconButtons) {
        if (!entry.item || !entry.player) continue;

        bool selected = entry.item->getTag() == m_selectedIconId;
        entry.player->setColor(selected ? ccColor3B{255, 255, 255} : ccColor3B{100, 100, 100});
        entry.player->setScale(0.8f);
    }
}

}