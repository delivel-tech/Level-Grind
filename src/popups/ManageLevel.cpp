#include "ManageLevel.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <matjson.hpp>

#include "../other/LGManager.hpp"
#include "AddNotePopup.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

ManageLevel* ManageLevel::create(GJGameLevel* level, GJDifficultySprite* diffSprite) {
    auto ret = new ManageLevel;
    if (ret && ret->init(level, diffSprite)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ManageLevel::onStarSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return;

    bool isToggled = !toggler->isToggled();
    if (isToggled) {
        m_star = true;
    } else {
        m_star = false;
    }
}

void ManageLevel::onMoonSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return;

    bool isToggled = !toggler->isToggled();
    if (isToggled) {
        m_moon = true;
    } else {
        m_moon = false;
    }
}

void ManageLevel::onDemonSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return;

    bool isToggled = !toggler->isToggled();
    if (isToggled) {
        m_demon = true;
    } else {
        m_demon = false;
    }
}

bool ManageLevel::init(GJGameLevel* level, GJDifficultySprite* diffSprite) {
    if (!Popup::init(460.f, 280.f, "GJ_square02.png")) return false;

    constexpr float kStatusRowMenuScale = 0.85f;

    if (!level) {
        return false;
    }

    m_levelID = level->m_levelID;
    m_levelName = level->m_levelName;
    m_levelDifficulty = level->m_stars.value();
    m_levelLength = level->m_levelLength;
    if (m_levelDifficulty == 10) {
        m_demonDifficulty = level->m_demonDifficulty;
    }
    
    m_star = false;
    m_moon = false;
    m_demon = false;
    m_coin = false;

    if (level->isPlatformer()) {
        m_moon = true;
    } else {
        m_star = true;
    }

    if (m_levelDifficulty == 10) {
        m_demon = true;
    }

    this->setTitle("Manage Level");

    for (size_t i = 0; i < 5; i++) {
        auto container = NineSlice::create("square02_001.png");
        if (!container) continue;
        container->setOpacity(100);
        container->setID(m_containerIDs[i]);
        m_containers.push_back(container);
    }

    if (m_containers.size() < 5) {
        return false;
    }

    m_levelInfoContainer = m_containers[0];
    m_levelInfoLGContainer = m_containers[1];
    m_helperActionsContainer = m_containers[2];
    m_adminActionsContainer = m_containers[3];
    m_notesContainer = m_containers[4];

    CCSize contentSizeUp = { 215.f, 100.f };
    CCSize contentSizeDown = { 140.f, 125.f };

    m_levelInfoContainer->setContentSize(contentSizeUp);
    m_levelInfoLGContainer->setContentSize(contentSizeUp);
    m_helperActionsContainer->setContentSize(contentSizeDown);
    m_adminActionsContainer->setContentSize(contentSizeDown);
    m_notesContainer->setContentSize(contentSizeDown);

    auto containersMenu = CCMenu::create();
    m_mainLayer->addChildAtPosition(containersMenu, Anchor::Center);
    
    for (auto container : m_containers) {
        containersMenu->addChild(container);
    }

    m_levelInfoContainer->setPosition({ -112.5f, 55.5f });
    m_levelInfoLGContainer->setPosition({ 112.5f, 55.5f });
    m_helperActionsContainer->setPosition({ -150.f, -67.5f });
    m_adminActionsContainer->setPosition({ 0.f, -67.5f });
    m_notesContainer->setPosition({ 150.f, -67.5f });

    for (auto container : m_containers) {
        auto menu = CCMenu::create();
        menu->setID(m_menuIDs[m_i]);
        container->addChildAtPosition(menu, Anchor::Center);
        m_menus.push_back(menu);
        m_i++;
    }
    m_levelInfoMenu = m_menus[0];
    m_levelInfoLGMenu = m_menus[1];
    m_helperActionsMenu = m_menus[2];
    m_adminActionsMenu = m_menus[3];
    m_notesMenu = m_menus[4];

    auto filtersContainer = NineSlice::create("GJ_square02.png");
    filtersContainer->setContentSize({ 45.f, 165.f });

    m_mainLayer->addChildAtPosition(filtersContainer, Anchor::Right, { 28.f, 0.f });

    auto filtersMenu = CCMenu::create();
    filtersMenu->setID("filters-menu");
    filtersMenu->setLayout(ColumnLayout::create()->setGap(10.f)->setAutoGrowAxis(true));

    filtersMenu->setScale(0.7f);

    m_mainLayer->addChildAtPosition(filtersMenu, Anchor::Right, { 28.f, 0.f });

    auto starFilterSpr = CCSprite::create("GJ_button_04.png");
    auto moonFilterSpr = CCSprite::create("GJ_button_04.png");
    auto coinFilterSpr = CCSprite::create("GJ_button_04.png");
    auto demonFilterSpr = CCSprite::create("GJ_button_04.png");

    auto starFilterOnSpr = CCSprite::create("GJ_button_02.png");
    auto moonFilterOnSpr = CCSprite::create("GJ_button_02.png");
    auto coinFilterOnSpr = CCSprite::create("GJ_button_02.png");
    auto demonFilterOnSpr = CCSprite::create("GJ_button_02.png");

    auto starSprOff = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    auto moonSprOff = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    auto coinSprOff = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    auto demonSprOff = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");

    auto starOnSpr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    auto moonOnSpr = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    auto coinOnSpr = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    auto demonOnSpr = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");

    starSprOff->setPosition({ 20.f, 20.f });
    moonSprOff->setPosition({ 20.f, 20.f });
    coinSprOff->setPosition({ 20.f, 20.f });
    demonSprOff->setPosition({ 20.f, 20.f });
    starOnSpr->setPosition({ 20.f, 20.f });
    moonOnSpr->setPosition({ 20.f, 20.f });
    coinOnSpr->setPosition({ 20.f, 20.f });
    demonOnSpr->setPosition({ 20.f, 20.f });

    starFilterSpr->addChild(starSprOff);
    moonFilterSpr->addChild(moonSprOff);
    coinFilterSpr->addChild(coinSprOff);
    demonFilterSpr->addChild(demonSprOff);

    starFilterOnSpr->addChild(starOnSpr);
    moonFilterOnSpr->addChild(moonOnSpr);
    coinFilterOnSpr->addChild(coinOnSpr);
    demonFilterOnSpr->addChild(demonOnSpr);

    auto starFilterToggler = CCMenuItemToggler::create(
        starFilterSpr,
        starFilterOnSpr,
        this,
        menu_selector(ManageLevel::onStarSwitcher)
    );

    auto moonFilterToggler = CCMenuItemToggler::create(
        moonFilterSpr,
        moonFilterOnSpr,
        this,
        menu_selector(ManageLevel::onMoonSwitcher)
    );

    auto coinFilterToggler = CCMenuItemToggler::create(
        coinFilterSpr,
        coinFilterOnSpr,
        this,
        menu_selector(ManageLevel::onCoinSwitcher)
    );

    auto demonFilterToggler = CCMenuItemToggler::create(
        demonFilterSpr,
        demonFilterOnSpr,
        this,
        menu_selector(ManageLevel::onDemonSwitcher)
    );

    starFilterToggler->setID("star-filter-toggler");
    moonFilterToggler->setID("moon-filter-toggler");
    coinFilterToggler->setID("coin-filter-toggler");
    demonFilterToggler->setID("demon-filter-toggler");

    filtersMenu->addChild(starFilterToggler);
    filtersMenu->addChild(moonFilterToggler);
    filtersMenu->addChild(coinFilterToggler);
    filtersMenu->addChild(demonFilterToggler);

    if (!LGManager::get()->isAdmin()) {
        starFilterSpr->setColor({ 128, 128, 128 });
        moonFilterSpr->setColor({ 128, 128, 128 });
        demonFilterSpr->setColor({ 128, 128, 128 });

        starSprOff->setColor({ 128, 128, 128 });
        moonSprOff->setColor({ 128, 128, 128 });
        demonSprOff->setColor({ 128, 128, 128 });

        starFilterOnSpr->setColor({ 128, 128, 128 });
        moonFilterOnSpr->setColor({ 128, 128, 128 });
        demonFilterOnSpr->setColor({ 128, 128, 128 });

        starOnSpr->setColor({ 128, 128, 128 });
        moonOnSpr->setColor({ 128, 128, 128 });
        demonOnSpr->setColor({ 128, 128, 128 });

        starFilterToggler->setEnabled(false);
        moonFilterToggler->setEnabled(false);
        demonFilterToggler->setEnabled(false);
    }

    if (m_star) {
        starFilterToggler->toggle(true);
    } else {
        starFilterToggler->toggle(false);
    }

    if (m_moon) {
        moonFilterToggler->toggle(true);
    } else {
        moonFilterToggler->toggle(false);
    }

    if (m_demon) {
        demonFilterToggler->toggle(true);
    } else {
        demonFilterToggler->toggle(false);
    }

    filtersMenu->updateLayout();

    if (m_levelInfoMenu) {
        auto levelVisualMenu = CCMenu::create();
        levelVisualMenu->setID("level_visual_menu");
        levelVisualMenu->setLayout(
            ColumnLayout::create()
                ->setGap(3.f)
                ->setAxisAlignment(AxisAlignment::Center)
                ->setAutoScale(false)
                ->setAxisReverse(true)
        );
        levelVisualMenu->setPosition({ -72.f, -2.f });
        m_levelInfoMenu->addChild(levelVisualMenu);

        if (diffSprite) {
            auto popupDiffSprite = CCSprite::createWithSpriteFrame(diffSprite->displayFrame());
            if (popupDiffSprite) {
                popupDiffSprite->setScale(diffSprite->getScale());
                popupDiffSprite->setID("diff_sprite");
                levelVisualMenu->addChild(popupDiffSprite);
            }
        }

        auto ratingRowMenu = CCMenu::create();
        ratingRowMenu->setID("rating_row_menu");
        ratingRowMenu->setLayout(
            RowLayout::create()
                ->setGap(2.f)
                ->setAxisAlignment(AxisAlignment::Center)
                ->setAutoScale(false)
        );
        levelVisualMenu->addChild(ratingRowMenu);

        auto ratingIcon = CCSprite::createWithSpriteFrameName(
            m_moon ? "GJ_moonsIcon_001.png" : "GJ_starsIcon_001.png"
        );
        auto ratingLabel = CCLabelBMFont::create(
            numToString(m_levelDifficulty).c_str(),
            "bigFont.fnt"
        );

        if (ratingIcon) {
            ratingIcon->setScale(0.4f);
            ratingIcon->setID("rating_icon");
        }

        if (ratingLabel) {
            ratingLabel->setScale(0.3f);
            ratingLabel->setID("rating_label");
            limitNodeSize(ratingLabel, {34.f, 14.f}, 1.f, 0.1f);
            ratingRowMenu->addChild(ratingLabel);
        }

        if (ratingIcon) {
            ratingRowMenu->addChild(ratingIcon);
        }

        ratingRowMenu->updateLayout();
        levelVisualMenu->updateLayout();

        std::string creatorName = level->m_creatorName;
        if (creatorName.empty()) {
            creatorName = "Unknown";
        }

        std::string lengthText = "Unknown";
        switch (m_levelLength) {
            case 0: lengthText = "Tiny"; break;
            case 1: lengthText = "Short"; break;
            case 2: lengthText = "Medium"; break;
            case 3: lengthText = "Long"; break;
            case 4: lengthText = "XL"; break;
            case 5: lengthText = "Plat"; break;
            default: break;
        }

        auto infoColumnMenu = CCMenu::create();
        infoColumnMenu->setID("level_info_column_menu");
        infoColumnMenu->setLayout(
            ColumnLayout::create()
                ->setGap(4.f)
                ->setAxisAlignment(AxisAlignment::Center)
        );
        infoColumnMenu->setPosition({ 25.f, 2.f });
        m_levelInfoMenu->addChild(infoColumnMenu);

        auto creatorLabel = CCLabelBMFont::create(
            fmt::format("Creator: {}", creatorName).c_str(),
            "goldFont.fnt"
        );
        if (creatorLabel) {
            creatorLabel->setScale(0.32f);
            creatorLabel->setID("creator_label");
            infoColumnMenu->addChild(creatorLabel);
        }

        auto lengthLabel = CCLabelBMFont::create(
            fmt::format("Length: {}", lengthText).c_str(),
            "goldFont.fnt"
        );
        if (lengthLabel) {
            lengthLabel->setScale(0.32f);
            lengthLabel->setID("length_label");
            infoColumnMenu->addChild(lengthLabel);
        }

        infoColumnMenu->setScale(0.55f);

        infoColumnMenu->updateLayout();

        limitNodeSize(lengthLabel, {250.f, 26.f}, 1.f, 0.1f);
        limitNodeSize(creatorLabel, {250.f, 26.f}, 1.f, 0.1f);

        m_statusRowMenu = CCMenu::create();
        m_statusRowMenu->setID("status_row_menu");
        m_statusRowMenu->setLayout(RowLayout::create()->setGap(4.f)->setAxisAlignment(AxisAlignment::Center)->setAutoGrowAxis(true));

        auto statusLabel = CCLabelBMFont::create(
            "Status: ", "goldFont.fnt"
        );
        statusLabel->setID("status_label");
        limitNodeSize(statusLabel, {170.f, 26.f}, 1.f, 0.1f);
        m_statusRowMenu->addChild(statusLabel);

        m_levelInfoLGMenu->addChildAtPosition(m_statusRowMenu, Anchor::Center, { 0.f, 20.f });
        m_statusRowMenu->setScale(kStatusRowMenuScale);

        auto loadingStatus = LoadingSpinner::create(20.f);
        loadingStatus->setID("loading_status");
        m_statusRowMenu->addChild(loadingStatus);

        m_statusRowMenu->updateLayout();
    }

    auto filtersRowMenu = CCMenu::create();
    filtersRowMenu->setID("filters_row_menu");
    filtersRowMenu->setLayout(RowLayout::create()->setGap(10.f));
    filtersRowMenu->setScale(0.7f);

    m_levelInfoLGMenu->addChildAtPosition(filtersRowMenu, Anchor::Center, {0.f, -10.f});

    auto starSpr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    auto moonSpr = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    auto coinSpr = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    auto demonSpr = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");

    m_starSpr = starSpr;
    m_moonSpr = moonSpr;
    m_coinSpr = coinSpr;
    m_demonSpr = demonSpr;

    auto starLoading = LoadingSpinner::create(20.f);
    auto moonLoading = LoadingSpinner::create(20.f);
    auto coinLoading = LoadingSpinner::create(20.f);
    auto demonLoading = LoadingSpinner::create(20.f);

    m_starLoading = starLoading;
    m_moonLoading = moonLoading;
    m_coinLoading = coinLoading;
    m_demonLoading = demonLoading;

    if (filtersRowMenu && starSpr && moonSpr && coinSpr && demonSpr && starLoading && moonLoading && coinLoading && demonLoading) {
        starSpr->setScale(0.6f);
        moonSpr->setScale(0.6f);
        coinSpr->setScale(0.6f);
        demonSpr->setScale(0.6f);

        filtersRowMenu->addChild(starSpr);
        filtersRowMenu->addChild(starLoading);
        filtersRowMenu->addChild(moonSpr);
        filtersRowMenu->addChild(moonLoading);
        filtersRowMenu->addChild(coinSpr);
        filtersRowMenu->addChild(coinLoading);
        filtersRowMenu->addChild(demonSpr);
        filtersRowMenu->addChild(demonLoading);

        filtersRowMenu->updateLayout();
    }

    auto helperToolsMenu = CCMenu::create();
    helperToolsMenu->setID("helper_tools_menu");
    helperToolsMenu->setLayout(ColumnLayout::create()->setGap(10.f));
    helperToolsMenu->setScale(0.8f);

    auto helperToolsLabel = CCLabelBMFont::create("Helper Tools", "goldFont.fnt");
    helperToolsLabel->setID("helper_tools_label");
    helperToolsLabel->setScale(0.7f);

    m_helperActionsMenu->addChildAtPosition(helperToolsMenu, Anchor::Center);
    m_helperActionsMenu->addChildAtPosition(helperToolsLabel, Anchor::Center, {0.f, 50.f});

    m_addBtnSpr = ButtonSprite::create("Add");
    if (m_addBtnSpr) {
        m_addBtnSpr->setColor({ 128, 128, 128 });
    }

    auto addBtn = CCMenuItemSpriteExtra::create(
        m_addBtnSpr,
        this,
        menu_selector(ManageLevel::onAddButton)
    );
    m_addBtn = addBtn;

    m_addBtn->setID("add-btn");
    m_addBtn->setEnabled(false);

    helperToolsMenu->addChild(m_addBtn);
    helperToolsMenu->updateLayout();

    auto adminToolsMenu = CCMenu::create();
    adminToolsMenu->setID("admin_tools_menu");
    adminToolsMenu->setLayout(ColumnLayout::create()->setGap(10.f));

    auto adminToolsLabel = CCLabelBMFont::create("Admin Tools", "goldFont.fnt");
    adminToolsLabel->setID("admin_tools_label");
    adminToolsLabel->setScale(0.7f);

    m_adminActionsMenu->addChildAtPosition(adminToolsMenu, Anchor::Center);
    m_adminActionsMenu->addChildAtPosition(adminToolsLabel, Anchor::Center, {0.f, 50.f});

    m_lockBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Lock"),
        this,
        menu_selector(ManageLevel::onLockBtn)
    );
    m_lockBtn->setID("lock-btn");

    m_lockBtn->setEnabled(false);

    if (LGManager::get()->isAdmin()) {
        adminToolsMenu->addChild(m_lockBtn);
    } else {
        auto lockedBtnSpr = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
        lockedBtnSpr->setScale(1.25f);
        auto lockedBtn = CCMenuItemSpriteExtra::create(
            lockedBtnSpr,
            this,
            menu_selector(ManageLevel::onLockedBtn)
        );
        lockedBtn->setID("locked-btn");
        adminToolsMenu->addChild(lockedBtn);
    }

    adminToolsMenu->setScale(0.8f);

    adminToolsMenu->updateLayout();

    auto notesLabel = CCLabelBMFont::create("Notes", "goldFont.fnt");
    notesLabel->setID("notes_label");
    notesLabel->setScale(0.7f);

    m_notesMenu->addChildAtPosition(notesLabel, Anchor::Center, {0.f, 50.f});

    auto notesMenu = CCMenu::create();
    notesMenu->setID("notes_menu");
    notesMenu->setLayout(RowLayout::create()->setGap(10.f));

    m_notesMenu->addChildAtPosition(notesMenu, Anchor::Center);

    auto addNoteBtn = CCMenuItemSpriteExtra::create(
        CircleButtonSprite::createWithSprite("button_edit_note.png"_spr, 1.f, CircleBaseColor::Green),
        this,
        menu_selector(ManageLevel::onAddNoteButton)
    );
    addNoteBtn->setEnabled(false);
    notesMenu->addChild(addNoteBtn);

    m_addNoteBtn = addNoteBtn;

    notesMenu->updateLayout();

    addSideArt(m_mainLayer, SideArt::All, SideArtStyle::PopupBlue, false);

    web::WebRequest req;
    matjson::Value body;

    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["levelId"] = m_levelID;
    body["token"] = LGManager::get()->getArgonToken();

    req.bodyJSON(body);

    auto self = Ref(this);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/get_level_info_staff"),
        [self](web::WebResponse res) {
            const CCSize statusRowLimit = { 190.f, 26.f };

            if (!self) {
                log::error("failed to get level info: no self");
                return;
            }
            if (!self->getParent() || !self->m_mainLayer) {
                return;
            }
            if (!res.ok()) {
                log::error("failed to get level info: bad response");
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                log::error("failed to get level info: invalid json");
                return;
            }

            auto json = jsonRes.unwrap();

            bool isOK = json["ok"].asBool().unwrapOrDefault();
            if (!isOK) {
                log::error("failed to get level info: not ok");
                return;
            }

            bool isLocked = json["isLocked"].asBool().unwrapOrDefault();
            bool isAdded = json["isAdded"].asBool().unwrapOrDefault();
            bool isStar = json["star"].asBool().unwrapOrDefault();
            bool isMoon = json["moon"].asBool().unwrapOrDefault();
            bool isCoin = json["coin"].asBool().unwrapOrDefault();
            bool isDemon = json["demon"].asBool().unwrapOrDefault();
            std::string addedBy = json["added_by"].asString().unwrapOrDefault();
            bool noteExists = json["noteExists"].asBool().unwrapOrDefault();
            if (noteExists) {
                auto noteAddParent = self->m_addNoteBtn->getParent();
                self->m_addNoteBtn->removeFromParent();

                auto readdNoteBtn = CCMenuItemSpriteExtra::create(
                CircleButtonSprite::createWithSprite("button_edit_note.png"_spr, 1.f, CircleBaseColor::Blue),
                self,
                menu_selector(ManageLevel::onAddNoteButton)
                );

                auto deleteNoteBtn = CCMenuItemSpriteExtra::create(
                    CircleButtonSprite::createWithSprite("button_edit_note.png"_spr, 1.f, CircleBaseColor::Red),
                self,
                menu_selector(ManageLevel::onDeleteNoteButton)
                );

                noteAddParent->addChild(deleteNoteBtn);
                noteAddParent->addChild(readdNoteBtn);

                noteAddParent->updateLayout();
            } else {
                self->m_addNoteBtn->setEnabled(true);
            }

            auto filtersMenu = self->m_starLoading ? self->m_starLoading->getParent() : nullptr;

            auto starFilterToggler = typeinfo_cast<CCMenuItemToggler*>(self->getChildByIDRecursive("star-filter-toggler"));
            auto moonFilterToggler = typeinfo_cast<CCMenuItemToggler*>(self->getChildByIDRecursive("moon-filter-toggler"));
            auto coinFilterToggler = typeinfo_cast<CCMenuItemToggler*>(self->getChildByIDRecursive("coin-filter-toggler"));
            auto demonFilterToggler = typeinfo_cast<CCMenuItemToggler*>(self->getChildByIDRecursive("demon-filter-toggler"));

            if (self->m_starLoading && self->m_starSpr) {
                auto parent = self->m_starLoading->getParent();
                if (parent) {
                    self->m_starLoading->removeFromParent();
                    auto iconName = isStar ? "GJ_completesIcon_001.png" : "GJ_deleteIcon_001.png";
                    if (auto resultSpr = CCSprite::createWithSpriteFrameName(iconName)) {
                        parent->insertAfter(resultSpr, self->m_starSpr);
                    }
                }
            }

            if (self->m_moonLoading && self->m_moonSpr) {
                auto parent = self->m_moonLoading->getParent();
                if (parent) {
                    self->m_moonLoading->removeFromParent();
                    auto iconName = isMoon ? "GJ_completesIcon_001.png" : "GJ_deleteIcon_001.png";
                    if (auto resultSpr = CCSprite::createWithSpriteFrameName(iconName)) {
                        parent->insertAfter(resultSpr, self->m_moonSpr);
                    }
                }
            }

            if (self->m_coinLoading && self->m_coinSpr) {
                auto parent = self->m_coinLoading->getParent();
                if (parent) {
                    self->m_coinLoading->removeFromParent();
                    auto iconName = isCoin ? "GJ_completesIcon_001.png" : "GJ_deleteIcon_001.png";
                    if (auto resultSpr = CCSprite::createWithSpriteFrameName(iconName)) {
                        parent->insertAfter(resultSpr, self->m_coinSpr);
                    }
                }
            }

            if (self->m_demonLoading && self->m_demonSpr) {
                auto parent = self->m_demonLoading->getParent();
                if (parent) {
                    self->m_demonLoading->removeFromParent();
                    auto iconName = isDemon ? "GJ_completesIcon_001.png" : "GJ_deleteIcon_001.png";
                    if (auto resultSpr = CCSprite::createWithSpriteFrameName(iconName)) {
                        parent->insertAfter(resultSpr, self->m_demonSpr);
                    }
                }
            }

            if (isAdded) {

            if (isStar && starFilterToggler) {
                starFilterToggler->toggle(true);
                self->m_star = true;
            } else {
                self->m_star = false;
                starFilterToggler->toggle(false);
            }

            if (isMoon && moonFilterToggler) {
                moonFilterToggler->toggle(true);
                self->m_moon = true;
            } else {
                self->m_moon = false;
                moonFilterToggler->toggle(false);
            }

            if (isCoin && coinFilterToggler) {
                coinFilterToggler->toggle(true);
                self->m_coin = true;
            } else {
                self->m_coin = false;
                coinFilterToggler->toggle(false);
            }

            if (isDemon && demonFilterToggler) {
                demonFilterToggler->toggle(true);
                self->m_demon = true;
            } else {
                self->m_demon = false;
                demonFilterToggler->toggle(false);
            }
        }

            if (filtersMenu) {
                filtersMenu->updateLayout();
            }

            if (!self->m_statusRowMenu) {
                return;
            }

            auto refreshStatusRowMenu = [&]() {
                self->m_statusRowMenu->updateLayout();
                limitNodeSize(self->m_statusRowMenu, statusRowLimit, 1.f, 0.6f);
            };

            if (isAdded) {
                self->m_statusRowMenu->removeAllChildren();
                auto statusLabel = CCLabelBMFont::create(fmt::format("Status: Added ({})", addedBy).c_str(), "goldFont.fnt");
                limitNodeSize(statusLabel, {170.f, 26.f}, 1.f, 0.1f);
                self->m_statusRowMenu->addChild(statusLabel);
                refreshStatusRowMenu();
                
                auto parent = self->m_addBtn->getParent();
                self->m_addBtn->removeFromParent();
                self->m_reAddBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Re-Add", "goldFont.fnt", "GJ_button_01.png"),
                    self,
                    menu_selector(ManageLevel::onAddButton)
                );
                parent->addChild(self->m_reAddBtn);

                self->m_deleteBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Delete", "goldFont.fnt", "GJ_button_06.png"),
                    self,
                    menu_selector(ManageLevel::onDeleteBtn)
                );
                parent->addChild(self->m_deleteBtn);
                parent->updateLayout();
            } else {
                self->m_statusRowMenu->removeAllChildren();
                auto statusLabel = CCLabelBMFont::create("Status: Not Added", "goldFont.fnt");
                limitNodeSize(statusLabel, {170.f, 26.f}, 1.f, 0.1f);
                self->m_statusRowMenu->addChild(statusLabel);
                refreshStatusRowMenu();

                self->m_addBtn->setEnabled(true);
            }

            if (isLocked) {
                auto lockedSpr = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
                self->m_statusRowMenu->addChild(lockedSpr);
                refreshStatusRowMenu();
            } else {
                auto unlockedSpr = CCSprite::createWithSpriteFrameName("GJ_lock_open_001.png");
                self->m_statusRowMenu->addChild(unlockedSpr);
                refreshStatusRowMenu();
            }

            if (!LGManager::get()->isAdmin()) return;

            if (isLocked) {
                if (!self->m_lockBtn) {
                    return;
                }

                auto parent = self->m_lockBtn->getParent();
                if (!parent) {
                    return;
                }

                self->m_lockBtn->removeFromParent();
                self->m_lockBtn = nullptr;
                auto unlockBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Unlock", "goldFont.fnt", "GJ_button_01.png"),
                    self,
                    menu_selector(ManageLevel::onUnlockBtn)
                );
                self->m_unlockBtn = unlockBtn;
                parent->addChild(unlockBtn);
                parent->updateLayout();
            } else {
                self->m_unlockBtn = nullptr;
                if (self->m_lockBtn) {
                    self->m_lockBtn->setEnabled(true);
                }
            }
        }
    );

    return true;
}

