#include "SettingsLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButton.hpp"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/async.hpp"
#include "ccTypes.h"
#include "cue/ListNode.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <algorithm>
#include <cstddef>
#include <cue/Colors.hpp>
#include <cue/ListBorder.hpp>
#include <fmt/format.h>
#include <string>

#include "../utils/utils.hpp"
#include "BackendManager.hpp"
#include "DataManager.hpp"

#include <argon/argon.hpp>

using namespace geode::prelude;

namespace levelgrind {

SettingsLayer* SettingsLayer::create() {
    auto ret = new SettingsLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SettingsLayer::init() {
    if (!BaseLayer::init()) return false;

    this->setTouchEnabled(true);
    this->setMouseEnabled(true);

    replaceBgToClassic();

    setBoolSetting("settings-were-opened", true);

    for (size_t i = 0; i < 3; i++) {
        auto list = Build(cue::ListNode::create(
            { 356.f, 220.f },
            cue::Brown,
            cue::ListBorderStyle::SlimLevels
        ))
            .zOrder(6)
            .center()
            .parent(this)
            .visible(false)
            .id(fmt::format("settings-list-{}", i + 1))
            .collect();

        m_listNodes.push_back(list);
    }

    m_grindList = m_listNodes[0];
    m_appearanceList = m_listNodes[1];
    m_otherList = m_listNodes[2];

    m_grindList->setVisible(true);
    m_currentList = m_grindList;

    for (auto* list : m_listNodes) {
        if (list && list->getScrollLayer()) {
            list->getScrollLayer()->setTouchEnabled(list == m_currentList);
            list->getScrollLayer()->setMouseEnabled(list == m_currentList);
        }
    }

    makeTabs();

    if (m_grindTab) m_grindTab->toggle(true);

    createGrindList();
    createAppearanceList();
    createOtherList();

    m_resetBtnMenu = Build(CCMenu::create())
        .pos({ 0, 0 })
        .zOrder(7)
        .pos({ m_winSize.width - 25, 25 })
        .id("reset-btn-menu")
        .parent(this)
        .collect();

    m_resetBtn = Build(CCSprite::createWithSpriteFrameName("GJ_deleteSongBtn_001.png"))
        .intoMenuItem([this] {
            createQuickPopup(
                "Reset Settings?",
                "Confirm that you want to reset Level Grind settings",
                "Cancel", "Confirm",
                [this](auto, bool btn2) {
                    if (btn2) {
                        applyDefaultSettings();

                        refreshBackgroundColorUI();
                        Notification::create("Settings were reset. Please re-open settings.", NotificationIcon::Success)->show();
                    }
                }
            );
        })
        .id("reset-btn")
        .parent(m_resetBtnMenu)
        .collect();

    return true;
}

void SettingsLayer::makeTabs() {
    float tabsY = m_winSize.height / 2 + m_grindList->getContentHeight() / 2 + 30.f;

    auto tabsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(25))
        .scale(0.85f)
        .pos({ centerX(), tabsY - 2 })
        .zOrder(5)
        .parent(this)
        .id("tabs-menu")
        .collect();

    auto grindTab = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Grind",
        this,
        makeCb(SettingsLayer::onTab)
    );
    grindTab->setID("grind-tab");

    auto appearanceTab = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Appearance",
        this,
        makeCb(SettingsLayer::onTab)
    );
    appearanceTab->setID("appearance-tab");

    auto otherTab = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Other",
        this,
        makeCb(SettingsLayer::onTab)
    );
    otherTab->setID("other-tab");

    m_tabs.push_back(grindTab);
    m_tabs.push_back(appearanceTab);
    m_tabs.push_back(otherTab);

    m_grindTab = m_tabs[0];
    m_appearanceTab = m_tabs[1];
    m_otherTab = m_tabs[2];

    tabsMenu->addChild(grindTab);
    tabsMenu->addChild(appearanceTab);
    tabsMenu->addChild(otherTab);

    tabsMenu->updateLayout();

    return;
}

