#include "Auth.hpp"
#include "NotifyManager.hpp"
#include "APIClient.hpp"
#include "GrindClansManager.hpp"
#include "AchievementsManager.hpp"
#include "../ui/layers/ClansLayer.hpp"

#include <Geode/binding/AchievementNotifier.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/OverlayManager.hpp>
#include <Geode/utils/web.hpp>
#include <deque>
#include <fmt/format.h>

namespace levelgrind::notify {

std::vector<TypeInfo> const& types() {
    static std::vector<TypeInfo> list = {
        { Type::ClanMessage, "clan-message", "Clan messages", "New messages in your clan chat (not while the chat is open)." },
        { Type::ClanTask, "clan-task", "Clan tasks", "A weekly clan task was completed." },
        { Type::ClanRequest, "clan-request", "Join requests", "Someone asked to join your clan (leader and officers)." },
        { Type::Events, "events", "New events", "A new Daily, Weekly or Monthly level is out." },
        { Type::WeeklyAch, "weekly-ach", "Weekly achievements", "New weekly achievements were picked." },
        { Type::NewLevels, "new-levels", "New levels", "Staff added levels to the Level Grind database." },
        { Type::NewPacks, "new-packs", "New Grind Packs", "A new Grind Pack is out." },
    };
    return list;
}

static const char* keyOf(Type type) {
    for (auto const& t : types()) {
        if (t.type == type) return t.key;
    }
    return "unknown";
}

bool enabled(Type type) {
    return Mod::get()->getSavedValue<bool>(fmt::format("notify-{}", keyOf(type)), true);
}

void setEnabled(Type type, bool on) {
    Mod::get()->setSavedValue<bool>(fmt::format("notify-{}", keyOf(type)), on);
}

namespace {

struct Pending {
    std::string title, text;
    std::string icon;
    bool modSprite = false;
    bool achievement = false;
    int gp = 0;
};

std::deque<Pending>& queue() {
    static std::deque<Pending> q;
    return q;
}

int& activeCards() {
    static int n = 0;
    return n;
}

CCSprite* iconFor(std::string const& name, bool modSprite) {
    if (name.empty()) return nullptr;
    return modSprite ? CCSprite::create(name.c_str()) : CCSprite::createWithSpriteFrameName(name.c_str());
}

CCNode* makeCard(Pending const& p, CCSize size) {
    auto card = CCNode::create();
    card->setContentSize(size);

    auto bg = NineSlice::create(p.achievement ? "GJ_square02.png" : "square02_small.png");
    bg->setContentSize(size);
    bg->setAnchorPoint({ 0.f, 0.f });
    if (!p.achievement) {
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(120);
    }
    card->addChild(bg);

    float textX = 10.f;
    if (auto icon = iconFor(p.icon, p.modSprite)) {
        float s = (size.height - 16.f) / std::max(icon->getContentWidth(), icon->getContentHeight());
        icon->setScale(s);
        icon->setPosition({ 8.f + icon->getScaledContentWidth() / 2.f, size.height / 2.f });
        card->addChild(icon);
        textX = 16.f + icon->getScaledContentWidth();
    }
    auto title = CCLabelBMFont::create(p.title.c_str(), "goldFont.fnt");
    title->setAnchorPoint({ 0.f, 0.5f });
    title->setPosition({ textX, size.height - 13.f });
    title->limitLabelWidth(size.width - textX - 8.f, p.achievement ? 0.55f : 0.45f, 0.2f);
    card->addChild(title);
    auto text = CCLabelBMFont::create(p.text.c_str(), "chatFont.fnt");
    text->setAnchorPoint({ 0.f, 0.5f });
    text->setPosition({ textX, 12.f });
    text->limitLabelWidth(size.width - textX - 8.f, 0.55f, 0.25f);
    card->addChild(text);
    if (p.achievement && p.gp > 0) {
        auto gp = CCLabelBMFont::create(fmt::format("+{} GP", p.gp).c_str(), "bigFont.fnt");
        gp->setScale(0.3f);
        gp->setColor({ 255, 220, 80 });
        gp->setAnchorPoint({ 1.f, 0.5f });
        gp->setPosition({ size.width - 10.f, size.height - 13.f });
        card->addChild(gp);
        title->limitLabelWidth(size.width - textX - 50.f, 0.55f, 0.2f);
    }
    return card;
}

void present(Pending const& p) {
    auto overlay = OverlayManager::get();
    auto win = CCDirector::sharedDirector()->getWinSize();

    if (p.achievement) {
        CCSize size{ 260.f, 46.f };
        auto card = makeCard(p, size);
        card->setAnchorPoint({ 0.5f, 0.f });
        card->setPosition({ win.width / 2.f, win.height + 4.f });
        overlay->addChild(card, 1001);
        card->runAction(CCSequence::create(
            CCEaseBackOut::create(CCMoveTo::create(0.4f, { win.width / 2.f, win.height - size.height - 6.f })),
            CCDelayTime::create(3.f),
            CCEaseIn::create(CCMoveTo::create(0.3f, { win.width / 2.f, win.height + 4.f }), 2.f),
            CCRemoveSelf::create(),
            nullptr
        ));
        return;
    }

    CCSize size{ 180.f, 40.f };
    int slot = activeCards()++;
    auto card = makeCard(p, size);
    card->setAnchorPoint({ 1.f, 0.f });
    float y = 8.f + slot * (size.height + 6.f);
    card->setPosition({ win.width + size.width + 4.f, y });
    overlay->addChild(card, 1000);
    card->runAction(CCSequence::create(
        CCEaseBackOut::create(CCMoveTo::create(0.4f, { win.width - 8.f, y })),
        CCDelayTime::create(3.5f),
        CCEaseIn::create(CCMoveTo::create(0.3f, { win.width + size.width + 4.f, y }), 2.f),
        CallFuncExt::create([] { activeCards() = std::max(0, activeCards() - 1); }),
        CCRemoveSelf::create(),
        nullptr
    ));
}

void flush() {
    if (PlayLayer::get()) return;
    while (!queue().empty()) {
        if (!queue().front().achievement && activeCards() >= 3) return;
        present(queue().front());
        queue().pop_front();
    }
}

}

void show(std::string const& title, std::string const& text, const char* iconFrame, bool modSprite) {
    queue().push_back({ title, text, iconFrame ? iconFrame : "", modSprite, false, 0 });
    flush();
}

void showAchievement(std::string const& title, std::string const& text, int gp) {
    if (Mod::get()->getSavedValue<bool>("disable-achievement-popups", false)) return;
    std::string desc = gp > 0 ? fmt::format("{} (+{} GP)", text, gp) : text;
    AchievementNotifier::sharedState()->notifyAchievement(title.c_str(), desc.c_str(), "", false);
}

namespace {

struct Snapshot {
    bool valid = false;
    long long daily = 0, weekly = 0, monthly = 0;
    std::string weeklyAch;
    long long packs = 0, levels = 0;
    bool hasClan = false;
    long long clanMessage = 0, weekEnd = 0;
    int tasks = 0, requests = 0;
};

bool clanChatOpen() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return false;
    for (auto child : CCArrayExt<CCNode*>(scene->getChildren())) {
        if (typeinfo_cast<ClansLayer*>(child)) return true;
    }
    return false;
}

class Poller : public CCObject {
public:
    static Poller* get() {
        static Poller* p = [] {
            auto p = new Poller();
            return p;
        }();
        return p;
    }

