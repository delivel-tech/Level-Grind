#include "../../managers/Auth.hpp"
#include "ReportPopups.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/GrindClansManager.hpp"
#include "../../managers/PetManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/Notification.hpp>
#include <UIBuilder.hpp>
#include <ctime>
#include <fmt/format.h>
#include "GrindProfilePopup.hpp"

namespace levelgrind {

using M = GrindClansManager;

namespace {

constexpr const char* kCategories[] = { "Name", "Text", "Cheating", "Other" };

CCLabelBMFont* lbl(CCNode* parent, std::string const& text, CCPoint pos, float scale, const char* font = "bigFont.fnt", CCPoint anchor = { 0.f, 0.5f }) {
    auto l = CCLabelBMFont::create(text.c_str(), font);
    l->setScale(scale);
    l->setAnchorPoint(anchor);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

ButtonSprite* segment(const char* text, bool on, int width = 60) {
    auto spr = ButtonSprite::create(text, width, true, "bigFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 24.f, 0.5f);
    spr->setScale(0.8f);
    return spr;
}

CCMenu* overlayMenu(CCNode* parent) {
    auto menu = CCMenu::create();
    menu->setContentSize(parent->getContentSize());
    menu->ignoreAnchorPointForPosition(false);
    menu->setAnchorPoint({ 0.f, 0.f });
    menu->setPosition({ 0.f, 0.f });
    parent->addChild(menu, 5);
    return menu;
}

ccColor3B rankColor(int rank) {
    switch (rank) {
        case 1: return { 255, 210, 60 };
        case 2: return { 210, 220, 235 };
        case 3: return { 230, 150, 90 };
    }
    return { 255, 255, 255 };
}

void setArrow(CCMenuItemSpriteExtra* btn, bool on) {
    if (!btn) return;
    btn->setEnabled(on);
    if (auto spr = typeinfo_cast<CCSprite*>(btn->getNormalImage())) {
        spr->setColor(on ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 90, 90, 90 });
        spr->setOpacity(on ? 255 : 150);
    }
}

CCMenuItemSpriteExtra* arrow(CCMenu* menu, bool next, CCPoint pos, geode::Function<void()> cb) {
    auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    spr->setScale(0.5f);
    spr->setFlipX(next);
    auto btn = CCMenuItemExt::createSpriteExtra(spr, [cb = std::move(cb)](CCObject*) mutable { cb(); });
    btn->setPosition(pos);
    menu->addChild(btn);
    return btn;
}

}

ReportPopup* ReportPopup::create(ReportTarget type, int targetId, std::string const& targetName) {
    auto ret = new ReportPopup;
    if (ret->init(type, targetId, targetName)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ReportPopup::init(ReportTarget type, int targetId, std::string const& targetName) {
    if (!BasePopup::init(340.f, 230.f)) return false;
    m_type = type;
    m_targetId = targetId;
    this->setTitle(type == ReportTarget::Clan ? "Report Clan" : type == ReportTarget::Pet ? "Report Pet" : "Report Message");

    auto name = lbl(m_mainLayer, targetName, { centerX(), top() - 44.f }, 0.5f, "bigFont.fnt", { 0.5f, 0.5f });
    name->limitLabelWidth(300.f, 0.5f, 0.2f);
    name->setColor({ 255, 220, 120 });

    lbl(m_mainLayer, "What's wrong?", { centerX(), top() - 68.f }, 0.45f, "goldFont.fnt", { 0.5f, 0.5f });

    m_catMenu = CCMenu::create();
    m_catMenu->setLayout(RowLayout::create()->setGap(4.f)->setAutoScale(false));
    m_catMenu->setContentSize({ 320.f, 26.f });
    m_catMenu->setPosition({ centerX(), top() - 90.f });
    m_mainLayer->addChild(m_catMenu);
    this->refreshCategories();

    m_reason = TextInput::create(300.f, "Tell staff more (optional)", "chatFont.fnt");
    m_reason->setMaxCharCount(150);
    m_reason->setCommonFilter(CommonFilter::Any);
    m_reason->setPosition({ centerX(), top() - 126.f });
    m_mainLayer->addChild(m_reason);

    lbl(m_mainLayer, "Please only report real problems", { centerX(), 52.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f })
        ->setOpacity(190);

    auto send = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Send", "goldFont.fnt", "GJ_button_06.png", 0.8f),
        [this](CCObject*) { this->onSend(); });
    send->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 22.f });
    m_buttonMenu->addChild(send);
    return true;
}

void ReportPopup::refreshCategories() {
    m_catMenu->removeAllChildren();
    for (int i = 0; i < 4; i++) {
        const char* text = kCategories[i];
        if (i == 1) text = m_type == ReportTarget::Clan ? "Text/Chat" : "Text";
        m_catMenu->addChild(CCMenuItemExt::createSpriteExtra(segment(text, i == m_category, 66), [this, i](CCObject*) {
            m_category = i;
            this->refreshCategories();
        }));
    }
    m_catMenu->updateLayout();
}

void ReportPopup::onSend() {
    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["targetType"] = static_cast<int>(m_type);
    body["targetId"] = m_targetId;
    body["category"] = m_category;
    body["reason"] = m_reason->getString();

    Ref<ReportPopup> self = this;
    M::getInstance().runAction("report_create", body, "Sending report...", "Report sent! Thank you.", [self](matjson::Value const&) {
        if (self) self->onClose(nullptr);
    });
}

PetLeaderboardPopup* PetLeaderboardPopup::create() {
    auto ret = new PetLeaderboardPopup;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PetLeaderboardPopup::init() {
    if (!BasePopup::init(390.f, 280.f)) return false;
    this->setTitle("Pet Leaderboard");

    m_tabs = CCMenu::create();
    m_tabs->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));
    m_tabs->setContentSize({ 200.f, 26.f });
    m_tabs->setPosition({ centerX(), top() - 44.f });
    m_mainLayer->addChild(m_tabs);

