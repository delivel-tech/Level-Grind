#include "LGSettingsLayer.hpp"

#include "../other/LGManager.hpp"

#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include "Geode/ui/BasedButton.hpp"
#include "Geode/ui/BasedButtonSprite.hpp"
#include <Geode/ui/ColorPickPopup.hpp>
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"

#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <matjson.hpp>
#include <cue/Colors.hpp>
#include <cue/ListBorder.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

LGSettingsLayer* LGSettingsLayer::create() {
    auto ret = new LGSettingsLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGSettingsLayer::init() {
    if (!BaseLayer::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    for (size_t i = 0; i < 3; i++) {
        auto tab = cue::ListNode::create({ 356.f, 220.f }, cue::Brown, cue::ListBorderStyle::SlimLevels);
        tab->setZOrder(6);
        tab->setPosition(winSize / 2.f);

        this->addChild(tab);

        tab->setVisible(false);

        m_tabs.push_back(tab);
        tab->setAutoUpdate(true);
    }

    m_grindTab = m_tabs[0];
    m_appearanceTab = m_tabs[1];
    m_staffTab = m_tabs[2];

    m_grindTab->setVisible(true);
    m_currentTab = m_grindTab;

    makeTabs();
    if (m_grindTabBtn) {
        m_grindTabBtn->toggle(true);
    }

    createGrindTab();
    createAppearanceTab();
    createStaffTab();

    auto resetSpr = CCSprite::createWithSpriteFrameName("GJ_deleteSongBtn_001.png");
    auto resetBtn = CCMenuItemSpriteExtra::create(
        resetSpr,
        this,
        menu_selector(LGSettingsLayer::onResetSettings)
    );
    resetBtn->setID("reset-settings-btn");

    auto resetMenu = CCMenu::create();
    resetMenu->setPosition({ 0.f, 0.f });
    resetMenu->setZOrder(7);
    resetBtn->setPosition({ winSize.width - 22.f, 22.f });
    resetMenu->addChild(resetBtn);
    this->addChild(resetMenu);

    setMouseEnabled(true);

    return true;
}

void LGSettingsLayer::scrollWheel(float y, float x) {
    if (!m_currentTab || !m_currentTab->isVisible()) {
        return;
    }

    auto scrollLayer = m_currentTab->getScrollLayer();
    if (!scrollLayer || !scrollLayer->m_contentLayer) {
        return;
    }

    auto content = scrollLayer->m_contentLayer;
    auto visibleHeight = content->getParent()->getScaledContentHeight();
    auto contentHeight = content->getScaledContentHeight();
    if (contentHeight <= visibleHeight + 0.01f) {
        return;
    }

    auto minY = visibleHeight - contentHeight;
    auto maxY = 0.f;
    auto nextY = content->getPositionY() + y * 18.f;
    content->setPositionY(std::clamp(nextY, minY, maxY));
}

void LGSettingsLayer::makeTabs() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto tabsY = winSize.height / 2.f + m_grindTab->getContentSize().height / 2.f + 30.f;

    if (!Mod::get()->getSavedValue<bool>("disable-star-particles")) {
        auto grindParticles = CCParticleSnow::create();
        auto texture = CCTextureCache::sharedTextureCache()->addImage("GJ_bigStar_noShadow.png"_spr, true);
        grindParticles->m_fStartSpin = 0.f;
        grindParticles->m_fEndSpin = 180.f;
        grindParticles->m_fStartSize = 6.f;
        grindParticles->m_fEndSize = 3.f;
        grindParticles->setTexture(texture);

        this->addChild(grindParticles);
    }

    auto tabsMenu = CCMenu::create();
    tabsMenu->setLayout(RowLayout::create()->setGap(25.f));

    tabsMenu->setPosition({ winSize.width / 2.f, tabsY });
    tabsMenu->setZOrder(5);

    this->addChild(tabsMenu);

    auto grindTabBtn = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Grind",
        this,
        menu_selector(LGSettingsLayer::onTabBtn)
    );

    auto appearanceTabBtn = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Appearance",
        this,
        menu_selector(LGSettingsLayer::onTabBtn)
    );

    auto staffTabBtn = TabButton::create(
        TabBaseColor::Unselected,
        TabBaseColor::UnselectedDark,
        "Staff",
        this,
        menu_selector(LGSettingsLayer::onTabBtn)
    );

    m_tabButtons.push_back(grindTabBtn);
    m_tabButtons.push_back(appearanceTabBtn);
    m_tabButtons.push_back(staffTabBtn);

    m_grindTabBtn = m_tabButtons[0];
    m_appearanceTabBtn = m_tabButtons[1];
    m_staffTabBtn = m_tabButtons[2];

    tabsMenu->addChild(grindTabBtn);
    tabsMenu->addChild(appearanceTabBtn);
    tabsMenu->addChild(staffTabBtn);

    tabsMenu->updateLayout();
}

