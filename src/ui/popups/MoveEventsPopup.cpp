#include "MoveEventsPopup.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/general.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <algorithm>
#include <fmt/format.h>

#include "../../managers/APIClient.hpp"

namespace levelgrind {

namespace {

constexpr CCSize kPanelSize { 300.f, 170.f };

constexpr const char* kModeNames[] = { "Daily", "Weekly", "Monthly" };

ButtonSprite* makeSegment(const char* caption, bool on) {
    return ButtonSprite::create(caption, 54, true, "goldFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 26.f, 0.6f);
}

void layoutRow(CCMenu* menu, std::vector<CCMenuItemToggler*> const& items, float y, float rightEdge) {
    float const gap = 4.f;
    float total = -gap;
    for (auto item : items) total += item->getScaledContentWidth() + gap;

    float x = rightEdge - total;
    for (auto item : items) {
        float w = item->getScaledContentWidth();
        item->setPosition({ x + w / 2.f, y });
        menu->addChild(item);
        x += w + gap;
    }
}

CCLabelBMFont* rowLabel(CCNode* panel, const char* text, float y) {
    return Build(CCLabelBMFont::create(text, "goldFont.fnt"))
        .scale(0.5f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 14.f, y })
        .parent(panel)
        .collect();
}

}

MoveEventsPopup* MoveEventsPopup::create(geode::Function<void()> onMoved) {
    auto ret = new MoveEventsPopup;
    if (ret && ret->init(std::move(onMoved))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoveEventsPopup::init(geode::Function<void()> onMoved) {
    if (!BasePopup::init({ 340.f, 280.f })) return false;

    m_onMoved = std::move(onMoved);

    this->setTitle("Move Events");

    Build(CCLabelBMFont::create("Shift queued events in time, e.g. to close a gap in the queue.", "chatFont.fnt"))
        .scale(0.5f)
        .pos(fromTop(40.f))
        .opacity(170)
        .parent(m_mainLayer);

    auto panel = Build<CCNode>::create()
        .contentSize(kPanelSize)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos({ centerX(), centerY() + 4.f })
        .parent(m_mainLayer)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(kPanelSize)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(75)
        .parent(panel);

    auto menu = Build<CCMenu>::create()
        .contentSize(kPanelSize)
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(panel)
        .collect();

    float const rightEdge = kPanelSize.width - 12.f;

    float y = kPanelSize.height - 22.f;
    rowLabel(panel, "Events", y);
    {
        constexpr std::pair<int, const char*> kModes[] = { { -1, "All" }, { 0, "Daily" }, { 1, "Weekly" }, { 2, "Monthly" } };
        std::vector<CCMenuItemToggler*> items;
        for (auto const& [mode, caption] : kModes) {
            auto t = m_modeRadio.createToggler(mode, makeSegment(caption, true), makeSegment(caption, false));
            t->setScale(0.75f);
            items.push_back(t);
        }
        layoutRow(menu, items, y, rightEdge);
        m_modeRadio.select(-1);
    }

    y -= 32.f;
    rowLabel(panel, "Direction", y);
    {
        constexpr std::pair<int, const char*> kDirs[] = { { 1, "Later" }, { -1, "Earlier" } };
        std::vector<CCMenuItemToggler*> items;
        for (auto const& [dir, caption] : kDirs) {
            auto t = m_dirRadio.createToggler(dir, makeSegment(caption, true), makeSegment(caption, false));
            t->setScale(0.75f);
            items.push_back(t);
        }
        layoutRow(menu, items, y, rightEdge);
        m_dirRadio.select(1);
    }

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, kPanelSize.width - 20.f, 1.f))
        .pos({ 10.f, y - 17.f })
        .parent(panel);

    y -= 34.f;
    m_daysInput = this->addStepper(panel, menu, "Days", y, 1, 0, 365);
    y -= 30.f;
    m_hoursInput = this->addStepper(panel, menu, "Hours", y, 0, 0, 23);

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, kPanelSize.width - 20.f, 1.f))
        .pos({ 10.f, y - 16.f })
        .parent(panel);

    y -= 32.f;
    m_sinceInput = this->addStepper(panel, menu, "Ended within", y, 0, 0, 365);
    Build(CCLabelBMFont::create("days (for events stuck in the past)", "chatFont.fnt"))
        .scale(0.5f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 14.f, y - 12.f })
        .opacity(150)
        .parent(panel);

    Build(CCLabelBMFont::create("Events that have not ended yet always move", "bigFont.fnt"))
        .scale(0.3f)
        .pos({ centerX(), 50.f })
        .opacity(160)
        .parent(m_mainLayer);

    Build(ButtonSprite::create("Move", "goldFont.fnt", "GJ_button_01.png", 0.9f))
        .scale(0.8f)
        .intoMenuItem([this] { this->onMove(); })
        .pos({ m_buttonMenu->getContentWidth() / 2.f, 22.f })
        .id("move-btn")
        .parent(m_buttonMenu);

    return true;
}

