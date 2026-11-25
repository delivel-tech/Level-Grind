#include "EventsManagePopup.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/ui/Label.hpp>
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

#include "../../managers/APIClient.hpp"
#include "MoveEventsPopup.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

namespace levelgrind {

namespace {

struct EventDef {
    const char* name;
    const char* dateKey;
    const char* crown;
    ccColor3B color;
};

const EventDef kEvents[] = {
    { "Daily",   "daily",   "crown_d.png"_spr,      { 255, 230, 120 } },
    { "Weekly",  "weekly",  "crown_w.png"_spr,      { 200, 160, 255 } },
    { "Monthly", "monthly", "crown_m.png"_spr,      { 120, 220, 255 } },
};

constexpr CCSize kCardSize { 310.f, 50.f };

}

EventsManagePopup* EventsManagePopup::create() {
    auto ret = new EventsManagePopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EventsManagePopup::init() {
    if (!BasePopup::init({ 340.f, 280.f })) return false;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Queue Events");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Coordinator: Queue Events");
    } else {
        this->onClose(nullptr);
    }

    Build(geode::Label::create("Queue the next level for each event (classic + platformer).", "chatFont.fnt"))
        .scale(0.55f)
        .pos(fromTop(40.f))
        .opacity(170)
        .parent(m_mainLayer);

    float const firstY = top() - 82.f;
    float const gap = kCardSize.height + 8.f;

    for (int mode = 0; mode < 3; mode++) {
        this->addEventCard(mode, firstY - gap * mode);
    }

    Ref<EventsManagePopup> self = this;
    Build(ButtonSprite::create("Move Events", "goldFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.6f)
        .intoMenuItem([self] {
            MoveEventsPopup::create([self] { if (self) self->loadDates(); })->show();
        })
        .pos({ m_buttonMenu->getContentWidth() / 2.f, 24.f })
        .id("move-events-btn")
        .parent(m_buttonMenu);

    this->loadDates();

    return true;
}

void EventsManagePopup::addEventCard(int mode, float y) {
    auto const& def = kEvents[mode];
    auto& card = m_cards[mode];

    auto cardNode = Build<CCNode>::create()
        .contentSize(kCardSize)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos({ centerX(), y })
        .id(fmt::format("{}-card", def.dateKey))
        .parent(m_mainLayer)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(kCardSize)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(75)
        .parent(cardNode);

    float const midY = kCardSize.height / 2.f;
    float textX = 12.f;

    if (auto crown = CCSprite::create(def.crown)) {
        crown->setScale(28.f / std::max(crown->getContentWidth(), crown->getContentHeight()));
        crown->setPosition({ 24.f, midY });
        cardNode->addChild(crown);
        textX = 44.f;
    }

    Build(geode::Label::create(def.name, "goldFont.fnt"))
        .scale(0.6f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ textX, midY + 8.f })
        .color(def.color)
        .parent(cardNode);

    card.dateLabel = Build(geode::Label::create("Last queued: ...", "chatFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ textX, midY - 9.f })
        .opacity(170)
        .parent(cardNode)
        .collect();

    auto makeInput = [&](const char* placeholder, float x) {
        auto input = Build(TextInput::create(78.f, placeholder, "chatFont.fnt"))
            .scale(0.8f)
            .pos({ x, midY })
            .parent(cardNode)
            .collect();
        input->setCommonFilter(CommonFilter::Uint);
        input->setMaxCharCount(10);
        return input;
    };

    card.classic = makeInput("Classic ID", 150.f);
    card.plat = makeInput("Platformer ID", 218.f);

    auto menu = Build<CCMenu>::create()
        .contentSize(kCardSize)
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(cardNode)
        .collect();

    Build(ButtonSprite::create("Queue", "goldFont.fnt", "GJ_button_01.png", 0.8f))
        .scale(0.6f)
        .intoMenuItem([this, mode] { this->onQueue(mode); })
        .pos({ kCardSize.width - 30.f, midY })
        .id("queue-btn")
        .parent(menu);
}

void EventsManagePopup::onQueue(int mode) {
    if (m_queueing) return;

    auto const& def = kEvents[mode];
    auto& card = m_cards[mode];

    auto classicID = numFromString<int>(card.classic->getString());
    auto platID = numFromString<int>(card.plat->getString());

    if (!classicID || !platID || classicID.unwrap() <= 0 || platID.unwrap() <= 0) {
        Notification::create(
            fmt::format("Enter both a Classic and a Platformer ID for {}", def.name),
            NotificationIcon::Error
        )->show();
        return;
    }

    m_queueing = true;

    auto uPopup = UploadActionPopup::create(nullptr, fmt::format("Queueing {}...", def.name));
    uPopup->show();

    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<EventsManagePopup> self = this;

    m_listener.spawn(
        APIClient::getInstance().setEvents(mode, classicID.unwrap(), platID.unwrap()),
        [uPopupRef, self, mode](web::WebResponse res) {
            if (!self) return;
            self->m_queueing = false;

            bool ok = APIClient::getInstance().setEventsParse(res);

            if (!ok) {
                if (uPopupRef) uPopupRef->showFailMessage("Failed! Try again later.");
                return;
            }

            if (uPopupRef) uPopupRef->showSuccessMessage(fmt::format("Success! {} queued.", kEvents[mode].name));

            self->m_cards[mode].classic->setString("");
            self->m_cards[mode].plat->setString("");
            self->loadDates();
        }
    );
}

void EventsManagePopup::loadDates() {
    Ref<EventsManagePopup> self = this;

    m_datesListener.spawn(
        web::WebRequest().get(fmt::format("{}/get_events_dates", APIClient::getInstance().baseUrl)),
        [self](web::WebResponse res) {
            if (!self) return;

            auto setAll = [&](auto&& textFor) {
                for (int mode = 0; mode < 3; mode++) {
                    if (auto label = self->m_cards[mode].dateLabel) {
                        label->setString(textFor(mode).c_str());
                    }
                }
            };

            auto jsonRes = res.json();
            if (!res.ok() || !jsonRes) {
                log::error("get_events_dates failed, code: {}", res.code());
                setAll([](int) { return std::string("Last queued: unknown"); });
                return;
            }

            auto dates = jsonRes.unwrap()["dates"];
            setAll([&](int mode) {
                std::string date = dates[kEvents[mode].dateKey].asString().unwrapOrDefault();
                if (date.empty()) return std::string("Last queued: never");
                return fmt::format("Last queued: {}", date.substr(0, date.find('T')));
            });
        }
    );
}

}