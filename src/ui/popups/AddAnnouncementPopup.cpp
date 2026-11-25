#include "AddAnnouncementPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"

using namespace geode::prelude;

namespace levelgrind {

namespace {

constexpr const char* kDraftKey = "announcement-draft";

CCMenuItemSpriteExtra* smallButton(const char* text, const char* bg, float scale, geode::Function<void()> cb) {
    return Build(ButtonSprite::create(text, "goldFont.fnt", bg, 0.8f))
        .scale(scale)
        .intoMenuItem([cb = std::move(cb)]() mutable { cb(); })
        .collect();
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
    if (!BasePopup::init({ 460.f, 290.f }, "GJ_square01.png")) return false;

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin) {
        setTitle("Grind Admin: Add Announcement");
    } else if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        setTitle("Grind Coordinator: Add Announcement");
    } else {
        setTitle("Unknown Role: Add Announcement");
        Notification::create("You are not allowed to add announcements.", NotificationIcon::Error)->show();
        return true;
    }

    m_titleTextInput = Build(TextInput::create(420.f, "Announcement title", "chatFont.fnt"))
        .scale(0.8f)
        .pos({ centerX(), top() - 46.f })
        .parent(m_mainLayer)
        .id("title-text-input")
        .collect();
    m_titleTextInput->setCommonFilter(CommonFilter::Any);
    m_titleTextInput->setMaxCharCount(100);

    m_body = Build(TextInput::createMultiline({ 420.f, 168.f }, "Announcement text", "chatFont.fnt"))
        .pos({ centerX(), centerY() - 7.f })
        .parent(m_mainLayer)
        .id("announcement-body")
        .collect();
    m_body->setWrap(true);
    this->loadDraft();
    m_titleTextInput->setCallback([this](auto const&) { saveDraft(); });
    m_body->setCallback([this](auto const&) { saveDraft(); });

    auto previewBtn = smallButton("Full Preview", "GJ_button_04.png", 0.6f, [this] { this->onPreview(); });
    previewBtn->setPosition({ m_buttonMenu->getContentWidth() / 2.f - 60.f, 22.f });
    m_buttonMenu->addChild(previewBtn);

    auto postBtn = smallButton("Post", "GJ_button_01.png", 0.75f, [this] { this->onPost(); });
    postBtn->setPosition({ m_buttonMenu->getContentWidth() / 2.f + 60.f, 22.f });
    m_buttonMenu->addChild(postBtn);

    return true;
}

void AddAnnouncementPopup::onPreview() {
    MDPopup::create(
        "Preview Announcement",
        fmt::format(
            "# {}\n\n"
            "{}\n"
            "{}",
            m_titleTextInput->getString(),
            m_body->getString(),
            GJAccountManager::sharedState()->m_username
        ),
        "OK"
    )->show();
}

void AddAnnouncementPopup::onPost() {
    if (m_posting) return;

    std::string title = m_titleTextInput->getString();
    std::string content = m_body->getString();

    if (title.empty()) {
        Notification::create("Add a title first", NotificationIcon::Error)->show();
        return;
    }
    if (content.empty()) {
        Notification::create("The announcement is empty", NotificationIcon::Error)->show();
        return;
    }

    Ref<AddAnnouncementPopup> self = this;
    createQuickPopup(
        "Post announcement?",
        fmt::format("<cy>{}</c> will be shown to <cg>everyone</c>.", title),
        "Cancel", "Post",
        [self, title, content](auto, bool confirmed) {
            if (!confirmed || !self) return;

            self->m_posting = true;
            auto uPopup = UploadActionPopup::create(nullptr, "Adding announcement...");
            uPopup->show();
            Ref<UploadActionPopup> uPopupRef = uPopup;

            self->m_listener.spawn(
                APIClient::getInstance().addAnnouncement(title, content),
                [self, uPopupRef](web::WebResponse res) {
                    if (!self) return;
                    self->m_posting = false;

                    auto parsed = APIClient::getInstance().addAnnouncementParse(res);
                    if (!parsed.ok) {
                        log::error("bad web req. add announcement endpoint");
                        if (uPopupRef) uPopupRef->showFailMessage("Failed! Try again later.");
                        return;
                    }

                    if (uPopupRef) uPopupRef->showSuccessMessage("Success! Announcement added.");

                    self->m_body->setString("");
                    self->m_titleTextInput->setString("");
                    self->saveDraft();
                }
            );
        }
    );
}

void AddAnnouncementPopup::saveDraft() {
    matjson::Value draft;
    draft["title"] = m_titleTextInput->getString();
    draft["body"] = m_body->getString();
    Mod::get()->setSavedValue<matjson::Value>(kDraftKey, draft);
}

void AddAnnouncementPopup::loadDraft() {
    if (!Mod::get()->hasSavedValue(kDraftKey)) return;
    auto draft = Mod::get()->getSavedValue<matjson::Value>(kDraftKey);
    m_titleTextInput->setString(draft["title"].asString().unwrapOrDefault());
    if (auto body = draft["body"].asString()) {
        m_body->setString(body.unwrap());
        return;
    }

    auto lines = draft["lines"].asArray();
    if (!lines) return;
    std::string markdown;
    int previousKind = -1;
    constexpr std::string_view tags = " yorpabljg";
    for (auto const& line : lines.unwrap()) {
        int kind = line["kind"].asInt().unwrapOr(0);
        std::string text;
        if (kind == 4) text = "---";
        else if (auto spans = line["spans"].asArray()) {
            for (auto const& span : spans.unwrap()) {
                auto part = span["text"].asString().unwrapOrDefault();
                if (span["bold"].asBool().unwrapOr(false)) part = "**" + part + "**";
                if (span["italic"].asBool().unwrapOr(false)) part = "*" + part + "*";
                auto color = span["color"].asInt().unwrapOr(0);
                if (color > 0 && color < tags.size()) part = fmt::format("<c{}>{}</c>", tags[color], part);
                if (!text.empty()) text += " ";
                text += part;
            }
        }
        if (text.empty()) continue;
        if (!markdown.empty()) markdown += kind == 3 && previousKind == 3 ? "\n" : "\n\n";
        if (kind == 1) markdown += "## ";
        if (kind == 2) markdown += "### ";
        if (kind == 3) markdown += "- ";
        markdown += text;
        previousKind = kind;
    }
    m_body->setString(markdown);
}
}