CCMenu* SettingsLayer::makeToggleCell(
    const char* title,
    const char* desc,
    const char* settingKey,
    CCNode* target
) {
    auto cell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE)
        .id(fmt::format("{}-cell", settingKey))
        .collect();

    auto label = Build(CCLabelBMFont::create(title, "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.62f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, CELL_SIZE.height / 2 })
        .parent(cell)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.5f)
        .intoMenuItem([title, desc] {
            FLAlertLayer::create(title, desc, "OK")->show();
        })
        .pos({
            8 + label->getScaledContentWidth() + 10,
            CELL_SIZE.height / 2
        })
        .parent(cell)
        .collect();

    auto toggler = Build<CCMenuItemToggler>::createToggle(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        [this, settingKey](CCMenuItemToggler* toggler) {
            setBoolSetting(settingKey, getNewTogglerState(toggler));
            if (std::string(settingKey) == "only-uncompleted" && getNewTogglerState(toggler)) {
                setBoolSetting("only-completed", false);
                m_onlyCompletedToggler->toggle(false);
            }
            if (std::string(settingKey) == "only-completed" && getNewTogglerState(toggler)) {
                setBoolSetting("only-uncompleted", false);
                m_onlyUncompletedToggler->toggle(false);
            }
            if (std::string(settingKey) == "newer-first" && getNewTogglerState(toggler)) {
                setBoolSetting("recently-added", false);
                m_recentlyAddedToggler->toggle(false);
            }
            if (std::string(settingKey) == "recently-added" && getNewTogglerState(toggler)) {
                setBoolSetting("newer-first", false);
                m_newerFirstToggler->toggle(false);
            }
        }
    )
        .pos({ CELL_SIZE.width - 18, CELL_SIZE.height / 2 })
        .scale(0.7f)
        .toggle(Mod::get()->getSavedValue<bool>(settingKey))
        .id(fmt::format("{}-toggler", settingKey))
        .parent(cell)
        .collect();

    if (std::string(settingKey) == "only-uncompleted") m_onlyUncompletedToggler = toggler;
    if (std::string(settingKey) == "only-completed") m_onlyCompletedToggler = toggler;
    if (std::string(settingKey) == "newer-first") m_newerFirstToggler = toggler;
    if (std::string(settingKey) == "recently-added") m_recentlyAddedToggler = toggler;

    return cell;
}

void SettingsLayer::applyDefaultSettings() {
    Mod::get()->setSavedValue("newer-first", true);
    Mod::get()->setSavedValue("only-uncompleted", false);
    Mod::get()->setSavedValue("only-completed", false);
    Mod::get()->setSavedValue("recently-added", false);
    Mod::get()->setSavedValue("rgbBackground", cocos2d::ccColor3B { 37, 50, 167 });
    Mod::get()->setSavedValue("background-speed", 1.f);
    Mod::get()->setSavedValue("disable-badges", false);
    Mod::get()->setSavedValue("disable-pet", false);
    Mod::get()->setSavedValue("enable-indicators", true);
    Mod::get()->setSavedValue("enable-completion-info", false);
    Mod::get()->setSavedValue("enable-progress-bar", false);
    Mod::get()->setSavedValue("hide-badges-for-mods", true);
    Mod::get()->setSavedValue("auto-notes", false);
}

