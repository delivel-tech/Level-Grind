#include "MainLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <UIBuilder.hpp>
#include <arc/future/Future.hpp>
#include <fmt/format.h>

#include "../../features/levels/LevelTypes.hpp"
#include "../../features/events/EventTypes.hpp"
#include "../../core/BrowserTypes.hpp"
#include "../../utils/utils.hpp"
#include "../../ui/popups/StaffPopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../ui/popups/WeeklyAchievementPopup.hpp"
#include "../popups/GuidePopup.hpp"
#include "../popups/AnnouncementsPopup.hpp"
#include "../popups/EventPopup.hpp"
#include "CustomBrowserLayer.hpp"

#include "../popups/DifficultySelectorPopup.hpp"
#include "../popups/DiscordPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"
#include "SettingsLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

struct DifficultyFaceInfo {
    const char* spriteFrame;
    const char* id;
    cocos2d::SEL_MenuHandler callback;
};

bool hasDifficulty(const std::vector<int>& values, int value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void applyDifficultyPair(std::vector<int>& values, int first, int second, bool enabled, bool firstSelected, bool secondSelected) {
    safeErase(values, first);
    safeErase(values, second);

    if (!enabled) return;

    if (firstSelected) safeAdd(values, first);
    if (secondSelected) safeAdd(values, second);
}

MainLayer* MainLayer::create() {
    auto ret = new MainLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MainLayer::init() {
    if (!BaseLayer::init()) return false;
    if (!initMainPanel()) return false;

    this->scheduleUpdate();

    return true;
}

bool MainLayer::initMainPanel() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto levelGrindLogo = Build<CCSprite>::create("lg-logo.png"_spr)
        .scale(1.1f)
        .pos({ winSize.width / 2, (winSize.height / 4) * 3.3f + 14.f })
        .id("level-grind-logo")
        .parent(this)
        .collect();

    if (!levelGrindLogo) return false;

    auto panel = Build(NineSlice::create("GJ_square02.png"))
        .id("search-panel")
        .contentSize({210.f, 140.f})
        .center()
        .parent(this)
        .collect();

    auto optionsPanelFirst = Build(NineSlice::create("square02_small.png"))
        .id("options-panel-first")
        .contentSize({ 284.5f, 30.f })
        .pos({ winSize.width / 2, winSize.height / 4.6f })
        .opacity(100)
        .parent(this)
        .collect();

    auto optionsPanelSecond = Build(NineSlice::create("square02_small.png"))
        .id("options-panel-second")
        .contentSize({ 284.5f, 30.f })
        .pos({ winSize.width / 2, winSize.height / 10.f })
        .opacity(100)
        .parent(this)
        .collect();
    
    if (!optionsPanelFirst) return false;
    if (!optionsPanelSecond) return false;

    auto ratingsMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(28.f))
        .parent(this)
        .pos(optionsPanelFirst->getPosition())
        .id("ratings-menu")
        .scale(0.65f)
        .collect();

    if (!ratingsMenu) return false;

    std::vector<DifficultyFaceInfo> difficulties = {
        {"diffIcon_auto_btn_001.png", "auto-difficulty-toggler", makeCb(MainLayer::onAutoToggler)},
        { "diffIcon_01_btn_001.png", "easy-difficulty-toggler", makeCb(MainLayer::onEasyToggler) },
        { "diffIcon_02_btn_001.png", "normal-difficulty-toggler", makeCb(MainLayer::onNormalToggler) },
        { "diffIcon_03_btn_001.png", "hard-difficulty-toggler", makeCb(MainLayer::onHardToggler) },
        { "diffIcon_04_btn_001.png", "harder-difficulty-toggler", makeCb(MainLayer::onHarderToggler) },
        { "diffIcon_05_btn_001.png", "insane-difficulty-toggler", makeCb(MainLayer::onInsaneToggler) }
    };

    for (auto const& difficulty : difficulties) {
        auto spriteOff = Build(CCSprite::createWithSpriteFrameName(difficulty.spriteFrame))
            .color({ 100, 100, 100 })
            .collect();
        auto spriteOn = Build(CCSprite::createWithSpriteFrameName(difficulty.spriteFrame))
            .collect();

        Build(CCMenuItemToggler::create(spriteOff, spriteOn, this, difficulty.callback))
            .id(difficulty.id)
            .parent(ratingsMenu)
            .with([](CCMenuItemToggler* toggler) {
                auto children = toggler->getChildren();

                for (auto& obj : CCArrayExt(children)) {
                    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
                    btn->m_scaleMultiplier = 1.1f;
                }
            })
            .collect();
    }

    ratingsMenu->updateLayout();

    auto selectorMenu = Build<CCMenu>::create()
        .parent(this)
        .pos(optionsPanelFirst->getPosition() + CCPoint { optionsPanelFirst->getScaledContentWidth() / 2.f - 18.f, 0.f })
        .id("difficulty-selector-menu")
        .collect();

    if (!selectorMenu) return false;

    Build(CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png"),
        this,
        makeCb(MainLayer::onOpenDifficultySelector)
    ))
        .id("difficulty-selector-btn")
        .parent(selectorMenu)
        .collect();

    selectorMenu->setVisible(false);

    auto lengthsMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(15))
        .parent(this)
        .pos(optionsPanelSecond->getPosition())
        .id("lengths-menu")
        .scale(0.6f)
        .collect();

    std::vector<LengthInfo> lengths = {
        { Length::Short, "Short", "short-length", makeCb(MainLayer::onShortToggler) },
        { Length::Medium, "Medium", "medium-length", makeCb(MainLayer::onMediumToggler) },
        { Length::Long, "Long", "long-length", makeCb(MainLayer::onLongToggler) },
        { Length::XL, "XL", "xl-length", makeCb(MainLayer::onXLToggler) }
    };

    for (const auto& length : lengths) {
        auto labelOn = Build(CCLabelBMFont::create(length.lengthName.c_str(), "bigFont.fnt"))
            .collect();
        auto labelOff = Build(CCLabelBMFont::create(length.lengthName.c_str(), "bigFont.fnt"))
            .color({ 100, 100, 100 })
            .collect();

        auto lengthToggler = Build(CCMenuItemToggler::create(labelOff, labelOn, this, length.cb))
            .parent(lengthsMenu)
            .id(length.lengthId)
            .with([](CCMenuItemToggler* toggler) {
                auto children = toggler->getChildren();

                for (auto& obj : CCArrayExt(children)) {
                    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
                    btn->m_scaleMultiplier = 1.1f;
                }
            })
            .collect();
    }

    lengthsMenu->updateLayout();

    auto versionsPanel = Build(NineSlice::create("square02_small.png"))
        .contentSize({ 30.f, winSize.height / 2.3f })
        .id("versions-panel")
        .parent(this)
        .opacity(100)
        .pos({ (winSize.width / 2) - 128.f, winSize.height / 2 })
        .collect();

    auto versionsMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()
                                ->setGap(25.f)
                                ->setAxisReverse(true))
        .parent(this)
        .pos(versionsPanel->getPosition())
        .id("versions-menu")
        .scale(0.4f)
        .collect();

    std::vector<VersionInfo> versions = {
        {22, "2.2", "version-22", makeCb(MainLayer::onVer22Toggler)},
        {21, "2.1", "version-21", makeCb(MainLayer::onVer21Toggler)},
        {20, "2.0", "version-20", makeCb(MainLayer::onVer20Toggler)},
        {19, "1.9", "version-19", makeCb(MainLayer::onVer19Toggler)},
        {18, "<1.9", "version-lower-than-19", makeCb(MainLayer::onVerLower19Toggler)}
    };

    for (const auto& version : versions) {
        auto labelOn = Build(CCLabelBMFont::create(version.versionName.c_str(), "bigFont.fnt"))
            .collect();
        auto labelOff = Build(CCLabelBMFont::create(version.versionName.c_str(), "bigFont.fnt"))
            .color({ 100, 100, 100 })
            .collect();

        auto versionToggler = Build(CCMenuItemToggler::create(labelOff, labelOn, this, version.cb))
            .parent(versionsMenu)
            .id(version.versionId)
            .with([](CCMenuItemToggler* toggler) {
                auto children = toggler->getChildren();

                for (auto& obj : CCArrayExt(children)) {
                    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
                    btn->m_scaleMultiplier = 1.1f;
                }
            })
            .collect();
    }

    versionsMenu->updateLayout();

    auto filtersLabel = Build(CCLabelBMFont::create("Filters", "bigFont.fnt"))
        .id("filters-label")
        .parent(this)
        .pos({ winSize.width / 2, (winSize.height / 4) * 2.55f })
        .collect();

    auto searchMenu = Build(CCMenu::create())
        .scale(0.8f)
        .layout(RowLayout::create())
        .pos({ winSize.width / 2, (winSize.height / 4) * 1.5f })
        .id("search-menu")
        .parent(this)
        .collect();

    auto searchBtn = Build(ButtonSprite::create("Search", "goldFont.fnt", "GJ_button_01.png"))
        .scale(0.8f)
        .intoMenuItem([this] {
            GetLevelsBody body {
                m_difficulties, m_lengths, m_demonDifficulties,
                m_grindTypes, m_versions, Mod::get()->getSavedValue<bool>("newer-first"), Mod::get()->getSavedValue<bool>("recently-added")
            };
            CustomBrowserLayer::create(body, "Grinding Levels", CustomBrowserType::Search, EventType::Daily)->open(); // eventtype is just a placeholder
        })
        .id("search-btn")
        .scaleMult(1.1f)
        .parent(searchMenu)
        .collect();

    auto randomBtn = Build<CCSprite>::create("random_btn.png"_spr)
        .scale(0.55f)
        .intoMenuItem([this] {
            GetLevelsBody body {
                m_difficulties, m_lengths, m_demonDifficulties,
                m_grindTypes, m_versions, true, false
            };

            auto ll = levelgrind::LoadingLayer::create("Loading random level...");
            ll->show();

            m_randomLoadingLayer = Ref(ll);

            auto loadingRef = Ref(ll);
            auto self = Ref(this);

            auto resetState = [loadingRef, self]() {
                self->m_randomPending = false;
                self->m_randomTimer = 0.f;
                self->m_randomLevelID = -1;
                self->m_randomKey.clear();
                if (loadingRef) loadingRef->hide();
                self->m_randomLoadingLayer = nullptr;
            };

            m_listener.spawn(
                APIClient::getInstance().getLevels(body),
                [self, loadingRef, resetState](web::WebResponse res) {
                    if (!self) {
                        resetState();
                        return;
                    }

                    auto parsed = APIClient::getInstance().getLevelsParse(res);

                    if (!parsed.ok) {
                        resetState();
                        Notification::create("Failed to get random level", NotificationIcon::Error)->show();
                        return;
                    }

                    if (parsed.ids.empty()) {
                        Notification::create("No levels found", NotificationIcon::Info)->show();
                        resetState();
                        return;
                    }

                    std::shuffle(parsed.ids.begin(), parsed.ids.end(), std::mt19937{ std::random_device{}() });

                    bool onlyUncompleted = false;
                    bool onlyCompleted = false;
                    if (auto mod = Mod::get()) {
                        onlyUncompleted = mod->getSavedValue<bool>("only-uncompleted");
                        onlyCompleted = mod->getSavedValue<bool>("only-completed");
                    }

                    auto gsm = GameStatsManager::sharedState();

                    int chosenID = -1;
                    for (auto id : parsed.ids) {
                        if (gsm) {
                            bool isCompleted = gsm->hasCompletedOnlineLevel(id);
                            if (onlyUncompleted && isCompleted) continue;
                            if (onlyCompleted && !isCompleted) continue;
                        }
                        chosenID = id;
                        break;
                    }

                    if (chosenID <= 0) {
                        if (onlyUncompleted)
                            Notification::create("No uncompleted levels found", NotificationIcon::Info)->show();
                        else if (onlyCompleted)
                            Notification::create("No completed levels found", NotificationIcon::Info)->show();
                        else
                            Notification::create("No levels found", NotificationIcon::Info)->show();
                        resetState();
                        return;
                    }

                    auto searchObj = GJSearchObject::create(SearchType::Search, numToString(chosenID));
                    auto key = searchObj->getKey();
                    auto glm = GameLevelManager::sharedState();

                    if (!glm) {
                        Notification::create("Failed to access level manager.", NotificationIcon::Error)->show();
                        resetState();
                        return;
                    }

                    auto stored = glm->getStoredOnlineLevels(key);
                    if (stored && stored->count() > 0) {
                        auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
                        if (level && level->m_levelID == chosenID) {
                            auto scene = LevelInfoLayer::scene(level, false);
                            auto trans = CCTransitionFade::create(0.5f, scene);
                            resetState();
                            CCDirector::sharedDirector()->pushScene(trans);
                            return;
                        }
                    }

                    self->m_randomTimer = 10.f;
                    self->m_randomLevelID = chosenID;
                    self->m_randomKey = key;
                    self->m_randomPending = true;

                    if (glm->m_levelManagerDelegate) {
                        glm->m_levelManagerDelegate = nullptr;
                    }
                    glm->getOnlineLevels(searchObj);
                    return;
                }
            );
        })
        .scaleMult(1.1f)
        .id("random-btn")
        .parent(searchMenu)
        .collect();

    searchMenu->updateLayout();

    auto typesBtnMenu = Build(CCMenu::create())
        .id("types-btn-menu")
        .layout(RowLayout::create()->setGap(15))
        .parent(this)
        .pos({ winSize.width / 2, (winSize.height / 4) * 2.05f })
        .scale(0.8f)
        .collect();

    std::vector<FilterInfo> filters = {
        {"star", "GJ_starsIcon_001.png", "star-toggler", makeCb(MainLayer::onStarToggler)},
        {"moon", "GJ_moonsIcon_001.png", "moon-toggler", makeCb(MainLayer::onMoonToggler)},
        {"coin", "GJ_coinsIcon2_001.png", "coin-toggler", makeCb(MainLayer::onCoinToggler)},
        {"demon", "GJ_demonIcon_001.png", "demon-toggler", makeCb(MainLayer::onDemonToggler)}
    };

    for (auto const& filter : filters) {
        auto btnSprOff = Build(CCSprite::create("GJ_button_04.png"))
            .collect();

        auto btnSprOn = Build(CCSprite::create("GJ_button_02.png"))
            .collect();

        auto top1 = Build(CCSprite::createWithSpriteFrameName(filter.top.c_str()))
            .pos({ 20, 20 })
            .parent(btnSprOff)
            .collect();

        auto top2 = Build(CCSprite::createWithSpriteFrameName(filter.top.c_str()))
            .pos({ 20, 20 })
            .parent(btnSprOn)
            .collect();

        auto filterToggler = Build(CCMenuItemToggler::create(
            btnSprOff,
            btnSprOn,
            this,
            filter.cb
        ))
            .id(filter.filterId)
            .with([](CCMenuItemToggler* toggler) {
                auto children = toggler->getChildren();

                for (auto& obj : CCArrayExt(children)) {
                    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
                    btn->m_scaleMultiplier = 1.1f;
                }
            })
            .parent(typesBtnMenu)
            .collect();

    }

    typesBtnMenu->updateLayout();

    auto demonsPanel = Build(NineSlice::create("square02_small.png"))
        .opacity(100)
        .contentSize({ 30.f, winSize.height / 2.3f })
        .pos({ (winSize.width / 2) + 128.f, winSize.height / 2 })
        .id("demons-panel")
        .visible(false)
        .parent(this)
        .collect();

    auto demonsMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setGap(25.f)->setAxisReverse(true))
        .scale(0.4f)
        .pos(demonsPanel->getPosition())
        .id("demons-menu")
        .visible(false)
        .parent(this)
        .collect();

    m_demonsFilters.first = demonsPanel;
    m_demonsFilters.second = demonsMenu;

    std::array<int, 5> ddIDs = {3, 4, 0, 5, 6};

    std::vector<DemonDifficultyInfo> demonDifficulties = {
        {ddIDs[0], "diffIcon_07_btn_001.png", "easy-demon-toggler", makeCb(MainLayer::onEasyDemonToggler)},
        {ddIDs[1], "diffIcon_08_btn_001.png", "medium-demon-toggler", makeCb(MainLayer::onMediumDemonToggler)},
        {ddIDs[2], "diffIcon_06_btn_001.png", "hard-demon-toggler", makeCb(MainLayer::onHardDemonToggler)},
        {ddIDs[3], "diffIcon_09_btn_001.png", "insane-demon-toggler", makeCb(MainLayer::onInsaneDemonToggler)},
        {ddIDs[4], "diffIcon_10_btn_001.png", "extreme-demon-toggler", makeCb(MainLayer::onExtremeDemonToggler)},
    };

    for (auto const& demonDifficulty : demonDifficulties) {
        auto sprOff = Build(CCSprite::createWithSpriteFrameName(demonDifficulty.sprite.c_str()))
            .color({ 100, 100, 100 })
            .collect();

        auto sprOn = Build(CCSprite::createWithSpriteFrameName(demonDifficulty.sprite.c_str()))
            .collect();

        auto ddToggler = Build(CCMenuItemToggler::create(
            sprOff,
            sprOn,
            this,
            demonDifficulty.cb
        ))
            .id(demonDifficulty.id)
            .with([](CCMenuItemToggler* toggler) {
                auto children = toggler->getChildren();

                for (auto& obj : CCArrayExt(children)) {
                    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(obj);
                    btn->m_scaleMultiplier = 1.1f;
                }
            })
            .parent(m_demonsFilters.second)
            .collect();
    }

    m_demonsFilters.second->updateLayout();

    if (auto mod = Mod::get()) {
        m_splitHard4 = mod->getSavedValue<bool>("split-4-enabled");
        m_splitHard5 = mod->getSavedValue<bool>("split-5-enabled");
        m_splitHarder6 = mod->getSavedValue<bool>("split-6-enabled");
        m_splitHarder7 = mod->getSavedValue<bool>("split-7-enabled");
        m_splitInsane8 = mod->getSavedValue<bool>("split-8-enabled");
        m_splitInsane9 = mod->getSavedValue<bool>("split-9-enabled");
        
        auto normalizePair = [](bool& a, bool& b) {
            if (!a && !b) {
                a = true;
                b = true;
            }
        };
        normalizePair(m_splitHard4, m_splitHard5);
        normalizePair(m_splitHarder6, m_splitHarder7);
        normalizePair(m_splitInsane8, m_splitInsane9);

        auto setTogglerState = [this](const char* id, bool on) {
            auto toggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive(id));
            if (toggler) toggler->toggle(on);
        };

        if (mod->getSavedValue<bool>("star-enabled")) {
            setTogglerState("star-toggler", true);
            m_grindTypes.push_back("star");
        }
        if (mod->getSavedValue<bool>("moon-enabled")) {
            setTogglerState("moon-toggler", true);
            m_grindTypes.push_back("moon");
        }
        if (mod->getSavedValue<bool>("coin-enabled")) {
            setTogglerState("coin-toggler", true);
            m_grindTypes.push_back("coin");
        }
        if (mod->getSavedValue<bool>("demon-enabled")) {
            setTogglerState("demon-toggler", true);
            m_grindTypes.push_back("demon");
            m_demonsFilters.first->setVisible(true);
            m_demonsFilters.second->setVisible(true);
        }

        if (mod->getSavedValue<bool>("short-enabled")) { setTogglerState("short-length", true); m_lengths.push_back(int(Length::Short)); }
        if (mod->getSavedValue<bool>("medium-enabled")) { setTogglerState("medium-length", true); m_lengths.push_back(int(Length::Medium)); }
        if (mod->getSavedValue<bool>("long-enabled")) { setTogglerState("long-length", true); m_lengths.push_back(int(Length::Long)); }
        if (mod->getSavedValue<bool>("xl-enabled")) { setTogglerState("xl-length", true); m_lengths.push_back(int(Length::XL)); }

        if (mod->getSavedValue<bool>("version-22-enabled")) { setTogglerState("version-22", true); m_versions.push_back(22); }
        if (mod->getSavedValue<bool>("version-21-enabled")) { setTogglerState("version-21", true); m_versions.push_back(21); }
        if (mod->getSavedValue<bool>("version-20-enabled")) { setTogglerState("version-20", true); m_versions.push_back(20); }
        if (mod->getSavedValue<bool>("version-19-enabled")) { setTogglerState("version-19", true); m_versions.push_back(19); }
        if (mod->getSavedValue<bool>("version-lower-than-19-enabled")) { setTogglerState("version-lower-than-19", true); m_versions.push_back(18); }

        if (mod->getSavedValue<bool>("auto-enabled")) { setTogglerState("auto-difficulty-toggler", true); m_difficulties.push_back(1); }
        if (mod->getSavedValue<bool>("easy-enabled")) { setTogglerState("easy-difficulty-toggler", true); m_difficulties.push_back(2); }
        if (mod->getSavedValue<bool>("normal-enabled")) { setTogglerState("normal-difficulty-toggler", true); m_difficulties.push_back(3); }

        if (mod->getSavedValue<bool>("hard-enabled")) setTogglerState("hard-difficulty-toggler", true);
        if (mod->getSavedValue<bool>("harder-enabled")) setTogglerState("harder-difficulty-toggler", true);
        if (mod->getSavedValue<bool>("insane-enabled")) setTogglerState("insane-difficulty-toggler", true);

        if (mod->getSavedValue<bool>("easy-demon-enabled")) { setTogglerState("easy-demon-toggler", true); m_demonDifficulties.push_back(3); }
        if (mod->getSavedValue<bool>("medium-demon-enabled")) { setTogglerState("medium-demon-toggler", true); m_demonDifficulties.push_back(4); }
        if (mod->getSavedValue<bool>("hard-demon-enabled")) { setTogglerState("hard-demon-toggler", true); m_demonDifficulties.push_back(0); }
        if (mod->getSavedValue<bool>("insane-demon-enabled")) { setTogglerState("insane-demon-toggler", true); m_demonDifficulties.push_back(5); }
        if (mod->getSavedValue<bool>("extreme-demon-enabled")) { setTogglerState("extreme-demon-toggler", true); m_demonDifficulties.push_back(6); }
    }

    refreshSplitDifficultyFilters();
    updateDifficultySelectorVisibility();

    return true;
}

