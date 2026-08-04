#pragma once
#include <Geode/Geode.hpp>
#include "../features/weekly-achievements/WeeklyAchTypes.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PodiumNineSlice : public NineSlice {
protected:
    CCSize m_targetSize;
public:
    static PodiumNineSlice* create(CCSize cs, CCSize const& targetSize, AchievementCellType type) {
        auto ret = new PodiumNineSlice;
        if (ret && ret->init(cs, targetSize, type)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(CCSize cs, CCSize const& targetSize, AchievementCellType type) {
        m_targetSize = targetSize;
        if (!NineSlice::initWithFile("GJ_square02.png", {0, 0, 80, 80}, {8, 8, 8, 8})) return false;
        this->setContentSize(cs);
        return true;
    }

    virtual void updateTweenAction(float value, const char* key) override {
        if (std::string(key) == "height") {
            this->setContentSize({m_targetSize.width, value});
        }
    }
};

}