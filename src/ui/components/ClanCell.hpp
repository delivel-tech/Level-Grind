#include <Geode/Geode.hpp>
#include "../../utils/globals.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/NineSlice.hpp"
#include "ccTypes.h"

#include <UIBuilder.hpp>

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

        return true;
    }
};

}