    m_list = cue::ListNode::create({ 316.f, 180.f });
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(30.f);
    m_list->setPosition({ centerX(), centerY() - 14.f });
    m_mainLayer->addChild(m_list);

    Ref<PetLeaderboardPopup> self = this;
    m_prev = arrow(m_buttonMenu, false, { 18.f, centerY() - 14.f }, [self] {
        if (self && self->m_page > 0) { self->m_page--; self->load(); }
    });
    m_next = arrow(m_buttonMenu, true, { right() - 18.f, centerY() - 14.f }, [self] {
        if (self && self->m_hasMore) { self->m_page++; self->load(); }
    });

    m_footer = lbl(m_mainLayer, "", { centerX(), 16.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f });
    this->load();
    return true;
}

void PetLeaderboardPopup::load() {
    m_tabs->removeAllChildren();
    for (auto [sort, text] : { std::pair<const char*, const char*>{ "level", "By Level" }, { "rarity", "By Rarity" } }) {
        std::string s = sort;
        m_tabs->addChild(CCMenuItemExt::createSpriteExtra(segment(text, m_sort == s, 80), [this, s](CCObject*) {
            if (m_sort == s) return;
            m_sort = s;
            m_page = 0;
            this->load();
        }));
    }
    m_tabs->updateLayout();

    m_list->clear();
    m_list->updateLayout(false);
    setArrow(m_prev, false);
    setArrow(m_next, false);
    if (m_status) m_status->removeFromParent();
    auto spinner = LoadingSpinner::create(30.f);
    spinner->setPosition(m_list->getPosition());
    m_mainLayer->addChild(spinner, 10);
    m_status = spinner;
    m_footer->setString("");

    int me = GJAccountManager::get()->m_accountID;
    Ref<PetLeaderboardPopup> self = this;
    m_task.spawn(
        web::WebRequest().get(fmt::format("{}/pet_leaderboard?sort={}&page={}&accountId={}", APIClient::getInstance().baseUrl, m_sort, m_page, me)),
        [self, me](web::WebResponse res) {
            if (!self) return;
            if (self->m_status) {
                self->m_status->removeFromParent();
                self->m_status = nullptr;
            }
            auto json = res.json();
            if (!res.ok() || !json) {
                self->m_footer->setString(fmt::format("Failed to load (code {})", res.code()).c_str());
                return;
            }
            auto data = json.unwrap();
            self->m_hasMore = data["hasMore"].asBool().unwrapOr(false);
            auto pets = data["pets"].asArray();

            float w = 316.f;
            if (pets) {
                for (auto const& p : pets.unwrap()) {
                    int rank = static_cast<int>(p["rank"].asInt().unwrapOr(0));
                    int accountId = static_cast<int>(p["accountId"].asInt().unwrapOr(0));
                    int level = static_cast<int>(p["petLevel"].asInt().unwrapOr(1));
                    int rarity = static_cast<int>(p["petRarity"].asInt().unwrapOr(1));
                    std::string petName = p["petName"].asString().unwrapOrDefault();
                    std::string owner = p["username"].asString().unwrapOrDefault();

                    auto row = CCNode::create();
                    row->setContentSize({ w, 30.f });
                    if (accountId == me) row->addChild(CCLayerColor::create({ 120, 255, 120, 40 }, w, 30.f), -1);
                    auto menu = overlayMenu(row);

                    lbl(row, fmt::format("#{}", rank), { 20.f, 15.f }, 0.45f, "goldFont.fnt", { 0.5f, 0.5f })->setColor(rankColor(rank));

                    auto name = lbl(row, petName.empty() ? "Pet" : petName, { 42.f, 19.f }, 0.4f);
                    name->limitLabelWidth(150.f, 0.4f, 0.15f);
                    auto ownerLbl = lbl(row, fmt::format("by {}", owner), { 42.f, 7.f }, 0.4f, "chatFont.fnt");
                    ownerLbl->setOpacity(180);

                    lbl(row, fmt::format("Lv {}", level), { 236.f, 19.f }, 0.45f, "goldFont.fnt", { 0.5f, 0.5f });
                    lbl(row, PetManager::getInstance().getRarityFromInt(rarity), { 236.f, 7.f }, 0.38f, "chatFont.fnt", { 0.5f, 0.5f })
                        ->setOpacity(200);

                    auto profile = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png");
                    if (profile) {
                        profile->setScale(0.45f);
                        auto pBtn = CCMenuItemExt::createSpriteExtra(profile, [accountId](CCObject*) {
                            GrindProfilePopup::create(accountId)->show();
                        });
                        pBtn->setPosition({ w - 48.f, 15.f });
                        menu->addChild(pBtn);
                    }
                    if (accountId != me) {
                        auto flag = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
                        flag->setScale(0.45f);
                        std::string target = fmt::format("{} ({})", petName, owner);
                        auto fBtn = CCMenuItemExt::createSpriteExtra(flag, [accountId, target](CCObject*) {
                            ReportPopup::create(ReportTarget::Pet, accountId, target)->show();
                        });
                        fBtn->setPosition({ w - 18.f, 15.f });
                        menu->addChild(fBtn);
                    }
                    self->m_list->addCell(row);
                }
            }
            self->m_list->updateLayout(false);
            self->m_list->scrollToTop();

            setArrow(self->m_prev, self->m_page > 0);
            setArrow(self->m_next, self->m_hasMore);

            int myRank = static_cast<int>(data["myRank"].asInt().unwrapOr(0));
            self->m_footer->setString(myRank > 0
                ? fmt::format("Page {}  -  your pet is #{}", self->m_page + 1, myRank).c_str()
                : fmt::format("Page {}", self->m_page + 1).c_str());
        }
    );
}

