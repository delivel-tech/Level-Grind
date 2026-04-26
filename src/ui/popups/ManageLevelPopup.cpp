#include "ManageLevelPopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <UIBuilder.hpp>
#include <fmt/format.h>
#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"

#include "../../utils/utils.hpp"

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
    if (!BasePopup::init({ 280.f, 200.f })) return false;

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
                .pos(self->centerX(), (self->m_title->getPositionY() - 20) / 2)
                .collect();

            self->m_adminButtonsMenu = Build<CCMenu>::create()
                .scale(0.8f)
                .layout(layout)
                .parent(self->m_buttonMenu)
                .id("admin-buttons-menu")
                .pos(self->centerX(), (self->m_title->getPositionY() - 20) / 2)
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
                    self->m_mainLayer->getContentWidth() / 1.2f,
                    self->m_mainLayer->getContentHeight() / 1.5f
                })
                .pos(self->centerX(), self->centerY() - 20)
                .id("menu-bg")
                .scale(0.9f)
                .opacity(80)
                .parent(self->m_mainLayer)
                .collect();

            // making helper menu
            if (parsed.isAdded) {
                Build(ButtonSprite::create("Delete", "bigFont.fnt", "GJ_button_06.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_helperButtonsMenu)
                    .collect();

                Build(ButtonSprite::create("Re-add", "bigFont.fnt", "GJ_button_02.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_helperButtonsMenu)
                    .collect();
            } else {
                Build(ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_01.png"))
                    .intoMenuItem([] {

                    })
                    .parent(self->m_helperButtonsMenu)
                    .collect();
            }

            // info button
            Build<CCSprite>::create("info_btn.png"_spr)
                .scale(0.4f)
                .intoMenuItem([parsed] {
                    MDPopup::create(
                        "Level Info",
                        fmt::format(
                            "# <cy>State:</c> __{}__\n"
                            "## <cp>Added by:</c> *<cy>{}</c>*\n"
                            "<cg>Star Filter:</c> __{}__\n\n"
                            "<cb>Moon Filter:</c> __{}__\n\n"
                            "<cj>Coin Filter:</c>: __{}__\n\n"
                            "<cr>Demon Filter:</c>: __{}__\n\n"
                            "<co>Locked:</c> __{}__",
                            parsed.isAdded ? "<cg>Added</c>" : "<cr>Not Added</c>",
                            !parsed.addedBy.empty() ? parsed.addedBy : "Not Added",
                            parsed.star ? "<cy>Yes</c>" : "<cr>No</c>",
                            parsed.moon ? "<cy>Yes</c>" : "<cr>No</c>",
                            parsed.coin ? "<cy>Yes</c>" : "<cr>No</c>",
                            parsed.demon ? "<cy>Yes</c>" : "<cr>No</c>",
                            parsed.isLocked ? "<cy>Yes</c>" : "<cr>No</c>"
                        ).c_str(),
                        "OK"
                    )->show();
                })
                .parent(self->m_levelInfoMenu);

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
                .pos({ self->m_buttonMenu->getContentWidth(), 0 })
                .collect();
            }

            // notes btn
            Build<CCSprite>::create("button_edit_note.png"_spr)
                .scale(0.6f)
                .intoMenuItem([] {

                })
                .parent(self->m_buttonMenu)
                .pos({ self->m_buttonMenu->getContentSize() });

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

            self->m_adminButtonsMenu->updateLayout();
            self->m_helperButtonsMenu->updateLayout();
            self->m_levelInfoMenu->updateLayout();

            return;
        }
    );

    return true;
}

}