void LGSettingsLayer::onTabBtn(CCObject* sender) {
    auto currentTabBtn = typeinfo_cast<TabButton*>(sender);

    for (size_t i = 0; i < m_tabButtons.size(); i++) {
        auto tabBtn = m_tabButtons[i];
        auto tab = m_tabs[i];

        if (tabBtn == currentTabBtn) {
            tabBtn->toggle(true);
            tab->setVisible(true);
            m_currentTab = tab;
        }
        else {
            tabBtn->toggle(false);
            tab->setVisible(false);
        }
    }
}

CCMenu* LGSettingsLayer::makeToggleCell(
    const char* title,
    const char* desc,
    const char* settingKey,
    cocos2d::SEL_MenuHandler cb,
    cocos2d::CCNode* target
) {
    auto cell = CCMenu::create();
    cell->ignoreAnchorPointForPosition(false);
    cell->setContentSize(CELL_SIZE);

    auto label = CCLabelBMFont::create(title, "bigFont.fnt");
    label->limitLabelWidth(CELL_SIZE.width * 0.62f, 0.5f, 0.1f);
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ 8.f, CELL_SIZE.height / 2.f });
    cell->addChild(label);

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.5f);

    auto info = CCMenuItemExt::createSpriteExtra(
        infoSpr,
        [title, desc](CCObject*) { FLAlertLayer::create(title, desc, "OK")->show(); }
    );
    info->setPosition({
        8.f + label->getScaledContentWidth() + 10.f,
        CELL_SIZE.height / 2.f
    });
    cell->addChild(info);

    auto toggler = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        target,
        cb
    );
    toggler->setPosition({ CELL_SIZE.width - 18.f, CELL_SIZE.height / 2.f });
    toggler->setScale(0.7f);
    toggler->toggle(Mod::get()->getSavedValue<bool>(settingKey));
    toggler->setID(settingKey);
    m_boolTogglers[settingKey] = toggler;
    cell->addChild(toggler);

    return cell;
}