namespace {

const char* resolutionText(std::string const& r) {
    if (r == "dismiss") return "Dismissed";
    if (r == "resolve") return "Handled";
    if (r == "reset_name") return "Name/text reset";
    if (r == "ban_pet") return "Pet banned";
    if (r == "remove_clan") return "Clan removed";
    if (r == "delete_message") return "Message deleted";
    return "Closed";
}

class ReportActionsPopup : public BasePopup {
public:
    static ReportActionsPopup* create(matjson::Value const& report, int modType, Ref<StaffReportsPopup> owner) {
        auto ret = new ReportActionsPopup;
        if (ret->init(report, modType, owner)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(matjson::Value const& r, int modType, Ref<StaffReportsPopup> owner) {
        int type = static_cast<int>(r["targetType"].asInt().unwrapOr(0));
        int buttons = 3 + (type != 0 ? 1 : 0) + (type != 2 && modType >= 2 ? 1 : 0);
        float const rowH = 24.f;
        float const listH = buttons * rowH;
        if (!BasePopup::init(240.f, listH + 74.f, "GJ_square01.png")) return false;
        long long id = r["id"].asInt().unwrapOr(0);
        int targetId = static_cast<int>(r["targetId"].asInt().unwrapOr(0));
        std::string name = r["targetName"].asString().unwrapOrDefault();
        this->setTitle(type == 0 ? "Clan Report" : type == 1 ? "Pet Report" : "Message Report");
        if (type == 2) {
            targetId = static_cast<int>(r["targetOwner"].asInt().unwrapOr(0));
            name = fmt::format("{}: {}", name, r["targetText"].asString().unwrapOrDefault());
        }

        auto title = lbl(m_mainLayer, name, { centerX(), top() - 40.f }, 0.4f, "bigFont.fnt", { 0.5f, 0.5f });
        title->limitLabelWidth(210.f, 0.4f, 0.15f);
        title->setColor({ 255, 220, 120 });

        auto menu = CCMenu::create();
        menu->setLayout(ColumnLayout::create()->setGap(5.f)->setAxisReverse(true)->setAutoScale(false));
        menu->setContentSize({ 200.f, listH });
        menu->setPosition({ centerX(), (top() - 52.f) / 2.f + 4.f });
        m_mainLayer->addChild(menu);

        Ref<ReportActionsPopup> self = this;
        auto act = [self, id, owner](std::string action, std::string note, std::string done) {
            matjson::Value body;
            body["accountId"] = GJAccountManager::get()->m_accountID;
            body["reportId"] = static_cast<int64_t>(id);
            body["action"] = action;
            body["note"] = note;
            M::getInstance().runAction("report_resolve", body, "Working...", done, [self, owner](matjson::Value const&) {
                if (self) self->onClose(nullptr);
                if (owner) owner->load();
            });
        };
        auto add = [&](const char* text, const char* bg, geode::Function<void()> cb) {
            auto spr = ButtonSprite::create(text, 170, true, "goldFont.fnt", bg, 26.f, 0.65f);
            spr->setScale(0.7f);
            menu->addChild(CCMenuItemExt::createSpriteExtra(spr, [cb = std::move(cb)](CCObject*) mutable { cb(); }));
        };

        if (type != 0 && targetId > 0) {
            add("View Profile", "GJ_button_04.png", [targetId] { GrindProfilePopup::create(targetId)->show(); });
        }
        add("Dismiss (nothing wrong)", "GJ_button_04.png", [act] { act("dismiss", "", "Report dismissed."); });
        add("Mark as handled", "GJ_button_01.png", [act] { act("resolve", "", "Marked as handled."); });
        if (type == 2) {
            add("Delete message", "GJ_button_06.png", [act] { act("delete_message", "", "Message deleted."); });
        } else {
            add(type == 0 ? "Clear description" : "Reset pet name", "GJ_button_02.png", [act] {
                act("reset_name", "", "Done! Reset.");
            });
        }
        if (modType >= 2 && type != 2) {
            std::string reason = r["reason"].asString().unwrapOrDefault();
            if (type == 1) {
                add("Ban pet", "GJ_button_06.png", [act, name, reason] {
                    createQuickPopup("Ban pet?", fmt::format("Ban <cr>{}</c>? The pet stops growing and can't be renamed or upgraded.", name),
                        "Cancel", "Ban", [act, reason](auto, bool yes) {
                            if (yes) act("ban_pet", reason.empty() ? "reported" : reason, "Pet banned.");
                        });
                });
            } else {
                add("Remove clan", "GJ_button_06.png", [act, name] {
                    createQuickPopup("Remove clan?", fmt::format("Delete <cr>{}</c> for every member? This can't be undone.", name),
                        "Cancel", "Remove", [act](auto, bool yes) {
                            if (yes) act("remove_clan", "", "Clan removed.");
                        });
                });
            }
        }
        menu->updateLayout();
        return true;
    }
};

}

StaffReportsPopup* StaffReportsPopup::create() {
    auto ret = new StaffReportsPopup;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StaffReportsPopup::init() {
    if (!BasePopup::init(420.f, 290.f, "GJ_square01.png")) return false;
    this->setTitle("Reports");

    m_tabs = CCMenu::create();
    m_tabs->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));
    m_tabs->setContentSize({ 220.f, 26.f });
    m_tabs->setPosition({ centerX(), top() - 44.f });
    m_mainLayer->addChild(m_tabs);

