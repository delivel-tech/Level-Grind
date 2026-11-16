#include "Auth.hpp"
#include "BadgesManager.hpp"
#include "APIClient.hpp"
#include "GrindClansManager.hpp"

#include <fmt/format.h>

namespace levelgrind::badges {

namespace {

struct State {
    bool loaded = false;
    long long daily = 0, weekly = 0, monthly = 0;
    std::string weeklyAch;
    long long packs = 0;
    bool hasClan = false;
    long long clanMessage = 0;
    long long clanWeekEnd = 0;
    int clanTasks = 0;
    int clanRequests = 0;
};

State& state() {
    static State s;
    return s;
}

TaskHolder<web::WebResponse>& task() {
    static TaskHolder<web::WebResponse> t;
    return t;
}

const char* key(Kind kind) {
    switch (kind) {
        case Kind::Daily: return "seen-daily";
        case Kind::Weekly: return "seen-weekly";
        case Kind::Monthly: return "seen-monthly";
        case Kind::WeeklyAch: return "seen-weekly-ach";
        case Kind::Packs: return "seen-packs";
        case Kind::Clans: return "seen-clan-msg";
    }
    return "seen-unknown";
}

template <class T>
T seen(const char* k, T current) {
    auto mod = Mod::get();
    if (!mod->hasSavedValue(k)) {
        mod->setSavedValue<T>(k, current);
        return current;
    }
    return mod->getSavedValue<T>(k);
}

}

void refresh(geode::Function<void()> onDone) {
    auto body = GrindClansManager::authBody();
    auto done = std::make_shared<geode::Function<void()>>(std::move(onDone));
    task().spawn(auth::post(fmt::format("{}/badges", APIClient::getInstance().baseUrl), std::move(body), std::chrono::seconds{ 15 }), [done](web::WebResponse res) {
        auto json = res.json();
        if (res.ok() && json) {
            auto v = json.unwrap();
            auto& s = state();
            s.loaded = true;
            s.daily = v["events"]["daily"].asInt().unwrapOr(0);
            s.weekly = v["events"]["weekly"].asInt().unwrapOr(0);
            s.monthly = v["events"]["monthly"].asInt().unwrapOr(0);
            s.weeklyAch = v["weeklyAch"].asString().unwrapOrDefault();
            s.packs = v["packs"].asInt().unwrapOr(0);
            s.hasClan = v.contains("clan") && v["clan"].isObject();
            if (s.hasClan) {
                s.clanMessage = v["clan"]["lastMessageId"].asInt().unwrapOr(0);
                s.clanWeekEnd = v["clan"]["weekEndsAt"].asInt().unwrapOr(0);
                s.clanTasks = static_cast<int>(v["clan"]["tasksDone"].asInt().unwrapOr(0));
                s.clanRequests = static_cast<int>(v["clan"]["requests"].asInt().unwrapOr(0));
            }
        }
        if (*done) (*done)();
    });
}

bool isNew(Kind kind) {
    auto const& s = state();
    if (!s.loaded) return false;
    switch (kind) {
        case Kind::Daily: return s.daily > seen<int64_t>(key(kind), s.daily);
        case Kind::Weekly: return s.weekly > seen<int64_t>(key(kind), s.weekly);
        case Kind::Monthly: return s.monthly > seen<int64_t>(key(kind), s.monthly);
        case Kind::WeeklyAch: return !s.weeklyAch.empty() && s.weeklyAch != seen<std::string>(key(kind), s.weeklyAch);
        case Kind::Packs: return s.packs > seen<int64_t>(key(kind), s.packs);
        case Kind::Clans: {
            if (!s.hasClan) return false;
            if (s.clanRequests > 0) return true;
            if (s.clanMessage > seen<int64_t>("seen-clan-msg", s.clanMessage)) return true;
            if (s.clanWeekEnd != seen<int64_t>("seen-clan-week-end", s.clanWeekEnd)) return true;
            return s.clanTasks > seen<int>("seen-clan-tasks", s.clanTasks);
        }
    }
    return false;
}

void markSeen(Kind kind) {
    auto const& s = state();
    if (!s.loaded) return;
    auto mod = Mod::get();
    switch (kind) {
        case Kind::Daily: mod->setSavedValue<int64_t>(key(kind), s.daily); break;
        case Kind::Weekly: mod->setSavedValue<int64_t>(key(kind), s.weekly); break;
        case Kind::Monthly: mod->setSavedValue<int64_t>(key(kind), s.monthly); break;
        case Kind::WeeklyAch: mod->setSavedValue<std::string>(key(kind), s.weeklyAch); break;
        case Kind::Packs: mod->setSavedValue<int64_t>(key(kind), s.packs); break;
        case Kind::Clans: break;
    }
}

void markClanChatSeen(long long lastMessageId) {
    auto mod = Mod::get();
    if (lastMessageId > mod->getSavedValue<int64_t>("seen-clan-msg", 0)) {
        mod->setSavedValue<int64_t>("seen-clan-msg", lastMessageId);
    }
}

void markClanTasksSeen(long long weekEndsAt, int tasksDone) {
    auto mod = Mod::get();
    mod->setSavedValue<int64_t>("seen-clan-week-end", weekEndsAt);
    mod->setSavedValue<int>("seen-clan-tasks", tasksDone);
}

void setMark(CCNode* button, bool on) {
    if (!button) return;
    auto existing = button->getChildByID("new-mark"_spr);
    if (!on) {
        if (existing) existing->removeFromParent();
        return;
    }
    if (existing) return;
    auto mark = CCSprite::createWithSpriteFrameName("exMark_001.png");
    if (!mark) return;
    mark->setID("new-mark"_spr);
    mark->setScale(0.6f);
    auto size = button->getContentSize();
    mark->setPosition({ 9.f, size.height - 11.f });
    mark->setRotation(-8.f);
    button->addChild(mark, 10);
}

}
