#include "EventsManagePopup.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

#include "../../managers/APIClient.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

namespace levelgrind {

EventsManagePopup* EventsManagePopup::create() {
    auto ret = new EventsManagePopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

struct EventSection {
    int mode;
    std::string name;
    std::string menuId;
};

bool EventsManagePopup::init() {
    if (!BasePopup::init({300, 190})) return false;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Queue Events");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Owner: Queue Events");
    } else {
        this->onClose(nullptr);
    }

    // i love walls of code
    auto columnMenu = Build<CCMenu>::create()
        .layout(ColumnLayout::create()->setAxisReverse(true))
        .parent(m_buttonMenu)
        .pos(center())
        .scale(0.75f)
        .id("column-menu")
        .collect();

    std::vector<EventSection> sections {
        {0, "Daily", "daily-menu"},
        {1, "Weekly", "weekly-menu"},
        {2, "Monthly", "monthly-menu"}
    };

    for (const auto& section : sections) {
        auto menu = Build<CCMenu>::create()
            .layout(RowLayout::create()->setAutoScale(false))
            .parent(columnMenu)
            .id(section.menuId)
            .collect();

        auto nameLabel = Build(CCLabelBMFont::create(
            section.name.c_str(), "bigFont.fnt"
        ))
            .scale(0.6f)
            .parent(menu)
            .collect();

        auto classicInput = Build(TextInput::create(80, "Classic ID", "chatFont.fnt"))
            .with([](TextInput* input) {
                input->setCommonFilter(CommonFilter::Int);
            })
            .parent(menu)
            .collect();

        auto platInput = Build(TextInput::create(80, "Platformer ID", "chatFont.fnt"))
            .with([](TextInput* input) {
                input->setCommonFilter(CommonFilter::Int);
            })
            .parent(menu)
            .collect();

        auto setBtn = Build(ButtonSprite::create("Queue", "bigFont.fnt", "GJ_button_01.png"))
            .scale(0.8f)
            .intoMenuItem([section, this, classicInput, platInput] {
                if (!numFromString<int>(classicInput->getString()) || !numFromString<int>(platInput->getString())) {
                    Notification::create("Invalid IDs", NotificationIcon::Error)->show();
                    return;
                }

                auto uPopup = UploadActionPopup::create(nullptr, fmt::format("Queueing {}...", section.name));
                uPopup->show();

                auto uPopupRef = Ref(uPopup);
                auto self = Ref(this);

                m_listener.spawn(
                    APIClient::getInstance().setEvents(
                        section.mode, 
                        numFromString<int>(classicInput->getString()).unwrap(), 
                        numFromString<int>(platInput->getString()).unwrap()
                    ),
                    [uPopupRef, self](web::WebResponse res) {
                        if (!uPopupRef || !self) return;
                        bool ok = APIClient::getInstance().setEventsParse(res);

                        if (ok) {
                            uPopupRef->showSuccessMessage("Success! Events queued.");
                        } else {
                            uPopupRef->showFailMessage("Failed! Try again later.");
                        }
                    }
                );
            })
            .parent(menu)
            .collect();

        menu->updateLayout();
    }

    columnMenu->updateLayout();

    // .substr(0, announcement.createdAt.find('T'))

    auto loading = Build(LoadingSpinner::create(20))
        .parent(m_mainLayer)
        .pos(centerX(), centerY() - 55)
        .id("loading-spinner")
        .collect();

    auto self = Ref(this);
    auto loadingRef = Ref(loading);

    m_listener.spawn(
        web::WebRequest().get("https://api.delivel.tech/get_events_dates"),
        [self, loadingRef](web::WebResponse res) {
            if (!self || !loadingRef) return;
            if (!res.ok()) {
                log::error("bad req");
                loadingRef->removeFromParent();
                return;
            }

            auto jsonRes = res.json();

            if (!jsonRes) {
                log::error("bad req");
                loadingRef->removeFromParent();
                return;
            }

            auto json = jsonRes.unwrap();

            auto dates = json["dates"];

            std::string dailyDate = dates["daily"].asString().unwrapOrDefault();
            std::string weeklyDate = dates["weekly"].asString().unwrapOrDefault();
            std::string monthlyDate = dates["monthly"].asString().unwrapOrDefault();

            loadingRef->removeFromParent();

            auto rowMenu = Build<CCMenu>::create()
                .layout(RowLayout::create()->setGap(10))
                .parent(self->m_mainLayer)
                .scale(0.65f)
                .pos(self->centerX(), self->centerY() - 55)
                .collect();

            auto dailyLabel = Build(CCLabelBMFont::create(
                fmt::format("{}: {}", "Daily", dailyDate.substr(0, dailyDate.find('T'))).c_str(), "chatFont.fnt"
            ))
                .parent(rowMenu)
                .collect();

            auto weeklyLabel = Build(CCLabelBMFont::create(
                fmt::format("{}: {}", "Weekly", weeklyDate.substr(0, weeklyDate.find('T'))).c_str(), "chatFont.fnt"
            ))
                .parent(rowMenu)
                .collect();

            auto monthlyLabel = Build(CCLabelBMFont::create(
                fmt::format("{}: {}", "Monthly", monthlyDate.substr(0, monthlyDate.find('T'))).c_str(), "chatFont.fnt"
            ))
                .parent(rowMenu)
                .collect();

            rowMenu->updateLayout();
        }
    );

    return true;
}

}