#include "GrindPacksLayer.hpp"

#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "../popups/AddPackPopup.hpp"
#include "../components/GrindPackCell.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include "Geode/utils/web.hpp"
#include <UIBuilder.hpp>
#include <fmt/format.h>

static constexpr CCSize LIST_SIZE {356.f, 220.f};
static constexpr int PER_PAGE = 10;

namespace levelgrind {

GrindPacksLayer* GrindPacksLayer::create() {
    auto ret = new GrindPacksLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GrindPacksLayer::init() {
    if (!BaseLayer::init()) return false;

    replaceBgToClassic();

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    addSideArt(this, SideArt::Bottom, false);

    auto uiMenu = Build(CCMenu::create())
        .pos({ 0, 0 })
        .collect();

    auto infoButton = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            FLAlertLayer::create(
                "Grind Packs",
                "Each pack consists of 3 grindable levels. Find one for you!",
                "OK"
            )->show();
        })
        .pos({ 25, 25 })
        .parent(uiMenu)
        .collect();

    m_listNode = Build(cue::ListNode::create({LIST_SIZE.width, LIST_SIZE.height}, {191, 114, 62, 255}, cue::ListBorderStyle::Levels))
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ winSize.width / 2 - 5.f, winSize.height / 2 - 5.f })
        .parent(this)
        .zOrder(5)
        .collect();

    auto titleLabel = Build(CCLabelBMFont::create("Grind Packs", "bigFont.fnt"))
        .scale(0.8f)
        .parent(m_listNode)
        .zOrder(6)
        .anchorPoint({ 0.5f, 0.5f })
        .pos({
            m_listNode->getContentWidth() / 2,
            m_listNode->getContentHeight() + 18
        })
        .collect();

    m_scrollLayer = m_listNode->getScrollLayer();

    auto scrollBar = Build(geode::Scrollbar::create(m_scrollLayer))
        .parent(this)
        .pos({
            m_listNode->getPositionX() + m_listNode->getContentWidth() / 2 + 25,
            centerY()
        })
        .scale(0.9f)
        .collect();

    auto countLabel = Build(CCLabelBMFont::create("0 packs found", "goldFont.fnt"))
        .pos({winSize.width - 5, winSize.height - 5})
        .anchorPoint({ 1, 1 })
        .scale(0.45f)
        .parent(this)
        .collect();

    auto contentLayer = m_scrollLayer->m_contentLayer;

    auto s = Ref(this);

    auto spinner = LoadingSpinner::create(64.f);
    if (spinner) {
        spinner->setID("spinner");
        auto win = CCDirector::sharedDirector()->getWinSize();
        spinner->setPosition(win / 2);
        this->addChild(spinner, 1000);
    }

    auto spinnerRef = Ref(spinner);
    auto countLabelRef = Ref(countLabel);

    m_listener.spawn(
        APIClient::getInstance().getGrindPacks(),
        [s, spinnerRef, countLabelRef](web::WebResponse r) {
            if (!s) return;
            auto parsed = APIClient::getInstance().getGrindPacksParse(r);

            if (!parsed.ok) {
                Notification::create("Failed! Try again later.", NotificationIcon::Error)->show();
                return;
            }

            spinnerRef->removeFromParent();

            for (const auto& val : parsed.packs) {
                auto cell = GrindPackCell::create(val);
                s->m_listNode->addCell(cell);
            }

            countLabelRef->setString(fmt::format("{} packs found", parsed.packs.size()).c_str());

            s->m_listNode->updateLayout();
            s->m_listNode->getScrollLayer()->m_contentLayer->updateLayout();
            s->m_listNode->scrollToTop();
        }
    );

    if (DataManager::getInstance().getUserPosition() != GrindPosition::Owner) return true;

    auto addPackMenu = Build(CCMenu::create())
        .pos({ 25, 25 })
        .parent(this)
        .id("add-pack-menu")
        .collect();

    auto addPackBtn = Build(CCSprite::createWithSpriteFrameName("GJ_listAddBtn_001.png"))
        .intoMenuItem([] {
            AddPackPopup::create()->show();
        })
        .parent(addPackMenu)
        .id("add-pack-btn")
        .collect();

    return true;
}

}