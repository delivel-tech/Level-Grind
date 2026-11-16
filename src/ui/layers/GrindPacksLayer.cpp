#include "GrindPacksLayer.hpp"

#include "../../managers/APIClient.hpp"
#include "../popups/AddPackPopup.hpp"
#include "../components/GrindPackCell.hpp"
#include "../../managers/GrindClansManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/SetIDPopup.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <cctype>
#include <Geode/binding/ButtonSprite.hpp>
#include "../popups/GuidePopup.hpp"
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
        .parent(this)
        .zOrder(10)
        .collect();

    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    refreshSpr->setScale(0.75f);
    m_refreshBtn = CCMenuItemSpriteExtra::create(
        refreshSpr, this, menu_selector(GrindPacksLayer::onRefresh)
    );
    m_refreshBtn->setPosition({ winSize.width - 35, 35 });
    m_refreshBtn->setID("refresh-btn");
    m_refreshBtn->setVisible(false);
    uiMenu->addChild(m_refreshBtn);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prevButton = CCMenuItemSpriteExtra::create(
        prevSpr, this, menu_selector(GrindPacksLayer::onPrevPage)
    );
    m_prevButton->setPosition({ 20, winSize.height / 2 });
    m_prevButton->setID("prev-page-button");
    m_prevButton->setVisible(false);
    uiMenu->addChild(m_prevButton);

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_nextButton = CCMenuItemSpriteExtra::create(
        nextSpr, this, menu_selector(GrindPacksLayer::onNextPage)
    );
    m_nextButton->setPosition({ winSize.width - 20, winSize.height / 2 });
    m_nextButton->setID("next-page-button");
    m_nextButton->setVisible(false);
    uiMenu->addChild(m_nextButton);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    if (pageBtnSpr) {
        m_pageButton = CCMenuItemSpriteExtra::create(
            pageBtnSpr, this, menu_selector(GrindPacksLayer::onPageButton)
        );
        if (m_pageButton) {
            auto pageMenu = CCMenu::create();
            pageMenu->setID("gp-page-menu");
            pageMenu->setContentSize({28, 110});
            pageMenu->setPosition({winSize.width - 7, winSize.height - 80});
            pageMenu->setAnchorPoint({1.f, 0.5f});
            pageMenu->setLayout(
                ColumnLayout::create()
                    ->setGap(5.f)
                    ->setAutoScale(true)
                    ->setGrowCrossAxis(true)
                    ->setCrossAxisOverflow(true)
                    ->setAxisReverse(true)
                    ->setAxisAlignment(AxisAlignment::End)
            );
            pageMenu->addChild(m_pageButton);
            this->addChild(pageMenu, 10);   

            m_pageButtonLabel = CCLabelBMFont::create(
                numToString(m_page + 1).c_str(), "bigFont.fnt"
            );
            if (m_pageButtonLabel) {
                auto size = m_pageButton->getContentSize();
                m_pageButtonLabel->setPosition({size.width / 2.f, size.height / 2.f});
                m_pageButtonLabel->setAnchorPoint({0.5f, 0.5f});
                m_pageButtonLabel->setID("gp-page-label");
                m_pageButtonLabel->setScale(0.6f);
                m_pageButton->addChild(m_pageButtonLabel, 1);
            }

        pageMenu->updateLayout();
        m_pageButton->setVisible(false);
        this->updatePageButton();
    }
}

    m_listNode = Build(cue::ListNode::create({LIST_SIZE.width, LIST_SIZE.height}, {191, 114, 62, 255}, cue::ListBorderStyle::Levels))
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ winSize.width / 2 - 5.f, winSize.height / 2 - 5.f })
        .parent(this)
        .zOrder(5)
        .collect();

    Build(CCLabelBMFont::create("Grind Packs", "bigFont.fnt"))
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

    Build(geode::Scrollbar::create(m_scrollLayer))
        .parent(this)
        .pos({
            m_listNode->getPositionX() + m_listNode->getContentWidth() / 2 + 25,
            centerY()
        })
        .scale(0.9f)
        .collect();

    m_packsLabel = Build(CCLabelBMFont::create("", "goldFont.fnt"))
        .pos({ winSize.width - 5, winSize.height - 5 })
        .anchorPoint({ 1.f, 1.f })
        .scale(0.45f)
        .parent(this)
        .zOrder(10)
        .collect();
    m_packsLabel->setVisible(false);

    auto spinner = LoadingSpinner::create(64.f);
    spinner->setID("gp-spinner");
    spinner->setPosition(winSize / 2);
    this->addChild(spinner, 1000);
    m_circle = spinner;

    m_loading = true;

    auto s = Ref(this);
    m_listener.spawn(
        APIClient::getInstance().getGrindPacks(),
        [s](web::WebResponse r) {
            if (!s || !s->getParent() || !s->isRunning()) return;

            auto parsed = APIClient::getInstance().getGrindPacksParse(r);

            if (!parsed.ok) {
                Notification::create("Failed! Try again later.", NotificationIcon::Error)->show();
                s->stopLoading();
                return;
            }

            s->m_allPacks = parsed.packs;
            s->m_page = 0;

            s->stopLoading();
            s->applyFilters();
            GrindClansManager::getInstance().checkPacks(parsed.packs);
        }
    );

    auto addPackMenu = Build(CCMenu::create())
            .pos({ 25, 25 })
            .parent(this)
            .id("add-pack-menu")
            .collect();

    m_showStar = !Mod::get()->getSavedValue<bool>("packs-show-moon", false);
    m_sortAlpha = Mod::get()->getSavedValue<bool>("packs-sort-alpha", false);
    m_difficultyFilter = Mod::get()->getSavedValue<int>("packs-difficulty", 0);
    this->buildFilterButtons();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::GrindPacksGuide, GuidePopupState::FromOutside)->show();
        })
        .parent(addPackMenu)
        .id("info-btn")
        .collect();


    if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner || 
    DataManager::getInstance().getUserPosition() == GrindPosition::Admin) {

        Build(CCSprite::createWithSpriteFrameName("GJ_listAddBtn_001.png"))
            .intoMenuItem([] {
                AddPackPopup::create()->show();
            })
            .parent(addPackMenu)
            .posY(35)
            .id("add-pack-btn")
            .collect();
    }

    return true;
}

