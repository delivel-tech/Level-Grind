#include "AddAnnouncementPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

#include "../../core/DataManager.hpp"
#include "../../core/BackendManager.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/async.hpp"

using namespace geode::prelude;

namespace levelgrind {

namespace {
std::string normalizeAnnouncementContent(std::string content) {
    size_t pos = 0;
    while ((pos = content.find("\\r\\n", pos)) != std::string::npos) {
        content.replace(pos, 4, "\n");
    }

    pos = 0;
    while ((pos = content.find("\\n", pos)) != std::string::npos) {
        content.replace(pos, 2, "\n");
    }

    pos = 0;
    while ((pos = content.find("\\r", pos)) != std::string::npos) {
        content.replace(pos, 2, "\n");
    }

    return content;
}
}

AddAnnouncementPopup* AddAnnouncementPopup::create() {
    auto ret = new AddAnnouncementPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AddAnnouncementPopup::init() {
    if (!BasePopup::init({ 360.f, 220.f }, "GJ_square01.png")) return false;

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin) {
        setTitle("Grind Admin: Add Announcement");
    } else if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        setTitle("Grind Owner: Add Announcement");
    } else {
        setTitle("Unknown Role: Add Announcement");
        Notification::create("You are not allowed to add announcements.", NotificationIcon::Error)->show();
        return true;
    }

    m_titleTextInput = Build(TextInput::create(
        340.f, "Write down the announcement title...", "chatFont.fnt"
    ))
        .pos(
            centerX(),
            centerY() + 20.f
        )
        .parent(m_mainLayer)
        .id("title-text-input")
        .with([](TextInput* ti) {
            ti->setCommonFilter(CommonFilter::Any);
            ti->setMaxCharCount(100);
        })
        .collect();

    m_contentTextInput = Build(TextInput::create(
        340.f, "Write down the announcement content...", "chatFont.fnt"
    ))
        .pos(
            centerX(),
            centerY() - 20.f
        )
        .parent(m_mainLayer)
        .id("content-text-input")
        .with([](TextInput* ti) {
            ti->setCommonFilter(CommonFilter::Any);
            ti->setMaxCharCount(4000);
        })
        .collect();

    m_buttonsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .id("buttons-menu")
        .parent(m_buttonMenu)
        .pos(
            centerX(),
            30.f
        )
        .scale(0.8f)
        .collect();

    m_previewBtn = Build(ButtonSprite::create("Preview", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this] {
            auto content = normalizeAnnouncementContent(m_contentTextInput->getString());

            MDPopup::create(
                "Preview Announcement",
                fmt::format(
                    "# {}\n\n"
                    "{}\n"
                    "{}",
                    m_titleTextInput->getString(),
                    content,
                    GJAccountManager::sharedState()->m_username
                ),
                "OK"
            )->show();
        })
        .parent(m_buttonsMenu)
        .id("preview-btn")
        .collect();

    m_addBtn = Build(ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this] {
            createQuickPopup(
                "Are you sure?",
                "Please confirm adding new announcement.",
                "Cancel", "Confirm",
                [this](auto, bool btn2) {
                    if (btn2) {
                        async::spawn(this->onAddClicked());
                    }
                }
            );
        })
        .parent(m_buttonsMenu)
        .id("add-btn")
        .collect();

    m_buttonsMenu->updateLayout();

    return true;
}

arc::Future<> AddAnnouncementPopup::onAddClicked() {
    Ref<AddAnnouncementPopup> self = this;

    auto uPopup = UploadActionPopup::create(nullptr, "Adding announcement...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);

    auto parsed = co_await BackendManager::getInstance().addAnnouncement(
        m_titleTextInput->getString(),
        m_contentTextInput->getString()
    );

    if (!self || !uPopupRef) co_return;

    if (!parsed.ok) {
        log::error("bad web req. add announcement endpoint");
        uPopupRef->showFailMessage("Failed! Try again later.");
        co_return;
    }

    uPopupRef->showSuccessMessage("Success! Announcement added.");
    co_return;
}

}