    m_list = cue::ListNode::create({ 346.f, 190.f });
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(42.f);
    m_list->setPosition({ centerX(), centerY() - 16.f });
    m_mainLayer->addChild(m_list);

    Ref<StaffReportsPopup> self = this;
    m_prev = arrow(m_buttonMenu, false, { 16.f, centerY() - 16.f }, [self] {
        if (self && self->m_page > 0) { self->m_page--; self->load(); }
    });
    m_next = arrow(m_buttonMenu, true, { right() - 16.f, centerY() - 16.f }, [self] {
        if (self && self->m_hasMore) { self->m_page++; self->load(); }
    });

    m_footer = lbl(m_mainLayer, "", { centerX(), 14.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f });
    this->load();
    return true;
}

void StaffReportsPopup::load() {
    m_tabs->removeAllChildren();
    for (auto [status, text] : { std::pair<int, const char*>{ 0, "Open" }, { 1, "Closed" } }) {
        m_tabs->addChild(CCMenuItemExt::createSpriteExtra(segment(text, m_status == status, 70), [this, status](CCObject*) {
            if (m_status == status) return;
            m_status = status;
            m_page = 0;
            this->load();
        }));
    }
    m_tabs->updateLayout();

    m_list->clear();
    m_list->updateLayout(false);
    setArrow(m_prev, false);
    setArrow(m_next, false);
    if (m_status_) m_status_->removeFromParent();
    auto spinner = LoadingSpinner::create(30.f);
    spinner->setPosition(m_list->getPosition());
    m_mainLayer->addChild(spinner, 10);
    m_status_ = spinner;
    m_footer->setString("");

    matjson::Value body;
    body["accountId"] = GJAccountManager::get()->m_accountID;
    body["status"] = m_status;
    body["page"] = m_page;

    Ref<StaffReportsPopup> self = this;
    m_task.spawn(M::post("reports_list", body), [self](web::WebResponse res) {
        if (!self) return;
        if (self->m_status_) {
            self->m_status_->removeFromParent();
            self->m_status_ = nullptr;
        }
        std::string err;
        if (!M::parseOk(res, err)) {
            self->m_footer->setString(err.c_str());
            return;
        }
        auto data = res.json().unwrap();
        self->m_hasMore = data["hasMore"].asBool().unwrapOr(false);
        self->m_modType = static_cast<int>(data["modType"].asInt().unwrapOr(1));
        long long now = data["now"].asInt().unwrapOr(static_cast<long long>(time(nullptr)));
        int openCount = static_cast<int>(data["openCount"].asInt().unwrapOr(0));

        float w = 346.f;
        if (auto reports = data["reports"].asArray()) {
            for (auto const& r : reports.unwrap()) {
                int type = static_cast<int>(r["targetType"].asInt().unwrapOr(0));
                int category = static_cast<int>(r["category"].asInt().unwrapOr(3));
                int onTarget = static_cast<int>(r["openOnTarget"].asInt().unwrapOr(0));

                auto row = CCNode::create();
                row->setContentSize({ w, 42.f });
                auto menu = overlayMenu(row);

                auto icon = CCSprite::createWithSpriteFrameName(
                    type == 0 ? "GJ_sFriendsIcon_001.png" : type == 1 ? "GJ_starsIcon_001.png" : "GJ_chatBtn_001.png");
                icon->setScale(type == 2 ? 0.3f : 0.6f);
                icon->setPosition({ 18.f, 21.f });
                row->addChild(icon);

                std::string targetName = r["targetName"].asString().unwrapOrDefault();
                if (type == 2) targetName = fmt::format("{}: {}", targetName, r["targetText"].asString().unwrapOrDefault());
                auto name = lbl(row, targetName, { 36.f, 32.f }, 0.38f);
                name->limitLabelWidth(220.f, 0.38f, 0.15f);
                lbl(row, fmt::format("{}  -  by {}  -  {} ago",
                        category >= 0 && category < 4 ? kCategories[category] : "Other",
                        r["reporterName"].asString().unwrapOrDefault(),
                        M::timeAgo(r["at"].asInt().unwrapOr(0), now)),
                    { 36.f, 20.f }, 0.38f, "chatFont.fnt")->setOpacity(200);
                std::string reason = r["reason"].asString().unwrapOrDefault();
                auto reasonLbl = lbl(row, reason.empty() ? "(no details)" : fmt::format("\"{}\"", reason), { 36.f, 8.f }, 0.38f, "chatFont.fnt");
                reasonLbl->limitLabelWidth(260.f, 0.38f, 0.15f);
                reasonLbl->setColor({ 200, 200, 255 });

                if (self->m_status == 0) {
                    if (onTarget > 1) {
                        lbl(row, fmt::format("x{}", onTarget), { w - 78.f, 21.f }, 0.45f, "goldFont.fnt", { 0.5f, 0.5f })
                            ->setColor({ 255, 110, 110 });
                    }
                    auto spr = ButtonSprite::create("Handle", 50, true, "goldFont.fnt", "GJ_button_01.png", 26.f, 0.6f);
                    spr->setScale(0.65f);
                    auto report = r;
                    auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, report](CCObject*) {
                        if (!self) return;
                        ReportActionsPopup::create(report, self->m_modType, self)->show();
                    });
                    btn->setPosition({ w - 32.f, 21.f });
                    menu->addChild(btn);
                } else {
                    lbl(row, resolutionText(r["resolution"].asString().unwrapOrDefault()), { w - 10.f, 26.f }, 0.4f, "goldFont.fnt", { 1.f, 0.5f });
                    lbl(row, fmt::format("by {}", r["resolvedBy"].asString().unwrapOrDefault()), { w - 10.f, 12.f }, 0.36f, "chatFont.fnt", { 1.f, 0.5f })
                        ->setOpacity(180);
                }
                self->m_list->addCell(row);
            }
        }
        self->m_list->updateLayout(false);
        self->m_list->scrollToTop();
        setArrow(self->m_prev, self->m_page > 0);
        setArrow(self->m_next, self->m_hasMore);

        if (self->m_list->size() == 0) {
            self->m_footer->setString(self->m_status == 0 ? "No open reports. Nice!" : "No closed reports yet");
        } else {
            self->m_footer->setString(fmt::format("Page {}  -  {} open", self->m_page + 1, openCount).c_str());
        }
    });
}

}