void ManageLevel::onAddNoteButton(CCObject* sender) {
    AddNotePopup::create(m_levelID, m_levelName)->show();
}

void ManageLevel::onDeleteNoteButton(CCObject* sender) {
    web::WebRequest req;

    matjson::Value body;

    body["accountID"] = GJAccountManager::sharedState()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();
    body["levelID"] = m_levelID;
    body["levelName"] = m_levelName;
    body["deletedBy"] = GJAccountManager::get()->m_username;

    req.bodyJSON(body);

    auto uPopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Deleting note...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/delete_note"),
        [uPopupRef](web::WebResponse res) {
            if (!uPopupRef) return;
            if (!res.ok()) {
                uPopupRef->showFailMessage("Failed! Try again later.");
                log::error("req failed, code: {}", res.code());
                return;
            } else {
                uPopupRef->showSuccessMessage("Success! Note deleted.");
                return;
            }
        }
    );
}

void ManageLevel::onCoinSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggler) return;

    bool isToggled = !toggler->isToggled();
	if (isToggled) {
		m_coin = true;
	} else {
		m_coin = false;
	}
}

void ManageLevel::onDeleteBtn(CCObject* sender) {
    matjson::Value body;
    body["token"] = LGManager::get()->getArgonToken();
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["id"] = m_levelID;

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Deleting level...");
    upopup->show();

    web::WebRequest req;
    req.bodyJSON(body);

    auto popupRef = Ref(upopup);

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/delete_level"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                log::error("req failed");
                popupRef->showFailMessage("Deleting level failed. Try again later.");
                return;
            }
            popupRef->showSuccessMessage("Success! Level deleted.");
        }
    );
}