void MainLayer::update(float dt) {
    if (m_randomPending) {
		    if (m_randomTimer > 0.f && m_randomLevelID > 0) {
			    m_randomTimer -= dt;
			    auto glm = GameLevelManager::sharedState();
			    if (glm) {
				    auto stored = glm->getStoredOnlineLevels(m_randomKey.c_str());
				    if (stored && stored->count() > 0) {
					    for (size_t i = 0; i < stored->count(); ++i) {
						    auto lvl = static_cast<GJGameLevel*>(stored->objectAtIndex(i));
						    if (lvl && lvl->m_levelID == m_randomLevelID) {
						    auto scene = LevelInfoLayer::scene(lvl, false);
						    auto trans = CCTransitionFade::create(0.5f, scene);
						    CCDirector::sharedDirector()->pushScene(trans);
						    m_randomPending = false;
						    m_randomKey.clear();
						    m_randomLevelID = -1;
						    m_randomTimer = 0.f;
						    if (m_randomLoadingLayer) {
                                m_randomLoadingLayer->hide();
                                m_randomLoadingLayer = nullptr;
                            }
						    return;
					    }
				    }
			    }
		    }

		    if (m_randomTimer <= 0.f) {
			    m_randomPending = false;
			    m_randomKey.clear();
			    m_randomLevelID = -1;
			    if (m_randomLoadingLayer) {
				    m_randomLoadingLayer->hide();
                    m_randomLoadingLayer = nullptr;
			    }
			    Notification::create("Failed to fetch random level", NotificationIcon::Error)->show();
		    }
	    }
    }
}