CCMenu* LGSettingsLayer::makeFloatCell(
    const char* title,
    const char* desc,
    const char* settingKey,
    float min,
    float max
) {
    auto cell = CCMenu::create();
    cell->ignoreAnchorPointForPosition(false);
    cell->setContentSize({ CELL_SIZE.width, 44.f });

    auto label = CCLabelBMFont::create(title, "bigFont.fnt");
    label->limitLabelWidth(CELL_SIZE.width * 0.45f, 0.5f, 0.1f);
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ 8.f, 22.f });
    cell->addChild(label);

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.5f);
    auto info = CCMenuItemExt::createSpriteExtra(
        infoSpr,
        [title, desc](CCObject*) { FLAlertLayer::create(title, desc, "OK")->show(); }
    );
    info->setPosition({ 8.f + label->getScaledContentWidth() + 10.f, 22.f });
    cell->addChild(info);

    auto makeArrowBtn = [&](bool isRight, bool isBig, float step) {
        auto arrowSprite = CCSprite::createWithSpriteFrameName(isBig ? "GJ_arrow_03_001.png" : "GJ_arrow_01_001.png");
        arrowSprite->setScale(isBig ? 0.36f : 0.52f);
        if (isRight) {
            arrowSprite->setFlipX(true);
        }

        auto btn = CCMenuItemSpriteExtra::create(
            arrowSprite,
            this,
            menu_selector(LGSettingsLayer::onBackgroundSpeedArrow)
        );
        m_speedArrowSteps[btn] = step;
        cell->addChild(btn);
        return btn;
    };

    m_bigSpeedLeftBtn = makeArrowBtn(false, true, -0.5f);
    m_speedLeftBtn = makeArrowBtn(false, false, -0.1f);
    m_speedRightBtn = makeArrowBtn(true, false, 0.1f);
    m_bigSpeedRightBtn = makeArrowBtn(true, true, 0.5f);

    m_bigSpeedLeftBtn->setPosition({ CELL_SIZE.width - 148.f, 22.f });
    m_speedLeftBtn->setPosition({ CELL_SIZE.width - 128.f, 22.f });
    m_speedRightBtn->setPosition({ CELL_SIZE.width - 43.f, 22.f });
    m_bigSpeedRightBtn->setPosition({ CELL_SIZE.width - 23.f, 22.f });

    m_backgroundSpeedInput = TextInput::create(90.f, "0.00");
    m_backgroundSpeedInput->setScale(0.6f);
    m_backgroundSpeedInput->setCommonFilter(CommonFilter::Float);
    m_backgroundSpeedInput->setPosition({ CELL_SIZE.width - 85.f, 22.f });
    m_backgroundSpeedInput->setCallback([this](std::string const& str) {
        if (str.empty() || str == "-" || str == "." || str == "-.") {
            return;
        }

        if (auto parsed = numFromString<float>(str)) {
            this->updateBackgroundSpeedUI(parsed.unwrap(), m_backgroundSpeedInput);
        }
        else {
            this->updateBackgroundSpeedUI(Mod::get()->getSavedValue<float>("background-speed"), nullptr);
        }
    });
    cell->addChild(m_backgroundSpeedInput);

    if (std::string(settingKey) == "background-speed") {
        updateBackgroundSpeedUI(std::clamp(Mod::get()->getSavedValue<float>(settingKey), min, max), nullptr);
    }

    return cell;
}

CCMenu* LGSettingsLayer::makeColorCell(
    const char* title,
    const char* desc,
    cocos2d::SEL_MenuHandler cb,
    cocos2d::CCNode* target
) {
    auto cell = CCMenu::create();
    cell->ignoreAnchorPointForPosition(false);
    cell->setContentSize({ CELL_SIZE.width, 34.f });

    auto label = CCLabelBMFont::create(title, "bigFont.fnt");
    label->setScale(0.45f);
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ 8.f, 17.f });
    cell->addChild(label);

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.42f);
    auto info = CCMenuItemExt::createSpriteExtra(
        infoSpr,
        [title, desc](CCObject*) { FLAlertLayer::create(title, desc, "OK")->show(); }
    );
    info->setPosition({ 8.f + label->getScaledContentWidth() + 10.f, 17.f });
    cell->addChild(info);

    m_bgPreview = ColorChannelSprite::create();
    m_bgPreview->setScale(0.58f);

    auto colorPreviewBtn = CCMenuItemSpriteExtra::create(
        m_bgPreview,
        target,
        cb
    );
    colorPreviewBtn->setPosition({ CELL_SIZE.width - 17.f, 17.f });
    colorPreviewBtn->setID("background-color-preview-btn");
    cell->addChild(colorPreviewBtn);

    return cell;
}

CCMenu* LGSettingsLayer::makeHeader(const char* title) {
    auto headerCell = CCMenu::create();
    headerCell->ignoreAnchorPointForPosition(false);
    headerCell->setContentSize({ CELL_SIZE.width, 24.f });

    auto headerLabel = CCLabelBMFont::create(title, "goldFont.fnt");
    headerLabel->setScale(0.5f);
    headerLabel->setPosition({ CELL_SIZE.width / 2.f, 12.f });
    headerCell->addChild(headerLabel);

    return headerCell;
}

