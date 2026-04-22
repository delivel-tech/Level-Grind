#include "AnnouncementsPopup.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>

#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/ui/MDTextArea.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include "Geode/utils/web.hpp"

#include "AddAnnouncementPopup.hpp"
#include <cue/ListNode.hpp>

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

AnnouncementsPopup* AnnouncementsPopup::create() {
    auto ret = new AnnouncementsPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AnnouncementsPopup::init() {
    if (!Popup::init(460.f, 270.f, "GJ_square01.png")) return false;

    m_spinner = Build(LoadingSpinner::create(40.f))
        .parent(m_mainLayer)
        .center()
        .collect();

    auto spinnerRef = Ref(m_spinner);
    auto self = Ref(this);

    m_listener.spawn(
        APIClient::getInstance().getAnnouncements(),
        [spinnerRef, self](web::WebResponse res) {
            if (!spinnerRef || !self) return;
            auto parsed = APIClient::getInstance().getAnnouncementsParse(res);

            if (!parsed.ok) {
                spinnerRef->removeFromParent();
                log::error("bad web req. get announcements endpoint");
                Notification::create("Failed to load announcements! Try again later.", NotificationIcon::Error)->show();
                return;
            }

            self->m_listNode = Build(cue::ListNode::create(
                { 140.f, 250.f }
            ))
                .pos(
                    80,
                    self->m_mainLayer->getContentHeight() / 2.f - 10.f
                )
                .scale(0.9f)
                .parent(self->m_mainLayer)
                .id("list-node")
                .collect();

            self->m_scrollBar = Build(Scrollbar::create(self->m_listNode->getScrollLayer()))
                .pos(
                    150,
                    self->m_mainLayer->getContentHeight() / 2.f - 10.f
                )
                .scale(0.8f)
                .parent(self->m_mainLayer)
                .id("scrollbar")
                .collect();

            self->setTitle("Level Grind Announcements");

            auto createListCell = [](std::string createdAtDate, auto cb, int sectionId) {
                auto section = Build(CCMenu::create())
                    .id(fmt::format("section-{}", sectionId))
                    .contentSize({ 140.f, 30.f })
                    .collect();

                auto sectionButton = Build(ButtonSprite::create(createdAtDate.c_str(), 150.f, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 0.7f))
                    .scale(0.7f)
                    .intoMenuItem(cb)
                    .scaleMult(1.1f)
                    .parent(section)
                    .id(fmt::format("section-button-{}", sectionId))
                    .center()
                    .collect();

                return section;
            };

            for (auto const& announcement : parsed.announcements) {
                auto content = normalizeAnnouncementContent(announcement.content);

                auto textArea = Build(MDTextArea::create(
                    fmt::format(
                        "# {}\n\n"
                        "{}\n"
                        "{}",
                        announcement.title, content, announcement.addedBy
                    ),
                    { 323.f, 250.f }
                ))
                    .pos(
                        self->m_mainLayer->getContentWidth() / 2.f + 73.f,
                        self->m_mainLayer->getContentHeight() / 2.f - 10.f
                    )
                    .scale(0.9f)
                    .parent(self->m_mainLayer)
                    .visible(false)
                    .collect();

                self->m_textAreas.push_back(textArea);

                self->m_listNode->addCell(createListCell(
                    announcement.createdAt.substr(0, announcement.createdAt.find('T')), [self, textArea, announcement] {
                        for (auto const& area : self->m_textAreas) {
                            if (textArea == area) {
                                area->setVisible(true);
                                self->m_currentAnnouncementID = announcement.id;
                            } else {
                                area->setVisible(false);
                            }
                        }
                    }, announcement.id
                ));

                if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
                    auto announcementId = announcement.id;

                    auto deleteButtonsMenu = Build(CCMenu::create())
                        .pos(textArea->getContentWidth() - 35.f, 17.f)
                        .parent(textArea)
                        .id("delete-btn-menu")
                        .collect();

                    auto deleteBtn = Build(ButtonSprite::create("Delete", "bigFont.fnt", "GJ_button_06.png"))
                        .scale(0.55f)
                        .anchorPoint({ 1.f, 0.5f })
                        .intoMenuItem([self, announcementId] {
                            createQuickPopup(
                                "Are you sure?",
                                "Confirm that you want to delete this announcement",
                                "Cancel", "Confirm",
                                [self, announcementId](auto, bool btn2) {
                                    if (btn2) {
                                        auto uPopup = UploadActionPopup::create(nullptr, "Deleting announcement...");
                                        uPopup->show();

                                        auto uPopupRef = Ref(uPopup);

                                        self->m_listener.spawn(
                                            APIClient::getInstance().deleteAnnouncement(announcementId),
                                            [uPopupRef](web::WebResponse res) {
                                                if (!uPopupRef) return;
                                                auto parsed = APIClient::getInstance().deleteAnnouncementParse(res);

                                                if (!parsed.ok) {
                                                    log::error("bad web req, endpoint deleteAnnouncement");
                                                    uPopupRef->showFailMessage("Failed! Try again later.");
                                                    return;
                                                } else {
                                                    uPopupRef->showSuccessMessage("Success! Announcement deleted.");
                                                    return;
                                                }
                                            }
                                        );
                                    }
                                }
                            );
                        })
                        .parent(deleteButtonsMenu)
                        .collect();
                }
            }

            spinnerRef->removeFromParent();

            if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin || DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
                auto section = Build(CCMenu::create())
                    .id(fmt::format("section-admin"))
                    .contentSize({ 140.f, 30.f })
                    .collect();

                auto newAnnBtn = Build(CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"))
                    .scale(0.5f)
                    .intoMenuItem([] {
                        AddAnnouncementPopup::create()->show();
                    })
                    .scaleMult(1.1f)
                    .parent(section)
                    .id("section-button-admin")
                    .center()
                    .collect();

                self->m_listNode->addCell(section);
            }

            self->m_textAreas[0]->setVisible(true);
            self->m_listNode->scrollToTop();
            self->m_listNode->getScrollLayer()->m_contentLayer->updateLayout();

            return;
        }
    );

    return true;
}

}