void MainLayer::onEasyDemonToggler(CCObject* sender) {
    processValueOnToggler(sender, m_demonDifficulties, 3);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("easy-demon-enabled", getNewTogglerState(sender));
}

void MainLayer::onMediumDemonToggler(CCObject* sender) {
    processValueOnToggler(sender, m_demonDifficulties, 4);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("medium-demon-enabled", getNewTogglerState(sender));
}


void MainLayer::onHardDemonToggler(CCObject* sender) {
    processValueOnToggler(sender, m_demonDifficulties, 0);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("hard-demon-enabled", getNewTogglerState(sender));
}


void MainLayer::onInsaneDemonToggler(CCObject* sender) {
    processValueOnToggler(sender, m_demonDifficulties, 5);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("insane-demon-enabled", getNewTogglerState(sender));
}


void MainLayer::onExtremeDemonToggler(CCObject* sender) {
    processValueOnToggler(sender, m_demonDifficulties, 6);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("extreme-demon-enabled", getNewTogglerState(sender));
}


void MainLayer::onStarToggler(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        if (std::find(m_grindTypes.begin(), m_grindTypes.end(), "star") == m_grindTypes.end()) {
            m_grindTypes.push_back("star");
        }
    } else {
        auto it = std::find(m_grindTypes.begin(), m_grindTypes.end(), "star");
        if (it != m_grindTypes.end()) {
            m_grindTypes.erase(it);
        }
    }
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("star-enabled", isToggled);
}