void LGSettingsLayer::createGrindTab() {
    auto tab = m_grindTab;

    tab->addCell(makeHeader("Grind Settings"));

    tab->addCell(makeToggleCell(
        "Only Uncompleted",
        "If enabled, only uncompleted levels are shown.",
        "only-uncompleted",
        menu_selector(LGSettingsLayer::onUncompletedToggled),
        this
    ));

    tab->addCell(makeToggleCell(
        "Only Completed",
        "If enabled, only completed levels are shown.",
        "only-completed",
        menu_selector(LGSettingsLayer::onCompletedToggled),
        this
    ));

    tab->addCell(makeToggleCell(
        "Newer First",
        "If enabled, newly created levels will be shown first.",
        "newer-first",
        menu_selector(LGSettingsLayer::onNewerFirst),
        this
    ));

    tab->addCell(makeToggleCell(
        "Recently Added",
        "If enabled, recently added levels into the database will be shown first.",
        "recently-added",
        menu_selector(LGSettingsLayer::onRecentlyAdded),
        this
    ));

    tab->addCell(makeToggleCell(
        "Hide Progress Bar",
        "If enabled, progress bar on level browser layer will be hidden.",
        "hide-bar",
        menu_selector(LGSettingsLayer::onHideProgressBar),
        this
    ));

    tab->addCell(makeToggleCell(
        "Hide Completion Info",
        "If enabled, completion info will be hidden on level browser layer.",
        "hide-completion-info",
        menu_selector(LGSettingsLayer::onHideCompletionInfo),
        this
    ));

    tab->updateLayout();
}

void LGSettingsLayer::createAppearanceTab() {
    auto tab = m_appearanceTab;

    tab->addCell(makeHeader("Background Settings"));

    tab->addCell(makeToggleCell(
        "Disable Star Particles",
        "Disables star particles at the background of grinding levels layer.",
        "disable-star-particles",
        menu_selector(LGSettingsLayer::onDisableStarParticles),
        this
    ));

    tab->addCell(makeColorCell(
        "Background Color",
        "Customize the background color using the native color picker.",
        menu_selector(LGSettingsLayer::onBackgroundColorPick),
        this
    ));

    tab->addCell(makeFloatCell(
        "Background Speed",
        "Changes the speed of the background from 0.00 to 5.00.",
        "background-speed",
        BG_SPEED_MIN,
        BG_SPEED_MAX
    ));

    tab->addCell(makeToggleCell(
        "Disable Custom Background",
        "Disables the custom background color and sets it to default.",
        "disable-custom-background",
        menu_selector(LGSettingsLayer::onDisableCustomBackground),
        this
    ));

    tab->addCell(makeHeader("Badge Settings"));

    tab->addCell(makeToggleCell(
        "No Grind Badges for GD Mods",
        "Hides the Helper / Admin badge for GD Staff Team.",
        "no-badge-for-mods",
        menu_selector(LGSettingsLayer::onNoBadgeForMods),
        this
    ));

    tab->addCell(makeToggleCell(
        "Disable Badges",
        "Hides all badges added by the mod.",
        "disable-badges",
        menu_selector(LGSettingsLayer::onDisableBadges),
        this
    ));

    refreshBackgroundColorUI();
    tab->updateLayout();
}