CCMenu* SettingsLayer::makeFloatCell(
    const char* title,
    const char* desc,
    const char* settingKey,
    float min,
    float max
) {
    auto cell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE.width, 44.f)
        .id(fmt::format("{}-cell", settingKey))
        .collect();

    auto label = Build(CCLabelBMFont::create(title, "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.45f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, 22 })
        .parent(cell)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.5f)
        .intoMenuItem([title, desc] {
            FLAlertLayer::create(title, desc, "OK")->show();
        })
        .pos({
            8 + label->getScaledContentWidth() + 10,
            22
        })
        .parent(cell)
        .collect();

    auto floatInput = Build(TextInput::create(90.f, "0.00"))
        .scale(0.6f)
        .pos({ CELL_SIZE.width - 85.f, 22.f })
        .parent(cell)
        .collect();

    auto updateFloatUI = [settingKey, min, max](float value, TextInput* input) {
        float clamped = std::clamp(value, min, max);
        Mod::get()->setSavedValue(settingKey, clamped);

        if (input) {
            input->setString(fmt::format("{:.2f}", clamped));
        }
    };

    floatInput->setCommonFilter(CommonFilter::Float);
    floatInput->setCallback([this, settingKey, floatInput, min, max, updateFloatUI](std::string const& str) {
        if (str.empty() || str == "-" || str == "." || str == "-.") {
            return;
        }

        if (auto parsed = numFromString<float>(str)) {
            updateFloatUI(parsed.unwrap(), floatInput);
        } else {
            updateFloatUI(Mod::get()->getSavedValue<float>(settingKey), nullptr);
        }
    });

    auto makeArrowBtn = [this, cell, settingKey, min, max, floatInput, updateFloatUI](bool isRight, bool isBig, float step) {
        auto arrowSprite = CCSprite::createWithSpriteFrameName(
            isBig ? "GJ_arrow_03_001.png" : "GJ_arrow_01_001.png"
        );
        arrowSprite->setScale(isBig ? 0.36f : 0.52f);

        if (isRight) {
            arrowSprite->setFlipX(true);
        }

        auto btn = Build(arrowSprite)
            .intoMenuItem([settingKey, min, max, step, floatInput, updateFloatUI] {
                float current = Mod::get()->getSavedValue<float>(settingKey);

                if (auto parsed = numFromString<float>(floatInput->getString())) {
                    current = parsed.unwrap();
                }

                updateFloatUI(std::clamp(current + step, min, max), floatInput);
            })
            .parent(cell)
            .collect();

        return btn;
    };

    auto bigSpeedLeftBtn = makeArrowBtn(false, true, -0.5f);
    auto speedLeftBtn = makeArrowBtn(false, false, -0.1f);
    auto speedRightBtn = makeArrowBtn(true, false, 0.1f);
    auto bigSpeedRightBtn = makeArrowBtn(true, true, 0.5f);

    bigSpeedLeftBtn->setPosition({ CELL_SIZE.width - 148.f, 22.f });
    speedLeftBtn->setPosition({ CELL_SIZE.width - 128.f, 22.f });
    speedRightBtn->setPosition({ CELL_SIZE.width - 43.f, 22.f });
    bigSpeedRightBtn->setPosition({ CELL_SIZE.width - 23.f, 22.f });

    if (std::string(settingKey) == "background-speed") {
        updateFloatUI(
            std::clamp(Mod::get()->getSavedValue<float>(settingKey), min, max),
            floatInput
        );
    }

    return cell;
}

CCMenu* SettingsLayer::makeColorCell(
    const char* title,
    const char* desc,
    SEL_MenuHandler cb,
    CCNode* target
) {
    auto cell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE.width, 34.f)
        .collect();

    auto label = Build(CCLabelBMFont::create(title, "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.45f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, 17 })
        .parent(cell)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.42f)
        .intoMenuItem([title, desc] {
            FLAlertLayer::create(title, desc, "OK")->show();
        })
        .pos({
            8 + label->getScaledContentWidth() + 10,
            17
        })
        .parent(cell)
        .collect();

    m_bgPreview = Build(ColorChannelSprite::create())
        .scale(0.58f)
        .collect();

    auto colorPreviewBtn = CCMenuItemSpriteExtra::create(
        m_bgPreview,
        target,
        cb
    );
    colorPreviewBtn->setPosition({ CELL_SIZE.width - 17.f, 17.f });
    colorPreviewBtn->setID("background-color-btn");
    cell->addChild(colorPreviewBtn);

    return cell;
}