void GrindPacksLayer::startLoading() {
    m_loading = true;

    if (m_circle) {
        m_circle->removeFromParent();
        m_circle = nullptr;
    }

    auto spinner = LoadingSpinner::create(64.f);
    spinner->setID("gp-spinner");
    spinner->setPosition(CCDirector::sharedDirector()->getWinSize() / 2);
    this->addChild(spinner, 1000);
    m_circle = spinner;

    if (m_scrollLayer) m_scrollLayer->setVisible(false);
    hideUIElements();
}

void GrindPacksLayer::stopLoading() {
    m_loading = false;

    if (m_circle) {
        m_circle->removeFromParent();
        m_circle = nullptr;
    }

    if (m_scrollLayer) m_scrollLayer->setVisible(true);
}

void GrindPacksLayer::hideUIElements() {
    if (m_prevButton)  m_prevButton->setVisible(false);
    if (m_nextButton)  m_nextButton->setVisible(false);
    if (m_pageButton)  m_pageButton->setVisible(false);
    if (m_refreshBtn)  m_refreshBtn->setVisible(false);
    if (m_packsLabel)  m_packsLabel->setVisible(false);
}

void GrindPacksLayer::showUIElements() {
    if (m_refreshBtn)  m_refreshBtn->setVisible(true);
    if (m_packsLabel)  m_packsLabel->setVisible(true);
    if (m_prevButton)  m_prevButton->setVisible(m_page > 0);
    if (m_nextButton)  m_nextButton->setVisible(m_page + 1 < m_totalPages);
    if (m_pageButton)  m_pageButton->setVisible(m_totalPages > 1);
}

void GrindPacksLayer::updatePageButton() {
    if (m_pageButtonLabel)
        m_pageButtonLabel->setString(numToString(m_page + 1).c_str());
    if (m_prevButton)
        m_prevButton->setVisible(m_page > 0);
    if (m_nextButton)
        m_nextButton->setVisible(m_page + 1 < m_totalPages);
    if (m_pageButton)
        m_pageButton->setVisible(m_totalPages > 1);
}

