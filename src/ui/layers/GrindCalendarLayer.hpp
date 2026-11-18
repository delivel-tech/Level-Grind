#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <unordered_map>
#include "../BaseLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GrindCalendarLayer : public BaseLayer {
public:
    static GrindCalendarLayer* create();

    enum class Metric { Stars = 0, Moons, Levels, Demons, GP };
    struct Day {
        int levels = 0, stars = 0, moons = 0, demons = 0, gp = 0, packs = 0;
    };

    static int goal(Metric metric, int period);
    static void setGoal(Metric metric, int period, int value);
    static const char* metricName(Metric metric);

    void rebuild();

private:
    bool init() override;
    void load();

    int value(long long day) const;
    int sumRange(long long from, long long to) const;
    void buildGrid();
    void buildSide();

    std::unordered_map<long long, Day> m_days;
    long long m_today = 0;
    long long m_selected = 0;
    int m_year = 1970, m_month = 1;
    Metric m_metric = Metric::Stars;
    bool m_loaded = false;

    CCNode* m_left = nullptr;
    CCNode* m_right = nullptr;
    CCNode* m_spinner = nullptr;
    TaskHolder<web::WebResponse> m_task;
    ~GrindCalendarLayer() { m_task.cancel(); }
};

}