void SettingsLayer::refreshBackgroundColorUI() {
    auto color = Mod::get()->getSavedValue<cocos2d::ccColor3B>("rgbBackground");

    if (m_bgPreview) {
        m_bgPreview->setColor(color);
    }
}

void SettingsLayer::onBackgroundColorPicker(CCObject* sender) {
    auto popup = geode::ColorPickPopup::create(Mod::get()->getSavedValue<cocos2d::ccColor3B>("rgbBackground"));
    if (!popup) {
        return;
    }

    if (m_bgPreview) {
        popup->setColorTarget(m_bgPreview);
    }

    popup->setCallback([this](cocos2d::ccColor4B const& color) {
        Mod::get()->setSavedValue("rgbBackground", cocos2d::ccColor3B { color.r, color.g, color.b });
        this->refreshBackgroundColorUI();
    });

    popup->show();
}

void SettingsLayer::setBoolSetting(const char* key, bool value) {
    Mod::get()->setSavedValue(key, value);
}

CCMenu* SettingsLayer::makeHeaderCell(const char* title) {
    auto headerCell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize({ CELL_SIZE.width, 24.f })
        .collect();

    auto headerLabel = Build(CCLabelBMFont::create(title, "goldFont.fnt"))
        .scale(0.5f)
        .pos({ CELL_SIZE.width / 2, 12 })
        .parent(headerCell)
        .collect();

    return headerCell;
}

void SettingsLayer::createGrindList() {
    cue::ListNode* list = m_grindList;

    list->addCell(this->makeHeaderCell("Grind Settings"));

    list->addCell(makeToggleCell(
        "Only Uncompleted",
        "If enabled, only uncompleted levels will be shown.",
        "only-uncompleted",
        this
    ));

    list->addCell(makeToggleCell(
        "Only Completed",
        "If enabled, only completed levels will be shown.",
        "only-completed",
        this
    ));

    list->addCell(makeToggleCell(
        "Newer First",
        "If enabled, newly created levels will be shown first.",
        "newer-first",
        this
    ));

    list->addCell(makeToggleCell(
        "Recently Added",
        "If enabled, recently added levels into the database will be shown first.",
        "recently-added",
        this
    ));

    list->addCell(makeToggleCell(
        "Enable Indicators",
        "Shows an info button next to level name that shows information about the level in Grind Database.",
        "enable-indicators",
        this
    ));

    list->addCell(makeToggleCell(
        "Enable Auto Notes",
        "Shows a note when clicking a play button if exists",
        "auto-notes",
        this
    ));

    list->updateLayout();
    list->getScrollLayer()->m_contentLayer->updateLayout();
    list->scrollToTop();
}

void SettingsLayer::initSettings() {
    if (Mod::get()->getSavedValue<bool>("settings-were-opened")) return;

    applyDefaultSettings();
}

void SettingsLayer::createAppearanceList() {
    cue::ListNode* list = m_appearanceList;
    if (!list) return;

    list->addCell(makeHeaderCell("Appearance Settings"));

    list->addCell(makeColorCell(
        "Background Color",
        "Customize the main layer background color.",
        makeCb(SettingsLayer::onBackgroundColorPicker),
        this
    ));

    list->addCell(makeFloatCell(
        "Background Speed",
        "Change the speed of the main layer background.",
        "background-speed",
        0.0f,
        5.f
    ));

    list->addCell(makeToggleCell(
        "Enable Completion Info",
        "Shows a completion information at the top of Level Browser Layer of Level Grind.",
        "enable-completion-info",
        this
    ));

    list->addCell(makeToggleCell(
        "Enable Progress Bar",
        "Shows a progress bar in Level Browser Layer of Level Grind.",
        "enable-progress-bar",
        this
    ));

    list->addCell(makeHeaderCell("Badge Settings"));

    list->addCell(makeToggleCell(
        "Disable Badges",
        "Hides all badges added by the mod",
        "disable-badges",
        this
    ));

    list->addCell(makeToggleCell(
        "Hide Grind badges for GD Mods",
        "Hides badges added by Level Grind for any kind of GD Moderators",
        "hide-badges-for-mods",
        this
    ));

    refreshBackgroundColorUI();
    list->updateLayout();
    list->getScrollLayer()->m_contentLayer->updateLayout();
    list->scrollToTop();

    return;
}