void GrindPacksLayer::populatePacks() {
    if (!m_listNode) return;

    m_listNode->clear();

    int startIdx = m_page * PER_PAGE;
    int endIdx   = std::min(startIdx + PER_PAGE, static_cast<int>(m_shownPacks.size()));

    for (int i = startIdx; i < endIdx; ++i) {
        auto cell = GrindPackCell::create(m_shownPacks[i]);
        if (cell) m_listNode->addCell(cell);
    }

    if (m_emptyLabel) m_emptyLabel->setVisible(m_shownPacks.empty() && !m_allPacks.empty());

    int shown = endIdx - startIdx;
    int total = static_cast<int>(m_shownPacks.size());
    int first = total == 0 ? 0 : startIdx + 1;
    int last  = startIdx + shown;

    if (m_packsLabel) {
        m_packsLabel->setString(
            fmt::format("{} to {} of {} packs", first, last, total).c_str()
        );
    }

    m_listNode->updateLayout();
    if (m_scrollLayer) {
        m_scrollLayer->m_contentLayer->updateLayout();
        m_scrollLayer->scrollToTop();
    }

    showUIElements();
    updatePageButton();
}

static const char* packDifficultyFrame(int difficulty) {
    switch (difficulty) {
        case CustomDifficultyEnum::Auto: return "diffIcon_auto_btn_001.png";
        case CustomDifficultyEnum::Easy: return "diffIcon_01_btn_001.png";
        case CustomDifficultyEnum::Normal: return "diffIcon_02_btn_001.png";
        case CustomDifficultyEnum::Hard: return "diffIcon_03_btn_001.png";
        case CustomDifficultyEnum::Harder: return "diffIcon_04_btn_001.png";
        case CustomDifficultyEnum::Insane: return "diffIcon_05_btn_001.png";
        case CustomDifficultyEnum::EasyDemon: return "diffIcon_07_btn_001.png";
        case CustomDifficultyEnum::MediumDemon: return "diffIcon_08_btn_001.png";
        case CustomDifficultyEnum::HardDemon: return "diffIcon_06_btn_001.png";
        case CustomDifficultyEnum::InsaneDemon: return "diffIcon_09_btn_001.png";
        case CustomDifficultyEnum::ExtremeDemon: return "diffIcon_10_btn_001.png";
    }
    return "diffIcon_00_btn_001.png";
}

void GrindPacksLayer::buildFilterButtons() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto tabs = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(6.f)->setAutoScale(false))
        .contentSize({ 80.f, 34.f })
        .pos({ 42.f, DataManager::getInstance().getUserPosition() >= GrindPosition::Admin ? 102.f : 68.f })
        .id("pack-type-tabs")
        .parent(this)
        .zOrder(10)
        .collect();

    m_starTab = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    m_starTab->setScale(0.6f);
    tabs->addChild(CCMenuItemExt::createSpriteExtra(m_starTab, [this](CCObject*) {
        if (m_showStar) return;
        m_showStar = true;
        this->applyFilters();
    }));
    m_moonTab = CCSprite::createWithSpriteFrameName("GJ_bigMoon_001.png");
    m_moonTab->setScale(0.6f);
    tabs->addChild(CCMenuItemExt::createSpriteExtra(m_moonTab, [this](CCObject*) {
        if (!m_showStar) return;
        m_showStar = false;
        this->applyFilters();
    }));
    tabs->updateLayout();

    auto filters = Build<CCMenu>::create()
        .layout(ColumnLayout::create()->setGap(6.f)->setAxisReverse(true)->setAutoScale(false)->setAxisAlignment(AxisAlignment::End))
        .contentSize({ 40.f, 100.f })
        .anchorPoint({ 0.5f, 1.f })
        .pos({ 25.f, winSize.height - 48.f })
        .id("pack-filters")
        .parent(this)
        .zOrder(10)
        .collect();

    m_sortSpr = ButtonSprite::create("A-Z", 30, true, "bigFont.fnt", "GJ_button_04.png", 26.f, 0.5f);
    m_sortSpr->setScale(0.75f);
    filters->addChild(CCMenuItemExt::createSpriteExtra(m_sortSpr, [this](CCObject*) {
        m_sortAlpha = !m_sortAlpha;
        this->applyFilters();
    }));

    m_diffHolder = CCNode::create();
    m_diffHolder->setContentSize({ 34.f, 40.f });
    filters->addChild(CCMenuItemExt::createSpriteExtra(m_diffHolder, [this](CCObject*) {
        m_difficultyFilter = (m_difficultyFilter + 1) % 12;
        this->applyFilters();
    }));
    filters->updateLayout();

    m_emptyLabel = Build(CCLabelBMFont::create("No packs here yet", "bigFont.fnt"))
        .scale(0.5f)
        .pos({ winSize.width / 2 - 5.f, winSize.height / 2 - 5.f })
        .parent(this)
        .zOrder(20)
        .collect();
    m_emptyLabel->setVisible(false);

    this->refreshFilterButtons();
}

