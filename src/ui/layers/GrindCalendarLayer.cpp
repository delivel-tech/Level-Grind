#include "GrindCalendarLayer.hpp"
#include "../BasePopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/Auth.hpp"
#include "../../managers/GrindClansManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <Geode/ui/TextInput.hpp>
#include <ctime>
#include <fmt/format.h>

namespace levelgrind {

using Metric = GrindCalendarLayer::Metric;

namespace {

constexpr Metric kMetrics[] = { Metric::Stars, Metric::Moons, Metric::Levels, Metric::Demons, Metric::GP };
constexpr const char* kPeriodNames[] = { "Daily", "Weekly", "Monthly" };
constexpr const char* kMonthNames[] = { "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December" };
constexpr const char* kWeekdays[] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };

ccColor3B metricColor(Metric m) {
    switch (m) {
        case Metric::Stars: return { 255, 210, 60 };
        case Metric::Moons: return { 120, 175, 255 };
        case Metric::Levels: return { 110, 230, 110 };
        case Metric::Demons: return { 255, 90, 90 };
        case Metric::GP: return { 255, 165, 60 };
    }
    return { 255, 255, 255 };
}

CCSprite* metricIcon(Metric m) {
    switch (m) {
        case Metric::Stars: return CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        case Metric::Moons: return CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
        case Metric::Levels: return CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        case Metric::Demons: return CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");
        case Metric::GP: return CCSprite::create("grind_point_icon.png"_spr);
    }
    return nullptr;
}

long long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    long long yoe = y - era * 400;
    long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

void civilFromDays(long long z, int& y, int& m, int& d) {
    z += 719468;
    long long era = (z >= 0 ? z : z - 146096) / 146097;
    long long doe = z - era * 146097;
    long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long yy = yoe + era * 400;
    long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    long long mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    y = static_cast<int>(yy + (m <= 2));
}

int daysInMonth(int y, int m) {
    return static_cast<int>(daysFromCivil(m == 12 ? y + 1 : y, m == 12 ? 1 : m + 1, 1) - daysFromCivil(y, m, 1));
}

int weekday(long long day) {
    return static_cast<int>(((day % 7) + 7 + 3) % 7);
}

int utcOffsetMinutes() {
    std::time_t now = std::time(nullptr);
    std::tm local{}, utc{};
#ifdef GEODE_IS_WINDOWS
    localtime_s(&local, &now);
    gmtime_s(&utc, &now);
#else
    localtime_r(&now, &local);
    gmtime_r(&now, &utc);
#endif
    utc.tm_isdst = local.tm_isdst;
    return static_cast<int>(std::difftime(std::mktime(&local), std::mktime(&utc)) / 60.0);
}

CCLabelBMFont* label(CCNode* parent, std::string const& text, CCPoint pos, float scale, const char* font = "bigFont.fnt", CCPoint anchor = { 0.5f, 0.5f }) {
    auto l = CCLabelBMFont::create(text.c_str(), font);
    l->setScale(scale);
    l->setAnchorPoint(anchor);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

NineSlice* panel(CCNode* parent, CCSize size, CCPoint pos, GLubyte opacity = 90) {
    auto bg = NineSlice::create("square02_small.png");
    bg->setContentSize(size);
    bg->setOpacity(opacity);
    bg->setPosition(pos);
    parent->addChild(bg);
    return bg;
}

std::string shortNumber(long long n) {
    if (n >= 1000000) return fmt::format("{:.1f}M", n / 1000000.0);
    if (n >= 10000) return fmt::format("{:.1f}K", n / 1000.0);
    return numToString(n);
}

class CalendarGoalsPopup : public BasePopup {
public:
    static CalendarGoalsPopup* create(Metric metric, GrindCalendarLayer* owner) {
        auto ret = new CalendarGoalsPopup;
        if (ret->init(metric, owner)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(Metric metric, GrindCalendarLayer* owner) {
        if (!BasePopup::init(260.f, 200.f)) return false;
        m_metric = metric;
        m_owner = owner;
        this->setTitle(fmt::format("{} Goals", GrindCalendarLayer::metricName(metric)).c_str());

        for (int i = 0; i < 3; i++) {
            float y = top() - 70.f - i * 32.f;
            label(m_mainLayer, kPeriodNames[i], { 30.f, y }, 0.4f, "goldFont.fnt", { 0.f, 0.5f });
            auto input = TextInput::create(110.f, "0", "bigFont.fnt");
            input->setCommonFilter(CommonFilter::Uint);
            input->setMaxCharCount(7);
            int g = GrindCalendarLayer::goal(metric, i);
            if (g > 0) input->setString(numToString(g));
            input->setScale(0.8f);
            input->setPosition({ right() - 80.f, y });
            m_mainLayer->addChild(input);
            m_inputs[i] = input;
        }

        Ref<CalendarGoalsPopup> self = this;
        auto save = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", 0.8f), [self](CCObject*) {
            if (!self) return;
            for (int i = 0; i < 3; i++) {
                int v = utils::numFromString<int>(self->m_inputs[i]->getString()).unwrapOr(0);
                GrindCalendarLayer::setGoal(self->m_metric, i, std::max(0, v));
            }
            if (auto owner = self->m_owner.lock()) owner->rebuild();
            self->onClose(nullptr);
        });
        save->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 22.f });
        m_buttonMenu->addChild(save);
        return true;
    }