void LGSettingsLayer::createStaffTab() {
    auto tab = m_staffTab;

    tab->addCell(makeHeader("Staff Settings"));

    auto reqCell = CCMenu::create();
    reqCell->ignoreAnchorPointForPosition(false);
    reqCell->setContentSize(CELL_SIZE);

    auto label = CCLabelBMFont::create("Request Staff Access", "bigFont.fnt");
    label->limitLabelWidth(CELL_SIZE.width * 0.6f, 0.5f, 0.1f);
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ 8.f, CELL_SIZE.height / 2.f });
    reqCell->addChild(label);

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.5f);
    auto info = CCMenuItemExt::createSpriteExtra(
        infoSpr,
        [](CCObject*) {
            FLAlertLayer::create(
                "Request Staff Access",
                "Gives you Helper / Admin perms if you're a staff member.",
                "OK"
            )->show();
        }
    );
    info->setPosition({ 8.f + label->getScaledContentWidth() + 10.f, CELL_SIZE.height / 2.f });
    reqCell->addChild(info);

    auto reqBtnSpr = ButtonSprite::create("Req", "goldFont.fnt", "GJ_button_04.png", 0.8f);
    reqBtnSpr->setScale(0.55f);
    auto reqBtn = CCMenuItemExt::createSpriteExtra(reqBtnSpr, [](CCObject*) {
        matjson::Value body;
        body["account_id"] = GJAccountManager::get()->m_accountID;
        body["token"] = LGManager::get()->getArgonToken();

        web::WebRequest req;
        req.bodyJSON(body);

        auto upopup = UploadActionPopup::create(nullptr, "Loading...");
        upopup->show();

        Ref<UploadActionPopup> popupRef = upopup;

        async::spawn(
            req.post("https://delivel.tech/grindapi/check_helper_new"),
            [popupRef](web::WebResponse res) {
                if (!popupRef) return;
                if (!res.ok()) {
                    log::error("req failed");
                    popupRef->showFailMessage("Request failed! Try again later.");
                    LGManager::get()->setHelper(false);
                    LGManager::get()->setAdmin(false);
                    return;
                }

                auto json = res.json().unwrapOrDefault();
                auto position = json["pos"].asInt().unwrapOrDefault();
                if (position == 1) {
                    popupRef->showSuccessMessage("Success! Helper granted.");
                    LGManager::get()->setHelper(true);
                    LGManager::get()->setAdmin(false);
                } else if (position == 2) {
                    popupRef->showSuccessMessage("Success! Admin granted.");
                    LGManager::get()->setAdmin(true);
                    LGManager::get()->setHelper(false);
                } else {
                    popupRef->showFailMessage("Failed! User is not a helper.");
                    LGManager::get()->setHelper(false);
                    LGManager::get()->setAdmin(false);
                }
            }
        );
    });
    reqBtn->setPosition({ CELL_SIZE.width - 22.f, CELL_SIZE.height / 2.f });
    reqCell->addChild(reqBtn);

    tab->addCell(reqCell);
    tab->updateLayout();
}

bool LGSettingsLayer::getIncomingToggleValue(CCObject* sender) const {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) {
        return false;
    }

    std::string settingKey;
    for (auto const& [key, ptr] : m_boolTogglers) {
        if (ptr == toggler) {
            settingKey = key;
            break;
        }
    }

    auto newValue = settingKey.empty()
        ? !toggler->isToggled()
        : !Mod::get()->getSavedValue<bool>(settingKey.c_str());
    return newValue;
}

void LGSettingsLayer::setBoolSetting(char const* key, bool value, bool syncUI) {
    Mod::get()->setSavedValue(key, value);

    if (syncUI) {
        auto it = m_boolTogglers.find(key);
        if (it != m_boolTogglers.end() && it->second) {
            it->second->toggle(value);
        }
    }
}

void LGSettingsLayer::refreshBackgroundColorUI() {
    auto color = Mod::get()->getSavedValue<cocos2d::ccColor3B>("rgbBackground");

    if (m_bgPreview) {
        m_bgPreview->setColor(color);
    }

    auto speed = std::clamp(Mod::get()->getSavedValue<float>("background-speed"), BG_SPEED_MIN, BG_SPEED_MAX);
    updateBackgroundSpeedUI(speed, nullptr);
}