void MainLayer::onMoonToggler(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        if (std::find(m_grindTypes.begin(), m_grindTypes.end(), "moon") == m_grindTypes.end()) {
            m_grindTypes.push_back("moon");
        }
    } else {
        auto it = std::find(m_grindTypes.begin(), m_grindTypes.end(), "moon");
        if (it != m_grindTypes.end()) {
            m_grindTypes.erase(it);
        }
    }
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("moon-enabled", isToggled);
}

void MainLayer::onCoinToggler(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        if (std::find(m_grindTypes.begin(), m_grindTypes.end(), "coin") == m_grindTypes.end()) {
            m_grindTypes.push_back("coin");
        }
    } else {
        auto it = std::find(m_grindTypes.begin(), m_grindTypes.end(), "coin");
        if (it != m_grindTypes.end()) {
            m_grindTypes.erase(it);
        }
    }
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("coin-enabled", isToggled);
}

void MainLayer::onDemonToggler(CCObject* sender) {
    bool isToggled = getNewTogglerState(sender);

    if (isToggled) {
        if (std::find(m_grindTypes.begin(), m_grindTypes.end(), "demon") == m_grindTypes.end()) {
            m_grindTypes.push_back("demon");
        }
        m_demonsFilters.first->setVisible(true);
        m_demonsFilters.second->setVisible(true);
    } else {
        auto it = std::find(m_grindTypes.begin(), m_grindTypes.end(), "demon");
        if (it != m_grindTypes.end()) {
            m_grindTypes.erase(it);
        }

        m_demonsFilters.first->setVisible(false);
        m_demonsFilters.second->setVisible(false);
    }
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("demon-enabled", isToggled);
}

