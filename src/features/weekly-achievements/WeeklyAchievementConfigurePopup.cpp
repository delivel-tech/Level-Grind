#include "WeeklyAchievementConfigurePopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../../core/DataManager.hpp"
#include "WeeklyAchCandidateCell.hpp"
#include "WeeklyAchievementsManager.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"

#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>

#include "../../core/BackendManager.hpp"
#include "Geode/utils/async.hpp"

namespace levelgrind {

WeeklyAchievementConfigurePopup* WeeklyAchievementConfigurePopup::create(GJUserScore *userScore) {
    auto ret = new WeeklyAchievementConfigurePopup;
    if (ret && ret->init(userScore)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchievementConfigurePopup::init(GJUserScore* userScore) {
    if (!BasePopup::init({490, 190})) return false;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Configure Weekly Achievements");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Owner: Configure Weekly Achievements");
    } else {
        this->setTitle("Unknown Role: Configure Weekly Achievements");
        this->onClose(nullptr);
    }

    auto pl1CongigData = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::First);
    auto pl2ConfigData = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::Second);
    auto pl3ConfigData = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::Third);

    auto rowMenu = Build(CCMenu::create())
        .layout(RowLayout::create())
        .id("row-menu")
        .parent(m_buttonMenu)
        .scale(0.8f)
        .pos(centerX(), centerY() - 10)
        .collect();

    auto cell2 = Build(WeeklyAchCandidateCell::create(pl2ConfigData, AchievementCellType::Second, userScore))
        .id("cell-2")
        .parent(rowMenu)
        .collect();

    auto cell1 = Build(WeeklyAchCandidateCell::create(pl1CongigData, AchievementCellType::First, userScore))
        .id("cell-1")
        .parent(rowMenu)
        .collect();

    auto cell3 = Build(WeeklyAchCandidateCell::create(pl3ConfigData, AchievementCellType::Third, userScore))
        .id("cell-3")
        .parent(rowMenu)
        .collect();

    rowMenu->updateLayout();

    auto setBtn = Build(ButtonSprite::create(
        "Set", "bigFont.fnt", "GJ_button_01.png"
    ))
        .scale(0.8f)
        .intoMenuItem([this] {
            if (!m_triedSetting) {
                Notification::create("Make sure to save all players!", NotificationIcon::Info)->show();
                m_triedSetting = true;
                return;
            }

            async::spawn(this->onSetClicked());
        })
        .id("set-btn")
        .parent(m_buttonMenu)
        .pos({
            centerX(),
            -20
        })
        .collect();

    return true;
}

arc::Future<> WeeklyAchievementConfigurePopup::onSetClicked() {
    Ref<UploadActionPopup> uPopupRef;
    AchievementCellInfo first, second, third;
    co_await async::waitForMainThread([&] {
        auto uPopup = UploadActionPopup::create(nullptr, "Setting...");
        uPopup->show();
        uPopupRef = uPopup;
        first = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::First).second;
        second = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::Second).second;
        third = WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(AchievementCellType::Third).second;
    });

    auto parsed = co_await BackendManager::getInstance().newWeeklyAch(first, second, third);

    co_await async::waitForMainThread([&] {
        if (!uPopupRef) return;
        if (!parsed.ok) { uPopupRef->showFailMessage("Failed! Try again later."); return; }
        uPopupRef->showSuccessMessage("Success! Set.");
    });
    co_return;
}

}