void LGSettingsLayer::applyDefaultSettings() {
    Mod::get()->setSavedValue("only-uncompleted", false);
    Mod::get()->setSavedValue("only-completed", false);
    Mod::get()->setSavedValue("newer-first", true);
    Mod::get()->setSavedValue("recently-added", false);
    Mod::get()->setSavedValue("hide-bar", false);
    Mod::get()->setSavedValue("hide-completion-info", false);

    Mod::get()->setSavedValue("disable-star-particles", false);
    Mod::get()->setSavedValue("rgbBackground", cocos2d::ccColor3B { 32, 69, 125 });
    Mod::get()->setSavedValue("background-speed", 0.5f);
    Mod::get()->setSavedValue("disable-custom-background", false);
    Mod::get()->setSavedValue("no-badge-for-mods", true);
    Mod::get()->setSavedValue("disable-badges", false);
}

void LGSettingsLayer::onResetSettings(CCObject* sender) {
    createQuickPopup(
        "Reset Settings",
        "Are you sure you want to reset all settings to default values?",
        "Cancel",
        "Reset",
        [this](auto, bool reset) {
            if (!reset) {
                return;
            }

            applyDefaultSettings();

            for (auto const& [key, toggler] : m_boolTogglers) {
                if (toggler) {
                    toggler->toggle(Mod::get()->getSavedValue<bool>(key.c_str()));
                }
            }

            refreshBackgroundColorUI();
            Notification::create("Settings were reset", NotificationIcon::Success)->show();
        }
    );
}

void LGSettingsLayer::updateBackgroundSpeedUI(float speed, cocos2d::CCNode* source) {
    auto value = std::clamp(speed, BG_SPEED_MIN, BG_SPEED_MAX);
    value = std::round(value * 100.f) / 100.f;

    Mod::get()->setSavedValue("background-speed", value);

    if (m_backgroundSpeedInput && source != m_backgroundSpeedInput) {
        m_backgroundSpeedInput->setString(numToString(value), false);
    }

    auto atMin = value <= BG_SPEED_MIN + 0.0001f;
    auto atMax = value >= BG_SPEED_MAX - 0.0001f;

    if (m_bigSpeedLeftBtn) m_bigSpeedLeftBtn->setEnabled(!atMin);
    if (m_speedLeftBtn) m_speedLeftBtn->setEnabled(!atMin);
    if (m_speedRightBtn) m_speedRightBtn->setEnabled(!atMax);
    if (m_bigSpeedRightBtn) m_bigSpeedRightBtn->setEnabled(!atMax);
}

void LGSettingsLayer::onUncompletedToggled(CCObject* sender) {
    auto value = getIncomingToggleValue(sender);
    setBoolSetting("only-uncompleted", value, false);
    if (value) {
        setBoolSetting("only-completed", false, true);
    }
}

void LGSettingsLayer::onCompletedToggled(CCObject* sender) {
    auto value = getIncomingToggleValue(sender);
    setBoolSetting("only-completed", value, false);
    if (value) {
        setBoolSetting("only-uncompleted", false, true);
    }
}

void LGSettingsLayer::onNewerFirst(CCObject* sender) {
    setBoolSetting("newer-first", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onRecentlyAdded(CCObject* sender) {
    setBoolSetting("recently-added", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onHideProgressBar(CCObject* sender) {
    setBoolSetting("hide-bar", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onHideCompletionInfo(CCObject* sender) {
    setBoolSetting("hide-completion-info", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onDisableStarParticles(CCObject* sender) {
    setBoolSetting("disable-star-particles", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onDisableCustomBackground(CCObject* sender) {
    setBoolSetting("disable-custom-background", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onNoBadgeForMods(CCObject* sender) {
    setBoolSetting("no-badge-for-mods", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onDisableBadges(CCObject* sender) {
    setBoolSetting("disable-badges", getIncomingToggleValue(sender), false);
}

void LGSettingsLayer::onBackgroundSpeedArrow(CCObject* sender) {
    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    if (!btn) {
        return;
    }

    auto it = m_speedArrowSteps.find(btn);
    if (it == m_speedArrowSteps.end()) {
        return;
    }

    auto value = Mod::get()->getSavedValue<float>("background-speed") + it->second;
    updateBackgroundSpeedUI(value, btn);
}

void LGSettingsLayer::onBackgroundColorPick(CCObject* sender) {
    (void)sender;

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