void MainLayer::onShortToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Short));
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("short-enabled", getNewTogglerState(sender));
}

void MainLayer::onMediumToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Medium));
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("medium-enabled", getNewTogglerState(sender));
}

void MainLayer::onLongToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::Long));
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("long-enabled", getNewTogglerState(sender));
}

void MainLayer::onXLToggler(CCObject* sender) {
    processValueOnToggler(sender, m_lengths, int(Length::XL));
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("xl-enabled", getNewTogglerState(sender));
}

void MainLayer::onAutoToggler(CCObject* sender) {
    processValueOnToggler(sender, m_difficulties, 1);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("auto-enabled", getNewTogglerState(sender));
}

void MainLayer::onEasyToggler(CCObject* sender) {
    processValueOnToggler(sender, m_difficulties, 2);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("easy-enabled", getNewTogglerState(sender));
}

void MainLayer::onNormalToggler(CCObject* sender) {
    processValueOnToggler(sender, m_difficulties, 3);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("normal-enabled", getNewTogglerState(sender));
}

void MainLayer::onHardToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 4, 5, getNewTogglerState(sender), m_splitHard4, m_splitHard5);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("hard-enabled", getNewTogglerState(sender));
}

void MainLayer::onHarderToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 6, 7, getNewTogglerState(sender), m_splitHarder6, m_splitHarder7);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("harder-enabled", getNewTogglerState(sender));
}