    void tick(float dt) {
        flush();
        m_wait -= dt;
        if (m_wait > 0.f || m_busy) return;
        m_wait = 60.f;
        this->poll();
        achievements::check();
    }

private:
    void poll() {
        m_busy = true;
        m_task.spawn(auth::post(fmt::format("{}/badges", APIClient::getInstance().baseUrl), GrindClansManager::authBody(), std::chrono::seconds{ 15 }), [this](web::WebResponse res) {
            m_busy = false;
            auto json = res.json();
            if (!res.ok() || !json) return;
            auto v = json.unwrap();
            Snapshot s;
            s.valid = true;
            s.daily = v["events"]["daily"].asInt().unwrapOr(0);
            s.weekly = v["events"]["weekly"].asInt().unwrapOr(0);
            s.monthly = v["events"]["monthly"].asInt().unwrapOr(0);
            s.weeklyAch = v["weeklyAch"].asString().unwrapOrDefault();
            s.packs = v["packs"].asInt().unwrapOr(0);
            s.levels = v["levels"].asInt().unwrapOr(0);
            s.hasClan = v.contains("clan") && v["clan"].isObject();
            if (s.hasClan) {
                s.clanMessage = v["clan"]["lastMessageId"].asInt().unwrapOr(0);
                s.weekEnd = v["clan"]["weekEndsAt"].asInt().unwrapOr(0);
                s.tasks = static_cast<int>(v["clan"]["tasksDone"].asInt().unwrapOr(0));
                s.requests = static_cast<int>(v["clan"]["requests"].asInt().unwrapOr(0));
            }
            this->compare(s);
            m_last = s;
        });
    }

    void compare(Snapshot const& s) {
        auto const& o = m_last;
        if (!o.valid) return;

        if (enabled(Type::Events)) {
            if (s.daily > o.daily) show("New Daily level!", "A new Grind Daily is out", "gj_dailyCrown_001.png");
            if (s.weekly > o.weekly) show("New Weekly level!", "A new Grind Weekly is out", "gj_dailyCrown_001.png");
            if (s.monthly > o.monthly) show("New Monthly level!", "A new Grind Monthly is out", "gj_dailyCrown_001.png");
        }
        if (enabled(Type::WeeklyAch) && !s.weeklyAch.empty() && s.weeklyAch != o.weeklyAch) {
            show("Weekly Achievements", "This week's grinders were picked", "rankIcon_1_001.png");
        }
        if (enabled(Type::NewPacks) && s.packs > o.packs) {
            show("New Grind Pack!", "Check out Grind Packs", "GJ_starsIcon_001.png");
        }
        if (enabled(Type::NewLevels) && o.levels > 0 && s.levels > o.levels) {
            long long n = s.levels - o.levels;
            show("New levels!", fmt::format("{} new level{} in Level Grind", n, n == 1 ? "" : "s"), "GJ_starsIcon_001.png");
        }
        if (s.hasClan && o.hasClan) {
            if (enabled(Type::ClanMessage) && s.clanMessage > o.clanMessage && !clanChatOpen()) {
                show("Clan chat", "New messages in your clan", "GJ_chatBtn_001.png");
            }
            if (enabled(Type::ClanTask) && s.weekEnd == o.weekEnd && s.tasks > o.tasks) {
                show("Clan task done!", "Your clan finished a weekly task", "GJ_completesIcon_001.png");
            }
            if (enabled(Type::ClanRequest) && s.requests > o.requests) {
                show("Join request", "Someone wants to join your clan", "GJ_sFriendsIcon_001.png");
            }
        }
    }

    Snapshot m_last;
    float m_wait = 15.f;
    bool m_busy = false;
    TaskHolder<web::WebResponse> m_task;
};

}

void start() {
    static bool started = false;
    if (started) return;
    started = true;
    CCDirector::sharedDirector()->getScheduler()->scheduleSelector(
        schedule_selector(Poller::tick), Poller::get(), 1.f, false
    );
}

}

$on_mod(Loaded) {
    levelgrind::notify::start();
}