void ManageLevel::onUnlockBtn(CCObject* sender) {
    matjson::Value body;
    auto finalLevelName = m_levelName.empty() ? std::string("blank name") : m_levelName;

    body["token"] = LGManager::get()->getArgonToken();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["levelId"] = m_levelID;
    body["levelName"] = finalLevelName;
    body["unbannedBy"] = GJAccountManager::sharedState()->m_username.c_str();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Unlocking level...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<ManageLevel> layerRef = this;
    auto clickedUnlockBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    Ref<CCMenuItemSpriteExtra> unlockBtnRef = clickedUnlockBtn;

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/unban_level"),
        [popupRef, layerRef, unlockBtnRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!layerRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Unlocking level failed. Try again later.");
                log::error("failed unlocking level");
                return;
            }
            popupRef->showSuccessMessage("Success! Level unlocked.");
            auto unlockBtn = unlockBtnRef ? unlockBtnRef.data() : layerRef->m_unlockBtn;
            if (!unlockBtn) return;
            auto parent = unlockBtn->getParent();
            if (!parent) return;

            unlockBtn->removeFromParent();
            layerRef->m_unlockBtn = nullptr;

            auto lockBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Lock"),
                layerRef,
                menu_selector(ManageLevel::onLockBtn)
            );
            layerRef->m_lockBtn = lockBtn;
            parent->addChild(lockBtn);
            parent->updateLayout();
        }
    );
}