void MainLayer::onInsaneToggler(CCObject* sender) {
    applyDifficultyPair(m_difficulties, 8, 9, getNewTogglerState(sender), m_splitInsane8, m_splitInsane9);
    updateDifficultySelectorVisibility();
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("insane-enabled", getNewTogglerState(sender));
}

void MainLayer::onOpenDifficultySelector(CCObject* sender) {
    DifficultySelectorPopup::create(this)->show();
}

void MainLayer::onVer22Toggler(CCObject* sender) {
    processValueOnToggler(sender, m_versions, 22);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("version-22-enabled", getNewTogglerState(sender));
}

void MainLayer::onVer21Toggler(CCObject* sender) {
    processValueOnToggler(sender, m_versions, 21);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("version-21-enabled", getNewTogglerState(sender));
}

void MainLayer::onVer20Toggler(CCObject* sender) {
    processValueOnToggler(sender, m_versions, 20);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("version-20-enabled", getNewTogglerState(sender));
}

void MainLayer::onVer19Toggler(CCObject* sender) {
    processValueOnToggler(sender, m_versions, 19);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("version-19-enabled", getNewTogglerState(sender));
}

void MainLayer::onVerLower19Toggler(CCObject* sender) {
    processValueOnToggler(sender, m_versions, 18);
    if (auto mod = Mod::get()) mod->setSavedValue<bool>("version-lower-than-19-enabled", getNewTogglerState(sender));
}