void SettingsLayer::createOtherList() {
    cue::ListNode* list = m_otherList;
    if (!list) return;

    list->addCell(makeHeaderCell("Staff Settings"));

    // req cell
    auto reqCell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE)
        .collect();

    auto reqLabel = Build(CCLabelBMFont::create("Request Staff Access", "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.6f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, CELL_SIZE.height / 2 })
        .parent(reqCell)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.5f)
        .intoMenuItem([] {
            FLAlertLayer::create(
                "Requesting Staff Access",
                "Gives you Staff access if you're a Grind Staff member. Does nothing for non-staff members.",
                "OK"
            )->show();
        })
        .pos({
            8 + reqLabel->getScaledContentWidth() + 10,
            CELL_SIZE.height / 2
        })
        .parent(reqCell)
        .collect();

    auto reqBtn = Build(ButtonSprite::create("Req", "bigFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.55f)
        .intoMenuItem([this] {
            async::spawn(this->onRequestStaffAccessClicked());
        })
        .pos({ CELL_SIZE.width - 26, CELL_SIZE.height / 2 })
        .parent(reqCell)
        .collect();

    list->addCell(reqCell);

    list->addCell(makeHeaderCell("Other Settings"));

    // sync cell

    auto syncCell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE)
        .collect();

    auto labelSync = Build(CCLabelBMFont::create("Sync data", "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.6f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, CELL_SIZE.height / 2 })
        .parent(syncCell)
        .collect();

    auto infoBtnSync = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.5f)
        .intoMenuItem([] {
            FLAlertLayer::create(
                "Sync data",
                "Updates such data as Staff badges and notes.",
                "OK"
            )->show();
        })
        .pos({ 8 + labelSync->getScaledContentWidth() + 10, CELL_SIZE.height / 2 })
        .parent(syncCell)
        .collect();

    auto syncBtn = Build(ButtonSprite::create("Sync", "bigFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.55f)
        .intoMenuItem([this] {
            DataManager::getInstance().clearSharedData();
            async::spawn(this->onSyncClicked());
        })
        .pos({ CELL_SIZE.width - 29, CELL_SIZE.height / 2 })
        .parent(syncCell)
        .collect();

    list->addCell(syncCell);

    // argon cell

    auto argonCell = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .contentSize(CELL_SIZE)
        .collect();

    auto labelArgon = Build(CCLabelBMFont::create("Request Argon Token", "bigFont.fnt"))
        .limitLabelWidth(CELL_SIZE.width * 0.6f, 0.5f, 0.1f)
        .anchorPoint({ 0, 0.5f })
        .pos({ 8, CELL_SIZE.height / 2 })
        .parent(argonCell)
        .collect();

    auto infoBtnArgon = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.5f)
        .intoMenuItem([] {
            FLAlertLayer::create(
                "Request Argon Token",
                "Requests Argon Token, which is used for authentification in some Level Grind API endpoints.\n"
                "Only use it if you're experiencing problems with accessing Level Grind partially or if you were told to do so by Delivel.",
                "OK"
            )->show();
        })
        .pos({ 8 + labelArgon->getScaledContentWidth() + 10, CELL_SIZE.height / 2 })
        .parent(argonCell)
        .collect();

    auto reqArgonBtn = Build(ButtonSprite::create("Req", "bigFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.55f)
        .intoMenuItem([this] {
            argon::clearAllTokens();
            auto uPopup = UploadActionPopup::create(nullptr, "Requesting argon...");
            uPopup->show();

            auto uPopupRef = Ref(uPopup);

            m_argonListener.spawn(
                argon::startAuth(),
                [uPopupRef](Result<std::string> res) {
                    if (!uPopupRef) return;

                    log::info("callback");
                    log::info("ok = {}", res.ok());

                    if (!res.ok()) {
                        log::error("{}", res.unwrapErr());
                        uPopupRef->showFailMessage("Failed! Try again later.");
                        return;
                    } else {
                        auto token = std::move(res).unwrap();
                        Mod::get()->setSavedValue("argon_token", token);
                        DataManager::getInstance().setUserToken(token);
                        log::info("token = {}", token);

                        uPopupRef->showSuccessMessage("Success! Argon token saved.");
                        return;
                    }
                }
            );
        })
        .pos({ CELL_SIZE.width - 26, CELL_SIZE.height / 2 })
        .parent(argonCell)
        .collect();

    list->addCell(argonCell);

    list->addCell(makeHeaderCell("Pet Settings"));

    list->addCell(makeToggleCell(
        "Disable Pet",
        "Removes Grinding Pet from the mod entirely.",
        "disable-pet",
        this
    ));

    list->updateLayout();
    list->getScrollLayer()->m_contentLayer->updateLayout();
    list->scrollToTop();
    return;
}

arc::Future<> SettingsLayer::onRequestStaffAccessClicked() {
    auto uPopup = UploadActionPopup::create(nullptr, "Loading...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    auto parsed = co_await BackendManager::getInstance().requestStaffAccess();

    if (!uPopupRef) co_return;

    if (!parsed.ok) {
        log::error("req failed");
        uPopupRef->showFailMessage("Failed! Try again later.");
        DataManager::getInstance().setUserPosition(GrindPosition::User);
        co_return;
    }

    int pos = parsed.pos;

    if (pos == 1) {
        uPopupRef->showSuccessMessage("Success! Helper granted.");
        DataManager::getInstance().setUserPosition(GrindPosition::Helper);
    } else if (pos == 2) {
        uPopupRef->showSuccessMessage("Success! Admin granted.");
        DataManager::getInstance().setUserPosition(GrindPosition::Admin);
    } else if (pos == 3) {
        uPopupRef->showSuccessMessage("Success! Owner granted.");
        DataManager::getInstance().setUserPosition(GrindPosition::Owner);
    } else {
        uPopupRef->showFailMessage("Failed! User is not staff.");
        DataManager::getInstance().setUserPosition(GrindPosition::User);
    }

    co_return;
}

arc::Future<> SettingsLayer::onSyncClicked() {
    auto uPopup = UploadActionPopup::create(nullptr, "Syncing data...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    auto parsed = co_await BackendManager::getInstance().bootupGet();

    if (!uPopupRef) co_return;

    if (!parsed.ok) {
        log::error("req failed");
        uPopupRef->showFailMessage("Failed! Try again later.");
        co_return;
    }

    DataManager::getInstance().setSharedData(parsed);
    uPopupRef->showSuccessMessage("Success! Data synced.");
    co_return;
}

void SettingsLayer::onTab(CCObject* sender) {
    auto currentTab = typeinfo_cast<TabButton*>(sender);

    for (size_t i = 0; i < m_tabs.size(); i++) {
        auto tab = m_tabs[i];
        auto list = m_listNodes[i];
        auto scrollLayer = list ? list->getScrollLayer() : nullptr;

        if (tab == currentTab) {
            tab->toggle(true);
            list->setVisible(true);
            if (scrollLayer) {
                scrollLayer->setTouchEnabled(true);
                scrollLayer->setMouseEnabled(true);
            }
            m_currentList = list;
        } else {
            tab->toggle(false);
            list->setVisible(false);
            if (scrollLayer) {
                scrollLayer->setTouchEnabled(false);
                scrollLayer->setMouseEnabled(false);
            }
        }
    }
}

}