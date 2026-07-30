#include "ClanViewerLayer.hpp"
#include "Geode/cocos/CCDirector.h"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

#include <UIBuilder.hpp>
#include "../popups/GuidePopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/PetManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/ScrollLayer.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include "Geode/utils/web.hpp"

#include "../popups/CreateClanPopup.hpp"
#include "../components/ClanCell.hpp"

namespace levelgrind {

ClanViewerLayer* ClanViewerLayer::create() {
    auto ret = new ClanViewerLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ClanViewerLayer::changeMusic(MusicMode mode) {
    if (mode == MusicMode::In) {
        auto fmod = FMODAudioEngine::sharedEngine();
        m_prevMusicVolume = fmod->getBackgroundMusicVolume();
        fmod->pauseMusic(0);
        fmod->playMusic("dangerLoop.mp3", true, 1.f, 0);
        fmod->setBackgroundMusicVolume(m_prevMusicVolume + 0.5f);
    }
    if (mode == MusicMode::Out) {
        FMODAudioEngine::sharedEngine()->setBackgroundMusicVolume(m_prevMusicVolume);
        auto gm = GameManager::sharedState();
        gm->fadeInMenuMusic();
    }
}

void ClanViewerLayer::onBack(CCObject *sender) {
    changeMusic(MusicMode::Out);

    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade
    );
}

bool ClanViewerLayer::init() {
    if (!BaseLayer::init()) return false;

    addSideArt(
        this, SideArt::Bottom, SideArtStyle::LayerGray, false
    );

    changeMusic(MusicMode::In);

    m_backBtn->removeFromParent();
    m_backBtn = nullptr;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this,
        menu_selector(ClanViewerLayer::onBack));
    backBtn->setPosition(25, winSize.height - 25);

    auto backBtnMenu = CCMenu::create();
    backBtnMenu->setPosition(0.f, 0.f);
    backBtnMenu->setZOrder(1);
    this->addChild(backBtnMenu);
    backBtnMenu->addChild(backBtn);

    m_bg->setColor({ 60, 60, 60 });
    m_bg->setSpeed(0.5f);

    auto infoBtnMenu = Build<CCMenu>::create()
        .id("info-btn-menu")
        .pos(
            winSize.width - 25.f, 25.f
        )
        .parent(this)
        .collect();

    auto infoBtn = Build(CCSprite::create("info_btn.png"_spr))
        .scale(0.7f)
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::PetGuide, GuidePopupState::FromOutside)->show();
        })
        .id("info-btn")
        .parent(infoBtnMenu)
        .collect();

    auto mainPanel = Build(NineSlice::create("GJ_square05.png"))
        .contentSize({460.f, 280.f})
        .parent(this)
        .center()
        .id("main-panel")
        .collect();

    CCSize mainPanelCS = mainPanel->getContentSize();

    auto loading = LoadingSpinner::create(50.f);
    mainPanel->addChild(loading);
    loading->setPosition(
        mainPanelCS.width / 2, mainPanelCS.height / 2
    );

    auto loadingRef = Ref(loading);
    auto self = Ref(this);
    auto mainPanelRef = Ref(mainPanel);

    auto& pm = PetManager::getInstance();

    m_listener.spawn(
        APIClient::getInstance().viewClans(
            pm.shouldUpdatePetStars(),
            pm.getPetStarsDelta(),
            pm.shouldUpdatePetMoons(),
            pm.getPetMoonsDelta()
        ),
        [loadingRef, self, mainPanelRef](web::WebResponse res) {
            if (!self || !loadingRef || !mainPanelRef) return;
            auto data = APIClient::getInstance().viewClansParse(res);
            if (!data.ok) {
                Notification::create("Failed to load clans!", NotificationIcon::Error)->show();
                if (!loadingRef) loadingRef->removeFromParent();
                return;
            }

            loadingRef->removeFromParent();
            self->initUI(data, mainPanelRef);
        }
    );
    
    return true;
}

void ClanViewerLayer::initUI(ViewClansResponse data, Ref<geode::NineSlice> mainPanel) {
    auto scrollLayerBG = Build(NineSlice::create("square02_small.png"))
        .contentSize({360, 200})
        .pos({195, 153})
        .opacity(100)
        .id("scroll-layer-bg")
        .parent(mainPanel)
        .collect();
    
    auto scrollLayer = Build(geode::ScrollLayer::create(
        {360, 200}
    ))
        .pos(
            15,
            53
        )
        .id("scroll-layer")
        .parent(mainPanel)
        .collect();

    auto scrollBar = Build(geode::Scrollbar::create(scrollLayer))
        .scale(0.9f)
        .pos(206 + scrollLayer->getContentWidth() / 2, 153)
        .id("scrollbar")
        .parent(mainPanel)
        .collect();

    auto buttonsBG = Build(NineSlice::create("square02_small.png"))
        .opacity(100)
        .contentSize({45, 200})
        .pos({420, 153})
        .id("buttons-bg")
        .parent(mainPanel)
        .collect();

    auto downButtonsMenu = Build(CCMenu::create())
        .layout(RowLayout::create()->setGap(15))
        .pos(
            mainPanel->getContentWidth() / 2,
            30
        )
        .scale(0.7f)
        .parent(mainPanel)
        .id("down-buttons-menu")
        .collect();

    /*
    if (data.myClanID != 0) {
        initClanUI(data, mainPanel);
        return;
    }
    */

    if (data.clans.empty()) {
        auto labelNotFound = Build(CCLabelBMFont::create("No clans are found - be first!", "bigFont.fnt"))
            .scale(0.5f)
            .pos(scrollLayer->getContentSize() / 2)
            .id("label-not-found")
            .parent(scrollLayer)
            .collect();
    }

    auto createClanBtn = Build(ButtonSprite::create(
        "Create Clan", "bigFont.fnt", "GJ_button_04.png"
    ))
        .intoMenuItem([] {
            CreateClanPopup::create([]{})->show();
        })
        .parent(downButtonsMenu)
        .id("create-clan-btn")
        .collect();

    downButtonsMenu->updateLayout();

    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()->setGap(-5)->setAxisAlignment(AxisAlignment::End)->setAxisReverse(true));

    auto clans = data.clans;

    for (const auto& clan : clans) {
        auto cell = ClanCell::create(clan);

        scrollLayer->m_contentLayer->addChild(cell);
        scrollLayer->m_contentLayer->updateLayout();
    }

    return;
}

void ClanViewerLayer::initClanUI(ViewClansResponse res, Ref<geode::NineSlice> mainPanel) {
    return;
}

}