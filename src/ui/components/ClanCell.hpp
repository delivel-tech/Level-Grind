#include <Geode/Geode.hpp>
#include "../../features/clans/ClanTypes.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/NineSlice.hpp"
#include "ccTypes.h"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

class ClanCell : public CCMenu {
public:
    static ClanCell* create(LGClanData data) {
        auto ret = new ClanCell;
        if (ret && ret->init(data)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(LGClanData data) {
        if (!CCMenu::init()) return false;

        this->setContentSize({360, 50});

        auto getColor = [](LGClanRating rating) {
            switch (rating) {
            case LGClanRating::Common: return ccColor3B{255,255,255}; break;
            case LGClanRating::Featured: return ccColor3B{240,211,42}; break;
            case LGClanRating::Epic: return ccColor3B{ 237, 191, 247 }; break;
            }
        };

        auto getOpacity = [](LGClanRating rating) {
            switch (rating) {
            case LGClanRating::Common: return 25; break;
            case LGClanRating::Featured: return 65; break;
            case LGClanRating::Epic: return 65; break;
            }
        };

        auto bg = Build(NineSlice::create("square02b_001.png"))
            .id("bg")
            .opacity(getOpacity(data.m_clanRating))
            .contentSize({490, 50})
            .color(getColor(data.m_clanRating))
            .ignoreAnchorPointForPos(false)
            .anchorPoint(.5f, .5f)
            .scale(.7f)
            .parentAtPos(this, Anchor::Center)
            .collect();

        auto clanNameLabel = Build(CCLabelBMFont::create(data.m_clanName.c_str(), "bigFont.fnt"))
            .parent(this)
            .id("clan-name-label")
            .pos({15, 33})
            .anchorPoint(0, 0.5)
            .scale(0.4f)
            .limitLabelWidth(150, 0.4, 0.1)
            .collect();

        if (data.m_clanDescription != "") {
            auto clanDescLabel = Build(CCLabelBMFont::create(data.m_clanDescription.c_str(), "goldFont.fnt"))
                .parent(this)
                .id("clan-desc-label")
                .pos({15, 18})
                .anchorPoint(0, 0.5)
                .limitLabelWidth(184, 0.4, 0.1)
                .scale(0.4f)
                .collect();
        }

        auto getJoinBtnSpr = [](LGClanJoinType type) {
            switch (type) {
            case LGClanJoinType::Open: return ButtonSprite::create("Join", "bigFont.fnt", "GJ_button_01.png"); break;
            case LGClanJoinType::ByRequest: return ButtonSprite::create("Request", "bigFont.fnt", "GJ_button_01.png"); break;
            case LGClanJoinType::Closed: return ButtonSprite::create("Closed", "bigFont.fnt", "GJ_button_04.png"); break;
            }
        };

        auto joinBtn = Build(getJoinBtnSpr(data.m_clanJoinType))
            .scale(0.5f)
            .anchorPoint({1, 0.5f})
            .intoMenuItem([&] {

            })
            .parent(this)
            .id("join-btn")
            .pos({305, 25})
            .collect();

        auto membersLabelText = fmt::format("{}/{}", data.m_membersAmount, data.m_maxMembers);

        auto membersLabel = Build(CCLabelBMFont::create(membersLabelText.c_str(), "goldFont.fnt"))
            .scale(0.4f)
            .anchorPoint({1, 0.5f})
            .parent(this)
            .id("members-label")
            .pos({300 - joinBtn->getScaledContentWidth() / 2, 25})
            .collect();

        auto typeSpr = Build(
            data.m_clanType == LGClanType::Star ? CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png") : CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png")
        )
            .scale(0.5f)
            .parent(this)
            .id("type-spr")
            .pos({25 + clanNameLabel->getScaledContentWidth(), 32})
            .collect();

        auto clanIDLabel = Build(CCLabelBMFont::create(
            fmt::format("{}", data.m_clanID).c_str(), "chatFont.fnt"
        ))
            .anchorPoint({0, 0.5f})
            .scale(0.5f)
            .opacity(100)
            .id("clan-id-label")
            .parent(this)
            .pos({25 + clanNameLabel->getScaledContentWidth() + typeSpr->getScaledContentWidth(), 32})
            .collect();

        return true;
    }
};

}