bool MainLayer::isSplitDifficultySelected(int difficulty) const {
    switch (difficulty) {
        case 4: return m_splitHard4;
        case 5: return m_splitHard5;
        case 6: return m_splitHarder6;
        case 7: return m_splitHarder7;
        case 8: return m_splitInsane8;
        case 9: return m_splitInsane9;
        default: return false;
    }
}

void MainLayer::setSplitDifficultySelected(int difficulty, bool selected) {
    switch (difficulty) {
        case 4: m_splitHard4 = selected; break;
        case 5: m_splitHard5 = selected; break;
        case 6: m_splitHarder6 = selected; break;
        case 7: m_splitHarder7 = selected; break;
        case 8: m_splitInsane8 = selected; break;
        case 9: m_splitInsane9 = selected; break;
        default: return;
    }

    auto normalizePair = [](bool& first, bool& second) {
        if (!first && !second) {
            first = true;
            second = true;
        }
    };

    if (difficulty == 4 || difficulty == 5) normalizePair(m_splitHard4, m_splitHard5);
    if (difficulty == 6 || difficulty == 7) normalizePair(m_splitHarder6, m_splitHarder7);
    if (difficulty == 8 || difficulty == 9) normalizePair(m_splitInsane8, m_splitInsane9);
    if (auto mod = Mod::get()) {
        if (difficulty == 4 || difficulty == 5) {
            mod->setSavedValue<bool>("split-4-enabled", m_splitHard4);
            mod->setSavedValue<bool>("split-5-enabled", m_splitHard5);
        } else if (difficulty == 6 || difficulty == 7) {
            mod->setSavedValue<bool>("split-6-enabled", m_splitHarder6);
            mod->setSavedValue<bool>("split-7-enabled", m_splitHarder7);
        } else if (difficulty == 8 || difficulty == 9) {
            mod->setSavedValue<bool>("split-8-enabled", m_splitInsane8);
            mod->setSavedValue<bool>("split-9-enabled", m_splitInsane9);
        } else {
            mod->setSavedValue<bool>(fmt::format("split-{}-enabled", difficulty), selected);
        }
    }
}

