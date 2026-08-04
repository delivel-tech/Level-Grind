#include "WeeklyAchievementCell.hpp"
#include "Geode/cocos/actions/CCActionEase.h"
#include "Geode/cocos/actions/CCActionInstant.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/actions/CCActionTween.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include <Geode/Enums.hpp>
#include <cue/PlayerIcon.hpp>
#include "Geode/cocos/particle_nodes/CCParticleExamples.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/utils/ZStringView.hpp"

#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <UIBuilder.hpp>

namespace levelgrind {

WeeklyAchievementCell* WeeklyAchievementCell::create(AchievementCellType cellType, AchievementCellInfo info) {
    auto ret = new WeeklyAchievementCell;
    if (ret && ret->init(cellType, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WeeklyAchievementCell::init(AchievementCellType cellType, AchievementCellInfo info) {
    if (!CCMenu::init()) return false;

    this->setContentSize({ 117, 225 });

    this->m_cellType = cellType;
    this->m_cellInfo = info;

    buildInfo();

    return true;
}

void WeeklyAchievementCell::buildInfo() {
    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .scale(0.7f)
        .intoMenuItem([this] {
            MDPopup::create(
                "Achievement Info",
                this->m_cellInfo.info.c_str(),
                "OK"
            )->show();
        })
        .id("info-btn")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            5
        })
        .collect();


    auto statsMenu = Build(CCMenu::create())
        .scale(0.6f)
        .anchorPoint({0.5f, 0})
        .layout(RowLayout::create())
        .id("stats-menu")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            20
        })
        .collect();


    auto title = Build(CCLabelBMFont::create(
        m_cellInfo.title.c_str(),
        "bigFont.fnt"
    ))
        .id("ach-title")
        .parent(statsMenu)
        .collect();


    auto getTypeSprite = [](AchievementType type) {
        switch (type) {
        case AchievementType::Star:
            return CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        case AchievementType::Moon:
            return CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
        case AchievementType::Coin:
            return CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
        case AchievementType::Demon:
            return CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");
        }

        return (CCSprite*)nullptr;
    };


    Build(getTypeSprite(static_cast<AchievementType>(m_cellInfo.type)))
        .id("type-sprite")
        .parent(statsMenu)
        .collect();

    statsMenu->updateLayout();    

    statsMenu->setOpacity(0);
    infoBtn->setOpacity(0);

    statsMenu->runAction(
        CCEaseInOut::create(
            CCFadeTo::create(0.5f, 255),
            2.f
        )
    );

    infoBtn->runAction(
        CCSequence::create(
            CCEaseInOut::create(
                CCFadeTo::create(0.5f, 255),
                2.f
            ),
            CCCallFunc::create(
                this,
                callfunc_selector(WeeklyAchievementCell::buildPodium)
            ),
            nullptr
        )
    );
}

void WeeklyAchievementCell::buildPodium() {
    auto getHeight = [](AchievementCellType type) {
        switch (type) {
        case AchievementCellType::First:
            return 80.f;
        case AchievementCellType::Second:
            return 60.f;
        case AchievementCellType::Third:
            return 40.f;
        }

        return 40.f;
    };


    float height = getHeight(m_cellType);


    m_podium = Build(
        PodiumNineSlice::create(
            {50, height},
            {50, height},
            m_cellType
        )
    )
        .anchorPoint({0.5f, 0})
        .id("podium")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            45
        })
        .collect();

    m_podium->setContentSize({50, 0});

    auto getRank = [](AchievementCellType type) {
        switch (type) {
        case levelgrind::AchievementCellType::First: return "rankIcon_1_001.png";
        case levelgrind::AchievementCellType::Second: return "rankIcon_top50_001.png";
        case levelgrind::AchievementCellType::Third: return "rankIcon_top100_001.png";
        }
    };

    auto getRankHeight = [](AchievementCellType type) {
        switch (type) {
        case levelgrind::AchievementCellType::First: return 23;
        case levelgrind::AchievementCellType::Second: return 23;
        case levelgrind::AchievementCellType::Third: return 28;
        }
    };

    auto rankIcon = Build(CCSprite::createWithSpriteFrameName(getRank(m_cellType)))
        .id("rank-icon")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            getRankHeight(m_cellType) + height
        })
        .collect();

    rankIcon->setScale(0);

    rankIcon->runAction(
        CCEaseInOut::create(
            CCScaleTo::create(0.5f, 1), 2.f
        )
    );

    m_podium->runAction(
        CCEaseInOut::create(
            CCActionTween::create(
                0.5f,
                "height",
                0.f,
                height
            ),
            2.f
        )
    );

    m_podium->runAction(
        CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create(
                this,
                callfunc_selector(WeeklyAchievementCell::buildAfterPodium)
            ),
            nullptr
        )
    );
}

void WeeklyAchievementCell::buildAfterPodium() {
    auto usernameMenu = Build(CCMenu::create())
        .anchorPoint({0.5f, 0})
        .layout(RowLayout::create()->setAutoScale(false))
        .id("username-menu")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            50 + m_podium->getContentHeight()
        })
        .collect();

    auto usernameBtn = Build(CCLabelBMFont::create(
        m_cellInfo.username.c_str(),
        "goldFont.fnt"
    ))
        .scale(0.6f)
        .intoMenuItem([this] {
            ProfilePage::create(
                m_cellInfo.accountID,
                false
            )->show();
        })
        .id("username-btn")
        .parent(usernameMenu)
        .collect();

    auto cube = cue::PlayerIcon::create(
        cue::Icons{
            IconType::Cube,
            m_cellInfo.icon,
            m_cellInfo.colorFirst,
            m_cellInfo.colorSecond,
            m_cellInfo.glow
        }
    );

    if (m_cellInfo.glow == 0) cube->disableGlowOutline();

    #ifndef GEODE_IS_IOS
    if (m_cellType == AchievementCellType::First) {

    auto particles = Build(CCParticleGalaxy::create())
        .with([](CCParticleGalaxy* galaxy) {
            galaxy->setLife(0.8f);
        })
        .id("cube-particles")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            85 + m_podium->getContentHeight()
        })
        .scale(0.85f)
        .collect();

    }
    #endif

    auto cubeSprite = Build(cube)
        .id("cube-sprite")
        .parent(this)
        .pos({
            this->getContentWidth() / 2,
            85 + m_podium->getContentHeight()
        })
        .collect();

    usernameMenu->updateLayout();

    usernameMenu->setOpacity(0);
    cubeSprite->setScale(0);

    usernameMenu->runAction(
        CCEaseInOut::create(
            CCFadeTo::create(0.5f, 255),
            2.f
        )
    );

    cubeSprite->runAction(
        CCEaseInOut::create(
            CCScaleTo::create(0.5f, 1),
            2.f
        )
    );
}

}