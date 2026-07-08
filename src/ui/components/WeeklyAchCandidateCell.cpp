#include "WeeklyAchCandidateCell.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"

#include <Geode/Enums.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>

#include "../../managers/WeeklyAchievementsManager.hpp"
#include "../popups/AchTypeChooserPopup.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"

namespace levelgrind {

WeeklyAchCandidateCell* WeeklyAchCandidateCell::create(std::pair<bool, AchievementCellInfo> candidateInfo, AchievementCellType placement, GJUserScore* userScore) {
    auto ret = new WeeklyAchCandidateCell;
    if (ret && ret->init(candidateInfo, placement, userScore)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchCandidateCell::init(std::pair<bool, AchievementCellInfo> candidateInfo, AchievementCellType placement, GJUserScore* userScore) {
    if (!CCMenu::init()) return false;

    m_type = candidateInfo.second.type;

    m_userScore = userScore;

    this->setContentSize({190, 150});

    auto bg = Build(NineSlice::create("GJ_square05.png"))
        .contentSize(this->getContentSize())
        .id("bg-nineslice")
        .parent(this)
        .pos(this->getContentSize() / 2)
        .collect();

    if (!candidateInfo.first) {
        auto createNewBtn = Build(CCSprite::createWithSpriteFrameName("GJ_newBtn_001.png"))
            .intoMenuItem([this, candidateInfo, placement, userScore] {
                WeeklyAchievementsManager::getInstance().saveConfigData(
                    true,
                    placement,
                    AchievementCellInfo{
                        userScore->m_userName,
                        userScore->m_accountID,
                        userScore->m_playerCube,
                        userScore->m_color1,
                        userScore->m_color2,
                        userScore->m_color3,
                        "", 0, ""
                    }
                );

                m_type = 0;
                
                this->createUI(WeeklyAchievementsManager::getInstance().loadConfigDataFromSaved(placement).second, placement);
                if (auto selfBtn = getChildByIDRecursive("create-new-btn")) selfBtn->removeFromParent();
                if (auto plLabel = getChildByIDRecursive("placement-label")) plLabel->removeFromParent();
            })
            .id("create-new-btn")
            .parent(this)
            .pos({
                this->getContentSize() / 2
            })
            .collect();

        auto placementLabel = Build(CCLabelBMFont::create(
            fmt::format("Placement: {}", (static_cast<int>(placement)) + 1).c_str(),
            "bigFont.fnt"
        ))
            .id("placement-label")
            .parent(this)
            .scale(0.5f)
            .pos({
                this->getScaledContentWidth() / 2,
                20
            })
            .collect();

        return true;
    }

    createUI(candidateInfo.second, placement);

    return true;
}

void WeeklyAchCandidateCell::createUI(AchievementCellInfo candidateInfo, AchievementCellType placement) {
    auto columnMenu = Build(CCMenu::create())
        .layout(ColumnLayout::create()->setGap(10)->setAutoScale(false)->setAxisReverse(true))
        .id("column-menu")
        .scale(0.6f)
        .parent(this)
        .pos(this->getContentSize() / 2)
        .collect();

    auto usernameMenu = Build(CCMenu::create())
        .layout(RowLayout::create())
        .id("username-menu")
        .parent(columnMenu)
        .collect();

    auto cubeSprite = Build(cue::PlayerIcon::create(
        cue::Icons{
            IconType::Cube, candidateInfo.icon, candidateInfo.colorFirst, 
            candidateInfo.colorSecond, candidateInfo.glow
        }
    ))
        .scale(0.6f)
        .parent(usernameMenu)
        .id("cube-sprite")
        .collect();

    if (candidateInfo.glow == 0) cubeSprite->disableGlowOutline();

    auto usernameLabel = Build(CCLabelBMFont::create(
        candidateInfo.username.c_str(), "goldFont.fnt"
    ))
        .scale(0.5f)
        .parent(usernameMenu)
        .id("username-label")
        .collect();

    usernameMenu->updateLayout();

    auto placementLabel = Build(CCLabelBMFont::create(
        fmt::format("Placement: {}", (static_cast<int>(placement)) + 1).c_str(),
        "bigFont.fnt"
    ))
        .scale(0.5f)
        .parent(columnMenu)
        .id("placement-label")
        .collect();

    auto titleMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setAutoScale(false))
        .id("title-menu")
        .parent(columnMenu)
        .collect();

    auto titleLabel = Build(CCLabelBMFont::create(
        "Title: ", "bigFont.fnt"
    ))
        .scale(0.5f)
        .parent(titleMenu)
        .id("title-label")
        .collect();

    auto titleInput = Build(TextInput::create(80, "200K"))
        .with([candidateInfo](TextInput* input) {
            input->setMaxCharCount(255);
            if (candidateInfo.title != "") input->setString(candidateInfo.title);
        })
        .parent(titleMenu)
        .id("title-input")
        .collect();

    titleMenu->updateLayout();

    auto typeMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setAutoScale(false))
        .id("type-menu")
        .parent(columnMenu)
        .collect();

    auto typeLabel = Build(CCLabelBMFont::create(
        "Type: ", "bigFont.fnt"
    ))
        .scale(0.5f)
        .parent(typeMenu)
        .id("type-label")
        .collect();

    auto addTitleBtn = Build(CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png"))
        .intoMenuItem([candidateInfo, this] {
            AchTypeChooserPopup::create(this)->show();
        })
        .parent(typeMenu)
        .id("add-title-btn")
        .collect();

    typeMenu->updateLayout();

    auto infoMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setAutoScale(false))
        .id("info-menu")
        .parent(columnMenu)
        .collect();

    auto infoLabel = Build(CCLabelBMFont::create(
        "Info: ", "bigFont.fnt"
    ))
        .scale(0.5f)
        .parent(infoMenu)
        .id("info-label")
        .collect();

    auto infoInput = Build(TextInput::create(80, "Desc."))
        .with([candidateInfo](TextInput* input) {
            input->setMaxCharCount(255);
            if (candidateInfo.info != "") input->setString(candidateInfo.info);
        })
        .parent(infoMenu)
        .id("info-input")
        .collect();

    infoMenu->updateLayout();

    auto buttonsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setAutoScale(false))
        .id("buttons-menu")
        .parent(columnMenu)
        .collect();

    auto clearBtn = Build(ButtonSprite::create(
        "Clear", "bigFont.fnt", "GJ_button_06.png"
    ))
        .intoMenuItem([this, placement] {
            this->removeAllChildrenWithCleanup(true);
            WeeklyAchievementsManager::getInstance().wipeConfigData(placement);
            this->init({false, AchievementCellInfo{}}, placement, this->m_userScore);
            this->getParent()->updateLayout();
        })
        .parent(buttonsMenu)
        .id("clear-btn")
        .collect();

    auto saveBtn = Build(ButtonSprite::create(
        "Save", "bigFont.fnt", "GJ_button_01.png"
    ))
        .intoMenuItem([placement, candidateInfo, titleInput, infoInput, this] {
            WeeklyAchievementsManager::getInstance().saveConfigData(
                true, placement, AchievementCellInfo{
                    candidateInfo.username,
                    candidateInfo.accountID,
                    candidateInfo.icon,
                    candidateInfo.colorFirst,
                    candidateInfo.colorSecond,
                    candidateInfo.glow,
                    titleInput->getString(),
                    m_type,
                    infoInput->getString()
                }
            );
            Notification::create("Config data saved!", NotificationIcon::Success)->show();
        })
        .parent(buttonsMenu)
        .id("save-btn")
        .collect();

    buttonsMenu->updateLayout();
    columnMenu->updateLayout();

    return;
}

}