    Metric m_metric = Metric::Stars;
    WeakRef<GrindCalendarLayer> m_owner;
    TextInput* m_inputs[3] = {};
};

}

const char* GrindCalendarLayer::metricName(Metric m) {
    switch (m) {
        case Metric::Stars: return "Stars";
        case Metric::Moons: return "Moons";
        case Metric::Levels: return "Levels";
        case Metric::Demons: return "Demons";
        case Metric::GP: return "Grind Points";
    }
    return "";
}

int GrindCalendarLayer::goal(Metric metric, int period) {
    return Mod::get()->getSavedValue<int>(fmt::format("calendar-goal-{}-{}", static_cast<int>(metric), period), 0);
}

void GrindCalendarLayer::setGoal(Metric metric, int period, int value) {
    Mod::get()->setSavedValue<int>(fmt::format("calendar-goal-{}-{}", static_cast<int>(metric), period), value);
}

GrindCalendarLayer* GrindCalendarLayer::create() {
    auto ret = new GrindCalendarLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GrindCalendarLayer::init() {
    if (!BaseLayer::init()) return false;
    m_metric = static_cast<Metric>(std::clamp(Mod::get()->getSavedValue<int>("calendar-metric", 0), 0, 4));

    auto win = m_winSize;
    label(this, "Grind Calendar", { win.width / 2.f, win.height - 18.f }, 0.75f, "goldFont.fnt");

    auto frame = NineSlice::create("GJ_square02.png");
    frame->setContentSize({ 546.f, 266.f });
    frame->setPosition({ win.width / 2.f, win.height / 2.f - 20.f });
    this->addChild(frame, 1);

    m_left = CCNode::create();
    m_left->setContentSize({ 296.f, 250.f });
    m_left->setAnchorPoint({ 0.5f, 0.5f });
    m_left->setPosition({ win.width / 2.f - 118.f, win.height / 2.f - 20.f });
    this->addChild(m_left, 2);

    m_right = CCNode::create();
    m_right->setContentSize({ 226.f, 250.f });
    m_right->setAnchorPoint({ 0.5f, 0.5f });
    m_right->setPosition({ win.width / 2.f + 150.f, win.height / 2.f - 20.f });
    this->addChild(m_right, 2);

    std::time_t now = std::time(nullptr);
    m_today = (static_cast<long long>(now) + utcOffsetMinutes() * 60LL) / 86400;
    m_selected = m_today;
    int d;
    civilFromDays(m_today, m_year, m_month, d);

    auto spinner = LoadingSpinner::create(50.f);
    spinner->setPosition(win / 2.f);
    this->addChild(spinner, 10);
    m_spinner = spinner;

    this->rebuild();
    this->load();
    return true;
}

void GrindCalendarLayer::load() {
    auto body = GrindClansManager::authBody();
    body["tzOffset"] = utcOffsetMinutes();
    body["days"] = 400;
    Ref<GrindCalendarLayer> self = this;
    m_task.spawn(auth::post(fmt::format("{}/calendar", APIClient::getInstance().baseUrl), std::move(body), std::chrono::seconds{ 20 }),
        [self](web::WebResponse res) {
            if (!self) return;
            if (self->m_spinner) {
                self->m_spinner->removeFromParent();
                self->m_spinner = nullptr;
            }
            auto json = res.json();
            if (!res.ok() || !json) {
                Notification::create("Couldn't load your calendar", NotificationIcon::Error)->show();
                return;
            }
            auto v = json.unwrap();
            self->m_today = v["today"].asInt().unwrapOr(self->m_today);
            self->m_days.clear();
            if (auto arr = v["days"].asArray()) {
                for (auto const& d : arr.unwrap()) {
                    Day day;
                    day.levels = static_cast<int>(d["levels"].asInt().unwrapOr(0));
                    day.stars = static_cast<int>(d["stars"].asInt().unwrapOr(0));
                    day.moons = static_cast<int>(d["moons"].asInt().unwrapOr(0));
                    day.demons = static_cast<int>(d["demons"].asInt().unwrapOr(0));
                    day.gp = static_cast<int>(d["gp"].asInt().unwrapOr(0));
                    day.packs = static_cast<int>(d["packs"].asInt().unwrapOr(0));
                    self->m_days[d["day"].asInt().unwrapOr(0)] = day;
                }
            }
            self->m_loaded = true;
            self->rebuild();
        });
}

int GrindCalendarLayer::value(long long day) const {
    auto it = m_days.find(day);
    if (it == m_days.end()) return 0;
    auto const& d = it->second;
    switch (m_metric) {
        case Metric::Stars: return d.stars;
        case Metric::Moons: return d.moons;
        case Metric::Levels: return d.levels;
        case Metric::Demons: return d.demons;
        case Metric::GP: return d.gp;
    }
    return 0;
}

int GrindCalendarLayer::sumRange(long long from, long long to) const {
    int sum = 0;
    for (long long d = from; d <= to; d++) sum += this->value(d);
    return sum;
}

void GrindCalendarLayer::rebuild() {
    this->buildGrid();
    this->buildSide();
}

void GrindCalendarLayer::buildGrid() {
    m_left->removeAllChildren();
    auto size = m_left->getContentSize();
    panel(m_left, size, size / 2.f);
    Ref<GrindCalendarLayer> self = this;

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setContentSize(size);
    m_left->addChild(menu, 5);

    float y = size.height - 14.f;
    label(m_left, fmt::format("{} {}", kMonthNames[m_month - 1], m_year), { size.width / 2.f, y }, 0.5f, "goldFont.fnt");
    for (int dir : { -1, 1 }) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        spr->setScale(0.35f);
        spr->setFlipX(dir > 0);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, dir](CCObject*) {
            if (!self) return;
            self->m_month += dir;
            if (self->m_month < 1) { self->m_month = 12; self->m_year--; }
            if (self->m_month > 12) { self->m_month = 1; self->m_year++; }
            Loader::get()->queueInMainThread([self] { if (self) self->rebuild(); });
        });
        btn->setPosition({ size.width / 2.f + dir * 85.f, y });
        menu->addChild(btn);
    }

    auto tabs = CCMenu::create();
    tabs->setLayout(RowLayout::create()->setGap(3.f)->setAutoScale(false));
    tabs->setContentSize({ size.width - 10.f, 20.f });
    tabs->setPosition({ size.width / 2.f, size.height - 34.f });
    m_left->addChild(tabs, 5);
    for (auto m : kMetrics) {
        bool on = m == m_metric;
        auto spr = ButtonSprite::create(m == Metric::GP ? "GP" : metricName(m), 40, true, "bigFont.fnt",
            on ? "GJ_button_02.png" : "GJ_button_04.png", 20.f, 0.45f);
        spr->setScale(0.75f);
        tabs->addChild(CCMenuItemExt::createSpriteExtra(spr, [self, m](CCObject*) {
            if (!self || self->m_metric == m) return;
            self->m_metric = m;
            Mod::get()->setSavedValue<int>("calendar-metric", static_cast<int>(m));
            Loader::get()->queueInMainThread([self] { if (self) self->rebuild(); });
        }));
    }
    tabs->updateLayout();

    float const cellW = 38.f, cellH = 26.f, gap = 2.f;
    float const gridW = 7 * cellW + 6 * gap;
    float const x0 = (size.width - gridW) / 2.f;
    float const headerY = size.height - 52.f;
    for (int i = 0; i < 7; i++) {
        auto l = label(m_left, kWeekdays[i], { x0 + i * (cellW + gap) + cellW / 2.f, headerY }, 0.28f, "bigFont.fnt");
        l->setOpacity(i >= 5 ? 220 : 170);
        if (i >= 5) l->setColor({ 255, 220, 140 });
    }

    long long first = daysFromCivil(m_year, m_month, 1);
    int count = daysInMonth(m_year, m_month);
    int lead = weekday(first);
    int best = 1;
    for (int i = 0; i < count; i++) best = std::max(best, this->value(first + i));
    auto color = metricColor(m_metric);
    float const topY = headerY - 10.f;

    for (int i = 0; i < count; i++) {
        long long day = first + i;
        int slot = lead + i;
        int col = slot % 7, row = slot / 7;
        float cx = x0 + col * (cellW + gap);
        float cy = topY - (row + 1) * cellH - row * gap;
        int v = this->value(day);
        bool future = day > m_today;

        auto cell = CCNode::create();
        cell->setContentSize({ cellW, cellH });

        auto bg = NineSlice::create(v > 0 ? "GJ_button_01.png" : "GJ_button_04.png");
        bg->setContentSize({ cellW * 1.6f, cellH * 1.6f });
        bg->setScale(1.f / 1.6f);
        bg->setPosition({ cellW / 2.f, cellH / 2.f });
        if (v > 0) {
            float k = 0.35f + 0.65f * std::sqrt(static_cast<float>(v) / best);
            auto mix = [k](GLubyte c) { return static_cast<GLubyte>(255 - (255 - c) * k); };
            bg->setColor({ mix(color.r), mix(color.g), mix(color.b) });
        }
        if (future) bg->setOpacity(110);
        cell->addChild(bg);

        auto num = label(cell, numToString(i + 1), { 4.f, cellH - 7.f }, 0.24f, "bigFont.fnt", { 0.f, 0.5f });
        num->setOpacity(future ? 90 : 230);
        if (v > 0) {
            auto val = label(cell, shortNumber(v), { cellW / 2.f + 3.f, 9.f }, 0.3f, "goldFont.fnt");
            val->limitLabelWidth(cellW - 10.f, 0.3f, 0.12f);
        }

        if (day == m_selected) num->setColor({ 255, 210, 60 });

        auto btn = CCMenuItemExt::createSpriteExtra(cell, [self, day](CCObject*) {
            if (!self) return;
            self->m_selected = day;
            Loader::get()->queueInMainThread([self] { if (self) self->rebuild(); });
        });
        btn->m_scaleMultiplier = 1.06f;
        btn->setPosition({ cx + cellW / 2.f, cy + cellH / 2.f });
        menu->addChild(btn);
    }

    int monthTotal = this->sumRange(first, first + count - 1);
    auto total = label(m_left, fmt::format("{} this month: {}", metricName(m_metric), monthTotal), { size.width / 2.f, 10.f }, 0.3f);
    total->setColor(color);
    total->limitLabelWidth(size.width - 20.f, 0.3f, 0.15f);
}