void MainLayer::refreshSplitDifficultyFilters() {
    auto hardToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("hard-difficulty-toggler"));
    auto harderToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("harder-difficulty-toggler"));
    auto insaneToggler = typeinfo_cast<CCMenuItemToggler*>(this->getChildByIDRecursive("insane-difficulty-toggler"));

    applyDifficultyPair(m_difficulties, 4, 5, hardToggler && hardToggler->isToggled(), m_splitHard4, m_splitHard5);
    applyDifficultyPair(m_difficulties, 6, 7, harderToggler && harderToggler->isToggled(), m_splitHarder6, m_splitHarder7);
    applyDifficultyPair(m_difficulties, 8, 9, insaneToggler && insaneToggler->isToggled(), m_splitInsane8, m_splitInsane9);

    updateDifficultySelectorVisibility();
}

void MainLayer::updateDifficultySelectorVisibility() {
    auto selectorMenu = this->getChildByIDRecursive("difficulty-selector-menu");
    if (!selectorMenu) return;

    bool const hasSplitDifficulty =
        hasDifficulty(m_difficulties, 4) || hasDifficulty(m_difficulties, 5) ||
        hasDifficulty(m_difficulties, 6) || hasDifficulty(m_difficulties, 7) ||
        hasDifficulty(m_difficulties, 8) || hasDifficulty(m_difficulties, 9);

    selectorMenu->setVisible(hasSplitDifficulty);
}

}