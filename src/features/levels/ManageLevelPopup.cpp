#include "ManageLevelPopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include <cue/RadioLogic.hpp>
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>
#include "../../core/BackendManager.hpp"
#include "../../core/DataManager.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/async.hpp"

#include "../../utils/utils.hpp"
#include "AddNotePopup.hpp"
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

    async::spawn(this->onLoadLevelInfo());

    return true;
}

arc::Future<> ManageLevelPopup::onLoadLevelInfo() {
    Ref<ManageLevelPopup> self;
    co_await async::waitForMainThread([&] {
        self = this;
    });

    auto parsed = co_await BackendManager::getInstance().getLevelInfo(m_level->m_levelID);

    co_await async::waitForMainThread([&] {
    if (!self) return;

    if (!parsed.ok) {
        Notification::create("Failed to get level info. Try again later.", NotificationIcon::Error)->show();
        self->m_loadingSpinner->removeFromParent();
        return;
    }

    self->m_loadingSpinner->removeFromParent();

            self->setTitle("Manage Level");

            auto leftNineSlice = Build(NineSlice::create("GJ_square01.png"))
                .contentSize({50, 150})
                .parent(self->m_mainLayer)
                .pos({-30, self->centerY()})
                .id("left-nine-slice")
                .collect();

            auto rightNineSlice = Build(NineSlice::create("GJ_square01.png"))
                .contentSize({50, 150})
                .parent(self->m_mainLayer)
                .pos({self->m_mainLayer->getContentWidth() + 30, self->centerY()})
                .id("right-nine-slice")
                .collect();

            auto leftCoinMenu = Build(CCMenu::create())
                .layout(ColumnLayout::create()->setAxisReverse(true))
                .parent(self->m_buttonMenu)
                .contentSize(leftNineSlice->getContentSize())
                .pos(leftNineSlice->getPosition())
                .id("left-coin-menu")
                .scale(0.8f)
                .zOrder(1)
                .collect();

            auto rightButtonsMenu = Build(CCMenu::create())
                .layout(ColumnLayout::create()->setAxisReverse(true))
                .parent(self->m_buttonMenu)
                .contentSize(rightNineSlice->getContentSize())
                .pos(rightNineSlice->getPosition())
                .id("right-buttons-menu")
                .scale(0.8f)
                .zOrder(1)
                .collect();

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
                            Notification::create("This level has been locked. Voting won't be considered while syncing.", NotificationIcon::Warning)->show();
                        })
                        .collect()
                );
            } else {
                self->m_levelInfoMenu->addChild(
                    Build<CCSprite>::createSpriteName("GJ_lock_open_001.png")
                        .intoMenuItem([] {
                            Notification::create("This level is not locked. Voting will be considered while syncing.", NotificationIcon::Success)->show();
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
                    async::spawn(self->onAcceptClicked());
                })
                .parent(self->m_helperButtonsMenu)
                .collect();

            Build(ButtonSprite::create("Reject", "bigFont.fnt", "GJ_button_06.png"))
                .intoMenuItem([self] {
                    async::spawn(self->onRejectClicked());
                })
                .parent(self->m_helperButtonsMenu)
                .collect();

            auto adminTogglerOffSpr = CCSprite::create("GJ_button_04.png");
            auto adminTogglerOffSprTop = CCSprite::create("badge_admin.png"_spr);
            adminTogglerOffSpr->addChild(adminTogglerOffSprTop);
            adminTogglerOffSprTop->setPosition({20, 20});
            adminTogglerOffSprTop->setScale(1.2f);

            auto adminTogglerOnSpr = CCSprite::create("GJ_button_02.png");
            auto adminTogglerOnSprTop = CCSprite::create("badge_admin.png"_spr);
            adminTogglerOnSpr->addChild(adminTogglerOnSprTop);
            adminTogglerOnSprTop->setPosition({20, 20});
            adminTogglerOnSprTop->setScale(1.2f);

            // admin toggler
            if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin
            || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
                Build<CCMenuItemToggler>::createToggle(
                    adminTogglerOffSpr,
                    adminTogglerOnSpr,
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
                .parent(rightButtonsMenu)
                .collect();
            }

            auto notesSpr = CCSprite::create(parsed.noteExists ? "GJ_button_03.png" : "GJ_button_01.png");
            auto notesSprTop = CCSprite::create("button_edit_note.png"_spr);
            notesSpr->addChild(notesSprTop);
            notesSprTop->setPosition({20, 20});
            notesSprTop->setScale(0.6f);

            // notes btn
            Build(notesSpr)
                .intoMenuItem([self] {
                    AddNotePopup::create(self->m_level->m_levelID, self->m_level->m_levelName.c_str())->show();
                })
                .parent(rightButtonsMenu);

            if (parsed.noteExists) {
                auto deleteNotesSpr = CCSprite::create("GJ_button_06.png");
                auto deleteNotesSprTop = CCSprite::create("button_edit_note.png"_spr);;
                deleteNotesSpr->addChild(deleteNotesSprTop);
                deleteNotesSprTop->setPosition({20, 20});
                deleteNotesSprTop->setScale(0.6f);

                Build(deleteNotesSpr)
                    .intoMenuItem([self] {
                        async::spawn(self->onDeleteNotesClicked());
                    })
                    .parent(rightButtonsMenu);
            }

            enum class CoinManage {
                Reject = -1,
                NoVote = 0,
                Accept = 1
            };

            cue::RadioLogic<CoinManage> radio;

            auto rejectSprOff = CCSprite::create("GJ_button_06.png");
            auto rejectSprOffTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            rejectSprOff->addChild(rejectSprOffTop);
            rejectSprOffTop->setPosition({20, 20});

            rejectSprOff->setColor({100, 100, 100});

            auto rejectSprOn = CCSprite::create("GJ_button_06.png");
            auto rejectSprOnTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            rejectSprOn->addChild(rejectSprOnTop);
            rejectSprOnTop->setPosition({20, 20});

            auto novoteSprOff = CCSprite::create("GJ_button_04.png");
            auto novoteSprOffTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            novoteSprOff->addChild(novoteSprOffTop);
            novoteSprOffTop->setPosition({20, 20});

            novoteSprOff->setColor({100, 100, 100});

            auto novoteSprOn = CCSprite::create("GJ_button_04.png");
            auto novoteSprOnTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            novoteSprOn->addChild(novoteSprOnTop);
            novoteSprOnTop->setPosition({20, 20});

            auto acceptSprOff = CCSprite::create("GJ_button_01.png");
            auto acceptSprOffTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            acceptSprOff->addChild(acceptSprOffTop);
            acceptSprOffTop->setPosition({20, 20});

            acceptSprOff->setColor({100, 100, 100});

            auto acceptSprOn = CCSprite::create("GJ_button_01.png");
            auto acceptSprOnTop = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
            acceptSprOn->addChild(acceptSprOnTop);
            acceptSprOnTop->setPosition({20, 20});

            auto rejectCoinToggler = Build(radio.createToggler(CoinManage::Reject, rejectSprOff, rejectSprOn))
                .parent(leftCoinMenu)
                .collect();

            auto novoteCoinToggler = Build(radio.createToggler(CoinManage::NoVote, novoteSprOff, novoteSprOn))
                .parent(leftCoinMenu)
                .collect();

            auto acceptCoinToggler = Build(radio.createToggler(CoinManage::Accept, acceptSprOff, acceptSprOn))
                .parent(leftCoinMenu)
                .collect();

            leftCoinMenu->updateLayout();

            radio.setCallback([self](CoinManage which) {
                self->m_body.coin = static_cast<int>(which);
            });

            radio.select(CoinManage::NoVote);

            // admin menu
            if (parsed.isLocked) {
                Build(ButtonSprite::create("Unlock", "bigFont.fnt", "GJ_button_06.png"))
                    .intoMenuItem([self] {
                        async::spawn(self->onUnlockClicked());
                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            } else {
                Build(ButtonSprite::create("Lock", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([self] {
                        async::spawn(self->onLockClicked());
                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            }

            if (parsed.isAdded) {
                Build(ButtonSprite::create("Delete", "bigFont.fnt", "GJ_button_06.png"))
                    .intoMenuItem([self] {
                        async::spawn(self->onDeleteClicked());
                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();

                Build(ButtonSprite::create("Re-add", "bigFont.fnt", "GJ_button_02.png"))
                    .intoMenuItem([self] {
                        async::spawn(self->onAddOrReaddClicked(true));
                    })
                    .parent(self->m_adminButtonsMenu)
                    .collect();
            } else {
                Build(ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([self] {
                        async::spawn(self->onAddOrReaddClicked(false));
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

            bool hasVoted = false;
            for (auto const& val : parsed.pointsInfo) {
                if (val.staffId == GJAccountManager::sharedState()->m_accountID) {
                    hasVoted = true;
                    break;
                }
            }

            if (hasVoted) {
                Build(ButtonSprite::create("Cancel Vote", "bigFont.fnt", "GJ_button_06.png"))
                    .scale(0.65f)
                    .intoMenuItem([self] {
                        async::spawn(self->onCancelVoteClicked());
                    })
                    .parent(self->m_buttonMenu)
                    .pos(self->m_buttonMenu->getContentWidth() / 2.f, 0.f)
                    .id("cancel-vote-button");
            }

            self->m_adminButtonsMenu->updateLayout();
            self->m_helperButtonsMenu->updateLayout();
            self->m_levelInfoMenu->updateLayout();

            rightButtonsMenu->updateLayout();
    });

    co_return;
}

arc::Future<> ManageLevelPopup::onAcceptClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Adding point...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().changePoint(PointType::AcceptPoint, m_body.coin, m_body);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) { uPopupRef->showFailMessage("Failed to add point."); return; }
        uPopupRef->showSuccessMessage("Success! Added point.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onRejectClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Removing point...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().changePoint(PointType::RejectPoint, m_body.coin, m_body);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) { uPopupRef->showFailMessage("Failed to remove point."); return; }
        uPopupRef->showSuccessMessage("Success! Removed point.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onDeleteNotesClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Deleting notes...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().deleteNotes(
        m_level->m_levelID,
        m_level->m_levelName.empty() ? "blank name" : m_level->m_levelName.c_str()
    );

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) {
            log::error("bad web req");
            uPopupRef->showFailMessage("Failed! Try again later.");
            return;
        }
        uPopupRef->showSuccessMessage("Success! Notes deleted.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onUnlockClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Unlocking level...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().unlockLevel(m_level->m_levelID, m_level->m_levelName);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) {
            log::error("bad web req");
            uPopupRef->showFailMessage("Failed! Try again later.");
            return;
        }
        uPopupRef->showSuccessMessage("Success! Level unlocked.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onLockClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Locking level...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().lockLevel(m_level->m_levelID, m_level->m_levelName);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) {
            log::error("bad web req");
            uPopupRef->showFailMessage("Failed! Try again later.");
            return;
        }
        uPopupRef->showSuccessMessage("Success! Level locked.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onDeleteClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Deleting level...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().deleteLevel(m_level->m_levelID);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) {
            log::error("bad web req");
            uPopupRef->showFailMessage("Failed! Try again later.");
            return;
        }
        uPopupRef->showSuccessMessage("Success! Level deleted.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onAddOrReaddClicked(bool isReadd) {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, isReadd ? "Re-adding level..." : "Adding level...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().newlevel(m_body);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) { uPopupRef->showFailMessage("Failed! Try again later."); return; }
        uPopupRef->showSuccessMessage(isReadd ? "Success! Level re-added." : "Success! Level added.");
    });
    co_return;
}

arc::Future<> ManageLevelPopup::onCancelVoteClicked() {
    Ref<UploadActionPopup> uPopupRef;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Cancelling vote...");
        uPopup->show();
        uPopupRef = uPopup;
    });

    auto parsed = co_await BackendManager::getInstance().cancelVote(m_level->m_levelID);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) { uPopupRef->showFailMessage("Failed to cancel vote."); return; }
        uPopupRef->showSuccessMessage("Success! Vote cancelled.");
    });
    co_return;
}

}