TextInput* MoveEventsPopup::addStepper(CCNode* panel, CCMenu* menu, const char* label, float y, int initial, int min, int max) {
    rowLabel(panel, label, y);

    float const x = kPanelSize.width - 52.f;

    auto input = Build(TextInput::create(56.f, "0", "bigFont.fnt"))
        .scale(0.65f)
        .pos({ x, y })
        .parent(panel)
        .collect();
    input->setMaxCharCount(3);
    input->setCommonFilter(CommonFilter::Uint);
    input->setString(numToString(initial));

    auto step = [input, min, max](int delta) {
        int value = numFromString<int>(input->getString()).unwrapOr(0);
        input->setString(numToString(std::clamp(value + delta, min, max)));
    };

    Build<CCSprite>::createSpriteName("edit_leftBtn_001.png")
        .scale(0.7f)
        .intoMenuItem([step] { step(-1); })
        .pos({ x - 32.f, y })
        .parent(menu);

    Build<CCSprite>::createSpriteName("edit_rightBtn_001.png")
        .scale(0.7f)
        .intoMenuItem([step] { step(1); })
        .pos({ x + 32.f, y })
        .parent(menu);

    return input;
}

std::string MoveEventsPopup::summary(MoveEventsResponse const& res, int days, int hours) {
    std::string out;
    for (auto const& e : res.events) {
        if (e.mode < 0 || e.mode > 2) continue;
        if (e.count == 0) {
            out += fmt::format("<cl>{}</c>: nothing to move\n", kModeNames[e.mode]);
        } else {
            out += fmt::format(
                "<cl>{}</c>: <cy>{}</c> event{}, {} to {}\n",
                kModeNames[e.mode], e.count, e.count == 1 ? "" : "s", e.newStart, e.newEnd
            );
        }
    }
    return out;
}

void MoveEventsPopup::onMove() {
    if (m_busy) return;

    auto days = numFromString<int>(m_daysInput->getString());
    auto hours = numFromString<int>(m_hoursInput->getString());
    auto since = numFromString<int>(m_sinceInput->getString());

    if (!days || !hours || !since || days.unwrap() > 365 || hours.unwrap() > 23 || since.unwrap() > 365) {
        Notification::create("Check the numbers: days 0-365, hours 0-23", NotificationIcon::Error)->show();
        return;
    }
    if (days.unwrap() == 0 && hours.unwrap() == 0) {
        Notification::create("Set how far to move the events", NotificationIcon::Error)->show();
        return;
    }

    int dir = m_dirRadio.getSelected();
    int d = days.unwrap() * dir;
    int h = hours.unwrap() * dir;
    int mode = m_modeRadio.getSelected();
    int sinceDays = since.unwrap();

    m_busy = true;

    auto uPopup = UploadActionPopup::create(nullptr, "Checking events...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<MoveEventsPopup> self = this;

    m_listener.spawn(
        APIClient::getInstance().moveEvents(mode, d, h, sinceDays, true),
        [self, uPopupRef, mode, d, h, sinceDays](web::WebResponse res) {
            if (!self) return;
            self->m_busy = false;

            auto parsed = APIClient::getInstance().moveEventsParse(res);
            if (!parsed.ok) {
                if (uPopupRef) uPopupRef->showFailMessage(fmt::format("Failed! {}", parsed.error));
                return;
            }
            if (uPopupRef) uPopupRef->closePopup();

            int total = 0;
            for (auto const& e : parsed.events) total += e.count;
            if (total == 0) {
                Notification::create("No events to move. Try including ended events.", NotificationIcon::Info)->show();
                return;
            }

            std::string by = fmt::format(
                "{}{}d {}h", d < 0 || h < 0 ? "-" : "+", std::abs(d), std::abs(h)
            );

            createQuickPopup(
                "Move events?",
                fmt::format("Move by <cg>{}</c>. New times:\n{}", by, summary(parsed, d, h)),
                "Cancel", "Move", 380.f,
                [self, mode, d, h, sinceDays](auto, bool confirmed) {
                    if (confirmed && self) self->runMove(mode, d, h, sinceDays);
                }
            );
        }
    );
}

void MoveEventsPopup::runMove(int mode, int days, int hours, int sinceDays) {
    m_busy = true;

    auto uPopup = UploadActionPopup::create(nullptr, "Moving events...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<MoveEventsPopup> self = this;

    m_listener.spawn(
        APIClient::getInstance().moveEvents(mode, days, hours, sinceDays, false),
        [self, uPopupRef](web::WebResponse res) {
            if (!self) return;
            self->m_busy = false;

            auto parsed = APIClient::getInstance().moveEventsParse(res);
            if (!uPopupRef) return;

            if (!parsed.ok) {
                uPopupRef->showFailMessage(fmt::format("Failed! {}", parsed.error));
                return;
            }

            uPopupRef->showSuccessMessage("Success! Events moved.");
            if (self->m_onMoved) self->m_onMoved();
        }
    );
}

}