void ManageLevel::onNotReadyBtn(CCObject* sender) {
    FLAlertLayer::create(
        "Coming soon",
        "Notes feature has not been implemented yet, but it's coming soon. Stay tuned!",
        "OK"
    )->show();
}

void ManageLevel::onLockedBtn(CCObject* sender) {
    FLAlertLayer::create(
        "Not an Admin",
        "You cannot access these features because you are not an admin.",
        "OK"
    )->show();
}

void ManageLevel::onLockBtn(CCObject* sender) {
    matjson::Value body;
    auto finalLevelName = m_levelName.empty() ? std::string("blank name") : m_levelName;

    body["token"] = LGManager::get()->getArgonToken();
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["levelId"] = m_levelID;
    body["levelName"] = finalLevelName;
    body["bannedBy"] = GJAccountManager::sharedState()->m_username.c_str();

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Locking level...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;
    Ref<ManageLevel> layerRef = this;
    auto clickedLockBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    Ref<CCMenuItemSpriteExtra> lockBtnRef = clickedLockBtn;

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/ban_level"),
        [popupRef, layerRef, lockBtnRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!layerRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Locking level failed. Try again later.");
                log::error("failed locking level");
                return;
            }
            popupRef->showSuccessMessage("Success! Level locked.");
            auto lockBtn = lockBtnRef ? lockBtnRef.data() : layerRef->m_lockBtn;
            if (!lockBtn) return;
            auto parent = lockBtn->getParent();
            if (!parent) return;

            lockBtn->removeFromParent();
            layerRef->m_lockBtn = nullptr;

            auto unlockBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Unlock"),
                layerRef,
                menu_selector(ManageLevel::onUnlockBtn)
            );
            layerRef->m_unlockBtn = unlockBtn;
            parent->addChild(unlockBtn);
            parent->updateLayout();
        }
    );
}