void GrindPacksLayer::refreshFilterButtons() {
    ccColor3B const on { 255, 255, 255 }, off { 90, 90, 90 };
    if (m_starTab) m_starTab->setColor(m_showStar ? on : off);
    if (m_moonTab) m_moonTab->setColor(m_showStar ? off : on);
    if (m_sortSpr) m_sortSpr->updateBGImage(m_sortAlpha ? "GJ_button_02.png" : "GJ_button_04.png");

    if (m_diffHolder) {
        m_diffHolder->removeAllChildren();
        auto face = CCSprite::createWithSpriteFrameName(packDifficultyFrame(m_difficultyFilter));
        face->setScale(0.6f);
        face->setPosition({ 17.f, 24.f });
        face->setOpacity(m_difficultyFilter == 0 ? 160 : 255);
        m_diffHolder->addChild(face);
        auto cap = CCLabelBMFont::create(m_difficultyFilter == 0 ? "All" : "Filter", "bigFont.fnt");
        cap->setScale(0.25f);
        cap->setPosition({ 17.f, 4.f });
        m_diffHolder->addChild(cap);
    }
}

void GrindPacksLayer::applyFilters() {
    Mod::get()->setSavedValue("packs-show-moon", !m_showStar);
    Mod::get()->setSavedValue("packs-sort-alpha", m_sortAlpha);
    Mod::get()->setSavedValue("packs-difficulty", m_difficultyFilter);

    m_shownPacks.clear();
    for (auto const& p : m_allPacks) {
        if (p.isStar != m_showStar) continue;
        if (m_difficultyFilter != 0 && static_cast<int>(p.difficulty) != m_difficultyFilter) continue;
        m_shownPacks.push_back(p);
    }
    if (m_sortAlpha) {
        auto lower = [](std::string s) {
            for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            return s;
        };
        std::stable_sort(m_shownPacks.begin(), m_shownPacks.end(), [&](GrindPack const& a, GrindPack const& b) {
            return lower(a.title) < lower(b.title);
        });
    }

    m_totalPages = std::max(1, (static_cast<int>(m_shownPacks.size()) + PER_PAGE - 1) / PER_PAGE);
    m_page = 0;
    this->refreshFilterButtons();
    if (!m_loading) this->populatePacks();
}

void GrindPacksLayer::onNextPage(CCObject*) {
    if (m_loading || m_page + 1 >= m_totalPages) return;
    m_page++;
    hideUIElements();
    populatePacks();
}

void GrindPacksLayer::onPrevPage(CCObject*) {
    if (m_loading || m_page <= 0) return;
    m_page--;
    hideUIElements();
    populatePacks();
}

void GrindPacksLayer::onPageButton(CCObject*) {
    int current = std::clamp(m_page + 1, 1, std::max(1, m_totalPages));
    auto popup = SetIDPopup::create(current, 1, std::max(1, m_totalPages),
                                    "Go to Page", "Go", false, current, 60.f, true, true);
    if (popup) {
        popup->m_delegate = this;
        popup->show();
    }
}

void GrindPacksLayer::setIDPopupClosed(SetIDPopup* popup, int value) {
    if (!popup || popup->m_cancelled) return;

    value = std::clamp(value, 1, std::max(1, m_totalPages));
    int newPage = value - 1;
    if (newPage == m_page) return;

    m_page = newPage;
    updatePageButton();
    populatePacks();
}

void GrindPacksLayer::onRefresh(CCObject*) {
    if (m_loading) return;

    m_allPacks.clear();
    m_page = 0;
    m_totalPages = 1;

    startLoading();

    auto s = Ref(this);
    m_listener.spawn(
        APIClient::getInstance().getGrindPacks(),
        [s](web::WebResponse r) {
            if (!s || !s->getParent() || !s->isRunning()) return;

            auto parsed = APIClient::getInstance().getGrindPacksParse(r);
            if (!parsed.ok) {
                Notification::create("Failed! Try again later.", NotificationIcon::Error)->show();
                s->stopLoading();
                return;
            }

            s->m_allPacks = parsed.packs;

            s->stopLoading();
            s->applyFilters();
        }
    );
}

}