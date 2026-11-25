#include "../../managers/Auth.hpp"
#include <Geode/ui/Label.hpp>
#include "StaffActivityPopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

constexpr CCSize kListSize { 300.f, 170.f };
constexpr const char* kTabNames[] = { "Today", "7 Days", "30 Days", "All Time" };

}

StaffActivityPopup* StaffActivityPopup::create() {
    auto ret = new StaffActivityPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StaffActivityPopup::init() {
    if (!BasePopup::init(340.f, 280.f, "GJ_square01.png")) return false;
    this->setTitle("Staff Activity");

    m_tabMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(6.f)->setAutoScale(false))
        .contentSize({ 300.f, 30.f })
        .pos({ centerX(), top() - 46.f })
        .id("tabs")
        .parent(m_mainLayer)
        .collect();

    m_list = Build(cue::ListNode::create(kListSize))
        .pos({ centerX(), centerY() - 12.f })
        .id("activity-list")
        .parent(m_mainLayer)
        .collect();
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(26.f);

    auto toggleRow = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(6.f)->setAutoScale(false))
        .contentSize({ 200.f, 26.f })
        .pos({ centerX(), 22.f })
        .id("notes-toggle")
        .parent(m_mainLayer)
        .collect();
    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.6f, [this](CCMenuItemToggler* t) {
        m_notes = !t->isToggled();
        this->load(m_mode);
    });
    toggleRow->addChild(toggler);
    auto toggleLabel = geode::Label::create("Show notes instead of votes", "bigFont.fnt");
    toggleLabel->setScale(0.32f);
    toggleRow->addChild(toggleLabel);
    toggleRow->updateLayout();

    this->load(1);
    return true;
}

void StaffActivityPopup::load(int mode) {
    m_mode = mode;

    m_tabMenu->removeAllChildren();
    for (int i = 0; i < 4; i++) {
        auto spr = ButtonSprite::create(kTabNames[i], 62, true, "bigFont.fnt", i == mode ? "GJ_button_02.png" : "GJ_button_04.png", 26.f, 0.5f);
        spr->setScale(0.8f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, i](CCObject*) {
            if (i != m_mode) this->load(i);
        });
        m_tabMenu->addChild(btn);
    }
    m_tabMenu->updateLayout();

    m_list->clear();
    m_list->updateLayout(false);
    if (m_status) m_status->removeFromParent();
    auto spinner = LoadingSpinner::create(30.f);
    spinner->setPosition(m_list->getPosition());
    m_mainLayer->addChild(spinner, 10);
    m_status = spinner;

    Ref<StaffActivityPopup> self = this;
    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["mode"] = mode;
    m_listener.spawn(
        auth::post(fmt::format("{}/{}", APIClient::getInstance().baseUrl,
            m_notes ? "get_staff_notes_activity" : "get_staff_activity"), body),
        [self, mode, notes = m_notes](web::WebResponse res) {
            if (!self || self->m_mode != mode || self->m_notes != notes) return;
            if (self->m_status) {
                self->m_status->removeFromParent();
                self->m_status = nullptr;
            }

            auto showText = [&](std::string const& text) {
                auto lbl = geode::Label::create(text, "bigFont.fnt");
                lbl->setScale(0.4f);
                lbl->setLimitLabelWidth(kListSize.width - 30.f, 0.4f, 0.2f);
                lbl->setPosition(self->m_list->getPosition());
                self->m_mainLayer->addChild(lbl, 10);
                self->m_status = lbl;
            };

            auto json = res.json();
            if (!res.ok() || !json) {
                showText(fmt::format("Failed to load: {}", APIClient::errorMessage(res, fmt::format("code {}", res.code()))));
                return;
            }

            auto staff = json.unwrap()["staff"].asArray();
            if (!staff || staff.unwrap().empty()) {
                showText("No staff found.");
                return;
            }

            for (auto const& s : staff.unwrap()) {
                auto row = CCNode::create();
                row->setContentSize({ kListSize.width, 26.f });

                auto name = geode::Label::create(s["username"].asString().unwrapOrDefault().c_str(), "bigFont.fnt");
                name->setScale(0.42f);
                name->setLimitLabelWidth(200.f, 0.42f, 0.15f);
                name->setAnchorPoint({ 0.f, 0.5f });
                name->setPosition({ 12.f, 13.f });
                row->addChild(name);

                int amount = static_cast<int>(s[notes ? "notes" : "votes"].asInt().unwrapOr(0));
                auto count = geode::Label::create(fmt::format("{} {}", amount, notes ? (amount == 1 ? "note" : "notes") : (amount == 1 ? "vote" : "votes")).c_str(), "goldFont.fnt");
                count->setScale(0.42f);
                count->setAnchorPoint({ 1.f, 0.5f });
                count->setPosition({ kListSize.width - 12.f, 13.f });
                row->addChild(count);

                self->m_list->addCell(row);
            }
            self->m_list->updateLayout(false);
            self->m_list->scrollToTop();
        }
    );
}

}
