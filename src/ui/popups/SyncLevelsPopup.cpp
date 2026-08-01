#include "SyncLevelsPopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>
#include <cue/Slider.hpp>

#include "../../managers/APIClient.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

namespace levelgrind {

SyncLevelsPopup* SyncLevelsPopup::create() {
    auto ret = new SyncLevelsPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SyncLevelsPopup::init() {
    if (!BasePopup::init(240.f, 240.f)) return false;

    setTitle("Sync Levels");

    auto objMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setGap(10)->setAxisReverse(true))
        .scale(0.5f)
        .parent(m_mainLayer)
        .pos(
            centerX(), centerY() + 5.f
        )
        .collect();

    auto addThresholdLabel = Build(CCLabelBMFont::create("Add Threshold", "bigFont.fnt"))
        .parent(objMenu)
        .scale(0.5f)
        .collect();

    auto addThresholdInput = Build(TextInput::create(40.f, "1", "bigFont.fnt"))
        .with([](TextInput* input) {
            input->setMaxCharCount(2);
            input->setCommonFilter(CommonFilter::Int);
        })
        .parent(objMenu)
        .collect();

    auto deleteThresholdLabel = Build(CCLabelBMFont::create("Delete Threshold", "bigFont.fnt"))
        .parent(objMenu)
        .scale(0.5f)
        .collect();

    auto deleteThresholdInput = Build(TextInput::create(40.f, "-1", "bigFont.fnt"))
        .with([](TextInput* input) {
            input->setMaxCharCount(2);
            input->setCommonFilter(CommonFilter::Int);
        })
        .parent(objMenu)
        .collect();

    auto coinAddThresholdLabel = Build(CCLabelBMFont::create("Coin Add Threshold", "bigFont.fnt"))
        .parent(objMenu)
        .scale(0.5f)
        .collect();

    auto coinAddThresholdInput = Build(TextInput::create(40.f, "2", "bigFont.fnt"))
        .with([](TextInput* input) {
            input->setMaxCharCount(2);
            input->setCommonFilter(CommonFilter::Int);
        })
        .parent(objMenu)
        .collect();

    auto coinDeleteThresholdLabel = Build(CCLabelBMFont::create("Coin Delete Threshold", "bigFont.fnt"))
        .parent(objMenu)
        .scale(0.5f)
        .collect();

    auto coinDeleteThresholdInput = Build(TextInput::create(40.f, "2", "bigFont.fnt"))
        .with([](TextInput* input) {
            input->setMaxCharCount(2);
            input->setCommonFilter(CommonFilter::Int);
        })
        .parent(objMenu)
        .collect();

    auto syncBtn = Build(ButtonSprite::create("Sync", "bigFont.fnt", "GJ_button_01.png"))
        .scale(0.7f)
        .intoMenuItem([this, addThresholdInput, deleteThresholdInput, coinAddThresholdInput, coinDeleteThresholdInput] {
            Notification::create("Syncing levels...", NotificationIcon::Loading)->show();

            auto self = Ref(this);

            m_listener.spawn(
                APIClient::getInstance().syncLevels(
                    numFromString<int>(addThresholdInput->getString()).unwrap(),
                    numFromString<int>(deleteThresholdInput->getString()).unwrap(),
                    numFromString<int>(coinAddThresholdInput->getString()).unwrap(),
                    numFromString<int>(coinDeleteThresholdInput->getString()).unwrap()
                ),
                [self](web::WebResponse res) {
                    auto parsed = APIClient::getInstance().syncLevelsParse(res);

                    if (!parsed.ok) {
                        Notification::create("Failed to sync levels.", NotificationIcon::Error)->show();
                        return;
                    }

                    self->onClose(nullptr);

                    MDPopup::create(
                        "Results",
                        fmt::format(
                            "# <cp>Level Changes</c>\n\n"
                            "<cr>Deleted:</c> {}\n\n"
                            "<cg>Inserted:</c> {}\n\n"
                            "<cj>Coins changed:</c> {}\n\n",
                            parsed.deleted, parsed.inserted, parsed.coinUpdates
                        ).c_str(),
                        "OK"
                    )->show();

                    return;
                }
            );
        })
        .parent(m_buttonMenu)
        .pos(
            m_buttonMenu->getContentWidth() / 2,
            25
        )
        .collect();

    objMenu->updateLayout();

    return true;
}

}