void GrindCalendarLayer::buildSide() {
    m_right->removeAllChildren();
    auto size = m_right->getContentSize();
    float const w = size.width;
    auto color = metricColor(m_metric);
    Ref<GrindCalendarLayer> self = this;

    float y = size.height;
    float const dayH = 62.f;
    panel(m_right, { w, dayH }, { w / 2.f, y - dayH / 2.f });
    {
        int yy, mm, dd;
        civilFromDays(m_selected, yy, mm, dd);
        static constexpr const char* kWeekdayNames[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
        std::string title = m_selected == m_today ? "Today" : m_selected == m_today - 1 ? "Yesterday"
            : fmt::format("{}, {} {}", kWeekdayNames[weekday(m_selected)], dd, kMonthNames[mm - 1]);
        label(m_right, title, { 8.f, y - 11.f }, 0.42f, "goldFont.fnt", { 0.f, 0.5f });

        Day d;
        if (auto it = m_days.find(m_selected); it != m_days.end()) d = it->second;
        struct { Metric m; int v; } stats[] = {
            { Metric::Stars, d.stars }, { Metric::Moons, d.moons }, { Metric::Levels, d.levels },
            { Metric::Demons, d.demons }, { Metric::GP, d.gp },
        };
        auto row = CCNode::create();
        row->setLayout(RowLayout::create()->setGap(6.f));
        row->setContentSize({ w - 14.f, 20.f });
        row->setAnchorPoint({ 0.5f, 0.5f });
        row->setPosition({ w / 2.f, y - 32.f });
        for (auto const& s : stats) {
            auto pair = CCNode::create();
            pair->setLayout(RowLayout::create()->setGap(2.f)->setAutoScale(false));
            pair->setContentSize({ 40.f, 16.f });
            if (auto icon = metricIcon(s.m)) {
                icon->setScale(12.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
                pair->addChild(icon);
            }
            auto l = CCLabelBMFont::create(shortNumber(s.v).c_str(), "bigFont.fnt");
            l->setScale(0.32f);
            if (s.v == 0) l->setOpacity(120);
            pair->addChild(l);
            pair->updateLayout();
            pair->setContentSize({ pair->getContentWidth(), 16.f });
            row->addChild(pair);
        }
        row->updateLayout();
        m_right->addChild(row);
        if (d.packs > 0) {
            label(m_right, fmt::format("+ {} Grind Pack{}", d.packs, d.packs == 1 ? "" : "s"), { w / 2.f, y - 50.f }, 0.31f, "bigFont.fnt")
                ->setColor({ 255, 150, 255 });
        } else if (m_selected <= m_today && d.levels == 0) {
            label(m_right, "Nothing beaten this day", { w / 2.f, y - 50.f }, 0.31f, "bigFont.fnt")->setOpacity(150);
        }
    }
    y -= dayH + 4.f;

    float const statH = 58.f;
    panel(m_right, { w, statH }, { w / 2.f, y - statH / 2.f });
    {
        long long weekStart = m_today - weekday(m_today);
        int ty, tm, td;
        civilFromDays(m_today, ty, tm, td);
        long long monthStart = daysFromCivil(ty, tm, 1);
        int week = this->sumRange(weekStart, m_today);
        int month = this->sumRange(monthStart, m_today);
        int last30 = this->sumRange(m_today - 29, m_today);

        int bestV = 0;
        long long bestDay = 0;
        int streak = 0, bestStreak = 0, run = 0;
        for (long long d = m_today - 399; d <= m_today; d++) {
            int v = this->value(d);
            if (v > bestV) { bestV = v; bestDay = d; }
            auto it = m_days.find(d);
            bool active = it != m_days.end() && it->second.levels > 0;
            run = active ? run + 1 : 0;
            bestStreak = std::max(bestStreak, run);
        }

        for (long long d = m_today; d > m_today - 400; d--) {
            auto it = m_days.find(d);
            bool active = it != m_days.end() && it->second.levels > 0;
            if (!active) {
                if (d == m_today) continue;
                break;
            }
            streak++;
        }

        auto cell = [&](float cx, float cy, std::string const& title, std::string const& val, ccColor3B c) {
            label(m_right, title, { cx, cy + 6.f }, 0.26f, "bigFont.fnt")->setOpacity(170);
            auto v = label(m_right, val, { cx, cy - 6.f }, 0.34f);
            v->setColor(c);
            v->limitLabelWidth(w / 3.f - 8.f, 0.34f, 0.15f);
        };
        float c1 = w / 6.f, c2 = w / 2.f, c3 = w * 5.f / 6.f;
        float r1 = y - 16.f, r2 = y - 42.f;
        cell(c1, r1, "This week", shortNumber(week), color);
        cell(c2, r1, "This month", shortNumber(month), color);
        cell(c3, r1, "Avg / day", fmt::format("{:.1f}", last30 / 30.0), color);
        std::string bestText = bestV > 0 ? shortNumber(bestV) : "-";
        if (bestV > 0) {
            int by, bm, bd;
            civilFromDays(bestDay, by, bm, bd);
            bestText += fmt::format(" ({}/{})", bd, bm);
        }
        cell(c1, r2, "Best day", bestText, color);
        cell(c2, r2, "Streak", fmt::format("{} day{}", streak, streak == 1 ? "" : "s"), { 255, 150, 80 });
        cell(c3, r2, "Best streak", numToString(bestStreak), { 255, 150, 80 });
    }
    y -= statH + 4.f;

    float const chartH = 52.f;
    panel(m_right, { w, chartH }, { w / 2.f, y - chartH / 2.f });
    {
        label(m_right, "Last 14 days", { 6.f, y - 7.f }, 0.25f, "bigFont.fnt", { 0.f, 0.5f })->setOpacity(170);
        int top = 1;
        for (int i = 0; i < 14; i++) top = std::max(top, this->value(m_today - 13 + i));
        float const barW = (w - 16.f) / 14.f;
        float const maxH = chartH - 22.f;
        for (int i = 0; i < 14; i++) {
            long long day = m_today - 13 + i;
            int v = this->value(day);
            float h = v > 0 ? std::max(2.f, maxH * v / top) : 1.f;
            auto bar = CCLayerColor::create({ color.r, color.g, color.b, static_cast<GLubyte>(day == m_selected ? 255 : 190) }, barW - 2.f, h);
            bar->setPosition({ 8.f + i * barW, y - chartH + 5.f });
            m_right->addChild(bar);
        }
        label(m_right, shortNumber(top), { w - 6.f, y - 7.f }, 0.25f, "bigFont.fnt", { 1.f, 0.5f })->setOpacity(170);
    }
    y -= chartH + 4.f;

    float const goalH = y;
    panel(m_right, { w, goalH }, { w / 2.f, goalH / 2.f });
    {
        label(m_right, fmt::format("{} Goals", metricName(m_metric)), { 6.f, y - 12.f }, 0.38f, "goldFont.fnt", { 0.f, 0.5f });
        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_right->addChild(menu, 5);
        auto editSpr = ButtonSprite::create("Edit", 40, true, "bigFont.fnt", "GJ_button_04.png", 20.f, 0.45f);
        editSpr->setScale(0.75f);
        auto edit = CCMenuItemExt::createSpriteExtra(editSpr, [self](CCObject*) {
            if (self) CalendarGoalsPopup::create(self->m_metric, self)->show();
        });
        edit->setPosition({ w - 8.f - editSpr->getScaledContentWidth() / 2.f, y - 12.f });
        menu->addChild(edit);

        long long weekStart = m_today - weekday(m_today);
        int ty, tm, td;
        civilFromDays(m_today, ty, tm, td);
        long long monthStart = daysFromCivil(ty, tm, 1);
        int progress[] = { this->value(m_today), this->sumRange(weekStart, m_today), this->sumRange(monthStart, m_today) };

        bool any = false;
        float rowY = y - 26.f;
        for (int i = 0; i < 3; i++) {
            int g = goal(m_metric, i);
            if (g <= 0) continue;
            any = true;
            label(m_right, kPeriodNames[i], { 8.f, rowY }, 0.26f, "bigFont.fnt", { 0.f, 0.5f });
            auto bar = ProgressBar::create(ProgressBar::Style::Solid);
            bool done = progress[i] >= g;
            bar->setFillColor(done ? ccColor3B{ 90, 230, 90 } : color);
            bar->setScale(0.26f);
            bar->setAnchorPoint({ 0.f, 0.5f });
            bar->setPosition({ 52.f, rowY });
            bar->updateProgress(std::clamp(100.f * progress[i] / g, 0.f, 100.f));
            m_right->addChild(bar);
            auto txt = label(m_right, fmt::format("{}/{}", shortNumber(progress[i]), shortNumber(g)), { w - 8.f, rowY }, 0.26f, "bigFont.fnt", { 1.f, 0.5f });
            if (done) txt->setColor({ 140, 255, 140 });
            rowY -= 13.f;
        }
        if (!any) {
            label(m_right, "No goals set", { w / 2.f, y - 30.f }, 0.28f, "bigFont.fnt")->setOpacity(160);
        }
    }

}

}
