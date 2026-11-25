#include "WeeklyAchievementConfigurePopup.hpp"
#include <Geode/ui/Label.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include "../../managers/DataManager.hpp"
#include "../components/WeeklyAchCandidateCell.hpp"
#include "../../managers/WeeklyAchievementsManager.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Notification.hpp"

#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>

#include "../../managers/APIClient.hpp"
#include "Geode/utils/web.hpp"

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
    if (!BasePopup::init({ 500.f, 290.f })) return false;

    m_userScore = userScore;

    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::Admin) {
        this->setTitle("Grind Admin: Configure Weekly Achievements");
    } else if (pos == GrindPosition::Owner) {
        this->setTitle("Grind Coordinator: Configure Weekly Achievements");
    } else {
        this->setTitle("Unknown Role: Configure Weekly Achievements");
        this->onClose(nullptr);
    }

    Build(geode::Label::create("Changes are saved as you type. Press Set to publish all three.", "chatFont.fnt"))
        .scale(0.55f)
        .pos(fromTop(40.f))
        .opacity(170)
        .parent(m_mainLayer);

    struct Spot { AchievementCellType type; float dx; float dy; };
    Spot const spots[] = {
        { AchievementCellType::Third, -158.f, -4.f },
        { AchievementCellType::First, 0.f, 4.f },
        { AchievementCellType::Second, 158.f, 0.f },
    };

    float const cardsY = centerY() - 8.f;
    for (auto const& spot : spots) {
        auto cell = WeeklyAchCandidateCell::create(spot.type, userScore);
        cell->setPosition({ centerX() + spot.dx, cardsY + spot.dy });
        cell->setID(fmt::format("cell-{}", static_cast<int>(spot.type) + 1));
        m_mainLayer->addChild(cell);
        m_cells[spot.type] = cell;
    }

    Build(ButtonSprite::create("Set", "goldFont.fnt", "GJ_button_01.png", 0.9f))
        .scale(0.8f)
        .intoMenuItem([this] { this->onSet(); })
        .id("set-btn")
        .pos({ m_buttonMenu->getContentWidth() / 2.f, 20.f })
        .parent(m_buttonMenu);

    return true;
}

void WeeklyAchievementConfigurePopup::onSet() {
    if (m_setting) return;

    int empty = 0;
    for (auto cell : m_cells) {
        if (cell && !cell->isFilled()) empty++;
    }

    if (empty > 0) {
        Ref<WeeklyAchievementConfigurePopup> self = this;
        createQuickPopup(
            "Empty spots",
            fmt::format("<cr>{}</c> of 3 spots {} empty. Set anyway?", empty, empty == 1 ? "is" : "are"),
            "Cancel", "Set",
            [self](auto, bool confirmed) {
                if (confirmed && self) self->sendSet();
            }
        );
        return;
    }

    this->sendSet();
}

void WeeklyAchievementConfigurePopup::sendSet() {
    m_setting = true;

    auto uPopup = UploadActionPopup::create(nullptr, "Setting...");
    uPopup->show();

    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<WeeklyAchievementConfigurePopup> self = this;
    auto& mgr = WeeklyAchievementsManager::getInstance();

    m_listener.spawn(
        APIClient::getInstance().newWeeklyAch(
            mgr.loadConfigDataFromSaved(AchievementCellType::First).second,
            mgr.loadConfigDataFromSaved(AchievementCellType::Second).second,
            mgr.loadConfigDataFromSaved(AchievementCellType::Third).second
        ),
        [uPopupRef, self](web::WebResponse res) {
            if (self) self->m_setting = false;
            if (!uPopupRef) return;

            if (APIClient::getInstance().newWeeklyAchParse(res)) {
                uPopupRef->showSuccessMessage("Success! Weekly achievements set.");
            } else {
                uPopupRef->showFailMessage("Failed! Try again later.");
            }
        }
    );
}

}