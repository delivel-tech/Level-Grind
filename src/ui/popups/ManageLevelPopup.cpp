#include "ManageLevelPopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>
#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

#include "../../utils/utils.hpp"
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class EventChooser : public BasePopup {
public:
    static EventChooser* create(GetLevelInfoResponse res) {
        auto ret = new EventChooser;
        if (ret && ret->init(res)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(GetLevelInfoResponse res) {
        if (!BasePopup::init({ 210.f, 180.f })) return false;

        auto buttonsMenu = Build<CCMenu>::create()
            .layout(ColumnLayout::create()->setGap(10))
            .parent(m_buttonMenu)
            .collect();

        Build(ButtonSprite::create("Lock", "bigFont.fnt", "GJ_button_01.png"))
            .intoMenuItem([] {

            })
            .parent(buttonsMenu)
            .collect();

        return true;
    }
};

ManageLevelPopup* ManageLevelPopup::create(GJGameLevel *level) {
    auto ret = new ManageLevelPopup;
    if (ret && ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ManageLevelPopup::formBody() {
    m_body.id = m_level->m_levelID;
    m_body.name = m_level->m_levelName;
    m_body.difficulty = m_level->m_stars.value();
    m_body.length = m_level->m_levelLength;

    if (m_body.difficulty == 10) m_body.demonDifficulty = m_level->m_demonDifficulty;

    if (m_level->isPlatformer()) m_body.moon = true;
    else m_body.star = true;

    if (m_body.difficulty == 10) m_body.demon = true;

    return;
}

bool ManageLevelPopup::init(GJGameLevel* level) {
    if (!BasePopup::init({ 350.f, 250.f })) return false;

    if (DataManager::getInstance().getUserPosition() != GrindPosition::Admin 
    && DataManager::getInstance().getUserPosition() != GrindPosition::Owner 
    && DataManager::getInstance().getUserPosition() != GrindPosition::Helper) {
        Notification::create("User is not staff.", NotificationIcon::Error)->show();
        return true;
    }

    if (!level) return false;
    m_level = level;

    formBody();

    m_loadingSpinner = Build(LoadingSpinner::create(40.f))
        .parent(m_mainLayer)
        .center()
        .collect();

    auto self = Ref(this);

    m_listener.spawn(
        APIClient::getInstance().getLevelInfo(m_level->m_levelID),
        [self](web::WebResponse res) {
            if (!self) return;

            auto parsed = APIClient::getInstance().getLevelInfoParse(res);

            if (!parsed.ok) {
                Notification::create("Failed to get level info. Try again later.", NotificationIcon::Error)->show();
                self->m_loadingSpinner->removeFromParent();
                return;
            }

            self->m_loadingSpinner->removeFromParent();

            self->setTitle("Manage Level");

            if (parsed.coin) {
                self->m_body.coin = true;
            } else {
                self->m_body.coin = false;
            }

            // making status menu

            auto levelInfoLayout = RowLayout::create()
                ->setGap(5);

            self->m_levelInfoMenu = Build<CCMenu>::create()
                .layout(levelInfoLayout)
                .scale(0.5f)
                .parent(self->m_buttonMenu)
                .pos(self->centerX(), self->m_title->getPositionY() - 20)
                .id("level-info-menu")
                .collect();

            auto layout = ColumnLayout::create()
                ->setGap(10);

            self->m_helperButtonsMenu = Build<CCMenu>::create()
                .scale(0.8f)
                .layout(layout)
                .parent(self->m_buttonMenu)
                .id("helper-buttons-menu")
                .pos(self->centerX() - 85.f, (self->m_title->getPositionY() - 20) / 2)
                .collect();

            self->m_adminButtonsMenu = Build<CCMenu>::create()
                .scale(0.65f)
                .layout(layout)
                .parent(self->m_buttonMenu)
                .id("admin-buttons-menu")
                .pos(self->centerX() - 85.f, (self->m_title->getPositionY() - 20) / 2)
                .visible(false)
                .collect();

            self->m_levelInfoMenu->addChild(
                CCLabelBMFont::create("Status:", "goldFont.fnt")
            );

            if (parsed.isAdded) {
                self->m_levelInfoMenu->addChild(
                    CCLabelBMFont::create("Added", "goldFont.fnt")
                );
                
                self->m_levelInfoMenu->addChild(
                    CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png")
                );

                self->m_levelInfoMenu->addChild(
                    CCLabelBMFont::create(fmt::format("({})", parsed.addedBy).c_str(), "goldFont.fnt")
                );
            } else {
                self->m_levelInfoMenu->addChild(
                    CCLabelBMFont::create("Not Added", "goldFont.fnt")
                );

                self->m_levelInfoMenu->addChild(
                    CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png")
                );
            }

            if (parsed.isLocked) {
                self->m_levelInfoMenu->addChild(
                    Build<CCSprite>::createSpriteName("GJ_lock_001.png")
                        .intoMenuItem([] {
                            Notification::create("This level has been locked. You cannot change it.", NotificationIcon::Warning)->show();
                        })
                        .collect()
                );
            } else {
                self->m_levelInfoMenu->addChild(
                    Build<CCSprite>::createSpriteName("GJ_lock_open_001.png")
                        .intoMenuItem([] {
                            Notification::create("This level is not locked. You are able to change it.", NotificationIcon::Success)->show();
                        })
                        .collect()
                );
            }

            self->m_levelInfoMenu->updateLayout();

            auto menuBg = Build(NineSlice::create("square02_small.png"))
                .contentSize({
                    self->m_mainLayer->getContentWidth() / 2.f,
                    self->m_mainLayer->getContentHeight() / 1.5f
                })
                .pos(self->centerX() - 85.f, self->centerY() - 20)
                .id("menu-bg")
                .scale(0.9f)
                .opacity(80)
                .parent(self->m_mainLayer)
                .collect();

            // making helper menu

            Build(ButtonSprite::create("Accept", "bigFont.fnt", "GJ_button_01.png"))
                .intoMenuItem([self] {
                    auto uPopup = UploadActionPopup::create(nullptr, "Adding point...");
                    uPopup->show();

                    auto uPopupRef = Ref(uPopup);

                    self->m_listener.spawn(
                        APIClient::getInstance().changePoint(PointType::AcceptPoint, self->m_body.coin ? CoinPointType::AcceptCoinPoint : CoinPointType::RejectCoinPoint, self->m_body),
                        [uPopupRef](web::WebResponse res) {
                            if (!uPopupRef) return;
                            auto parsed = APIClient::getInstance().changePointParse(res);

                            if (!parsed.ok) {
                                uPopupRef->showFailMessage("Failed to add point.");
                                return;
                            } else {
                                uPopupRef->showSuccessMessage("Success! Added point.");
                                return;
                            }
                        }
                    );
                })
                .parent(self->m_helperButtonsMenu)
                .collect();
            
            Build(ButtonSprite::create("Reject", "bigFont.fnt", "GJ_button_06.png"))
                .intoMenuItem([self] {
                    auto uPopup = UploadActionPopup::create(nullptr, "Removing point...");
                    uPopup->show();

                    auto uPopupRef = Ref(uPopup);

                    self->m_listener.spawn(
                        APIClient::getInstance().changePoint(PointType::RejectPoint, self->m_body.coin ? CoinPointType::AcceptCoinPoint : CoinPointType::RejectCoinPoint, self->m_body),
                        [uPopupRef](web::WebResponse res) {
                            if (!uPopupRef) return;
                            auto parsed = APIClient::getInstance().changePointParse(res);

                            if (!parsed.ok) {
                                uPopupRef->showFailMessage("Failed to remove point.");
                                return;
                            } else {
                                uPopupRef->showSuccessMessage("Success! Removed point.");
                                return;
                            }
                        }
                    );
                })
                .parent(self->m_helperButtonsMenu)
                .collect();

            // admin toggler
            if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin
            || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
                Build<CCMenuItemToggler>::createToggle(
                    Build<CCSprite>::create("badge_admin.png"_spr).color({ 100, 100, 100 }).collect(),
                    Build<CCSprite>::create("badge_admin.png"_spr).collect(),
                    [self](CCMenuItemToggler* toggler) {
                        bool isToggled = getNewTogglerState(toggler);

                        if (isToggled) {
                            self->m_helperButtonsMenu->setVisible(false);
                            self->m_adminButtonsMenu->setVisible(true);
                        } else {
                            self->m_helperButtonsMenu->setVisible(true);
                            self->m_adminButtonsMenu->setVisible(false);
                        }
                    }
                )
                .parent(self->m_buttonMenu)
                .pos({ self->m_buttonMenu->getContentSize() })
                .collect();
            }

            // notes btn
            Build(CircleButtonSprite::createWithSprite("button_edit_note.png"_spr))
                .scale(0.6f)
                .intoMenuItem([] {

                })
                .parent(self->m_buttonMenu)
                .pos({ self->m_mainLayer->getContentWidth(), 0 });

            Build<CCMenuItemToggler>::createToggle(
                Build<CCSprite>::createSpriteName("GJ_coinsIcon2_001.png").color(100, 100, 100).collect(),
                Build<CCSprite>::createSpriteName("GJ_coinsIcon2_001.png").collect(),
                [self](CCMenuItemToggler* toggler) {
                    bool isToggled = getNewTogglerState(toggler);

                    if (isToggled) {
                        self->m_body.coin = true;
                    } else {
                        self->m_body.coin = false;
                    }
                }
            )
                .parent(self->m_buttonMenu)
                .pos({ 0, 0 })
                .with([self](CCMenuItemToggler* toggler) {
                    if (self->m_body.coin) toggler->toggle(true);
                })
                .collect();

            // admin menu
            if (parsed.isLocked) {
                Build(ButtonSprite::create("Unlock", "bigFont.fnt", "GJ_button_06.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            } else {
                Build(ButtonSprite::create("Lock", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            }

            Build(ButtonSprite::create("Event", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();

            if (parsed.isAdded) {
                Build(ButtonSprite::create("Delete", "bigFont.fnt", "GJ_button_06.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();

                Build(ButtonSprite::create("Re-add", "bigFont.fnt", "GJ_button_02.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            } else {
                Build(ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            }

            auto pointsListNode = Build(cue::ListNode::create({self->m_mainLayer->getContentWidth() / 2.f,
                    self->m_mainLayer->getContentHeight() / 1.5f}))
                .pos(self->centerX() + 85.f, self->centerY() - 20)
                .scale(0.9f)
                .id("points-list-node")
                .parent(self->m_mainLayer)
                .with([](cue::ListNode* list) {
                    list->setAutoUpdate(true);
                })
                .collect();

            auto createListCell = [self, pointsListNode](PointInfo pointInfo) {
                CCNode* cell = Build(CCNode::create())
                    .contentSize({
                        self->m_mainLayer->getContentWidth() / 2.f,
                        self->m_mainLayer->getContentHeight() / 8.f
                    })
                    .collect();

                CCMenu* cellMenu = Build(CCMenu::create())
                    .layout(RowLayout::create()->setGap(10))
                    .parent(cell)
                    .scale(0.8f)
                    .center()
                    .collect();

                auto staffUsernameBtn = Build(CCLabelBMFont::create(fmt::format("{}", pointInfo.staffUsername).c_str(), "goldFont.fnt"))
                    .limitLabelWidth(100, 1.f, 0.2f)
                    .intoMenuItem([pointInfo] {
                        ProfilePage::create(pointInfo.staffId, false)->show();
                    })
                    .parent(cellMenu)
                    .collect();

                if (pointInfo.point == 1) {
                    auto acceptedBtn = Build(CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))
                        .intoMenuItem([self] {
                            Notification::create(fmt::format("This user accepted {}.", self->m_body.name), NotificationIcon::Success)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                } else if (pointInfo.point == -1) {
                    auto rejectedBtn = Build(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"))
                        .intoMenuItem([self] {
                            Notification::create(fmt::format("This user rejected {}.", self->m_body.name), NotificationIcon::Error)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                } else {
                    auto warningBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
                        .intoMenuItem([] {
                            Notification::create("Something went wrong.", NotificationIcon::Warning)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                }

                if (pointInfo.coinPoint == 1) {
                    auto acceptedBtn = Build(CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png"))
                        .intoMenuItem([self] {
                            Notification::create(fmt::format("This user accepted coin for {}.", self->m_body.name), NotificationIcon::Success)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                } else if (pointInfo.coinPoint == -1) {
                    auto rejectedBtn = Build(CCSprite::createWithSpriteFrameName("GJ_coinsIcon_gray_001.png"))
                        .intoMenuItem([self] {
                            Notification::create(fmt::format("This user rejected coin for {}.", self->m_body.name), NotificationIcon::Error)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                } else {
                    auto warningBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
                        .intoMenuItem([] {
                            Notification::create("Something went wrong.", NotificationIcon::Warning)->show();
                        })
                        .parent(cellMenu)
                        .collect();
                }

                cellMenu->updateLayout();
                pointsListNode->addCell(cell);
                return;
            };

            auto createInfoCells = [parsed, self, pointsListNode] {
                CCNode* cellPoints = Build(CCNode::create())
                    .contentSize({
                        self->m_mainLayer->getContentWidth() / 2.f,
                        self->m_mainLayer->getContentHeight() / 8.f
                    })
                    .collect();

                CCNode* cellCoinPoints = Build(CCNode::create())
                    .contentSize({
                        self->m_mainLayer->getContentWidth() / 2.f,
                        self->m_mainLayer->getContentHeight() / 8.f
                    })
                    .collect();
                
                Build(CCLabelBMFont::create(fmt::format("Points: {}", parsed.points).c_str(), "bigFont.fnt"))
                    .scale(0.6f)
                    .parent(cellPoints)
                    .center()
                    .collect();

                Build(CCLabelBMFont::create(fmt::format("Coin Points: {}", parsed.coinPoints).c_str(), "bigFont.fnt"))
                    .scale(0.5f)
                    .parent(cellCoinPoints)
                    .center()
                    .collect();

                pointsListNode->addCell(cellPoints);
                pointsListNode->addCell(cellCoinPoints);
                return;
            };

            if (parsed.pointsInfo.empty()) {
                Build(CCLabelBMFont::create("No points info found.", "bigFont.fnt"))
                    .scale(0.4f)
                    .parent(pointsListNode)
                    .center()
                    .collect();
            } else {
                createInfoCells();
                for (auto const& val : parsed.pointsInfo) {
                    createListCell(val);
                }
            }

            self->m_adminButtonsMenu->updateLayout();
            self->m_helperButtonsMenu->updateLayout();
            self->m_levelInfoMenu->updateLayout();

            return;
        }
    );

    return true;
}

}