void ManageLevel::onAddButton(CCObject* sender) {
    matjson::Value body;
    auto finalLevelName = m_levelName.empty() ? "blank name" : m_levelName;

    body["id"] = m_levelID;
    body["name"] = finalLevelName;
    body["difficulty"] = m_levelDifficulty;
    body["length"] = m_levelLength;

    body["token"] = LGManager::get()->getArgonToken();
    body["account_id"] = GJAccountManager::get()->m_accountID;

    if (m_levelDifficulty == 10) {
        body["demon_difficulty"] = m_demonDifficulty;
    }

    if (m_star) body["star"] = m_star;
    if (m_moon) body["moon"] = m_moon;
    if (m_coin) body["coin"] = m_coin;
    if (m_demon) body["demon"] = m_demon;

    body["added_by"] = GJAccountManager::sharedState()->m_username.c_str();

    web::WebRequest req;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(typeinfo_cast<::UploadPopupDelegate*>(this), "Adding level...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/new_level"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Adding level failed. Try again later.");
                log::error("failed adding level");
                return;
            }
            popupRef->showSuccessMessage("Success! Level added.");
        }
    );
}

void ManageLevel::onClosePopup(::UploadActionPopup* popup) {
    if (popup) {
        popup->m_delegate = nullptr;
        if (popup->getParent()) {
            popup->closePopup();
        }
    }
    this->onClose(nullptr);
}