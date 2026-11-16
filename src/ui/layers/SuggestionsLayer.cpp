#include "SuggestionsLayer.hpp"
#include "../popups/SuggestionsFilterPopup.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/utils/web.hpp"
#include "../../managers/DataManager.hpp"
#include "../../utils/utils.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <UIBuilder.hpp>
#include <cue/ListNode.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <algorithm>

#include "../popups/GuidePopup.hpp"

using namespace geode::prelude;

static constexpr CCSize LIST_SIZE {356.f, 220.f};
static constexpr int PER_PAGE = 10;

namespace levelgrind {

SuggestionsLayer* SuggestionsLayer::create() {
    auto ret = new SuggestionsLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SuggestionsLayer::init() {
    if (!BaseLayer::init()) return false;

    m_title = "Helper Suggestions";

    replaceBgToClassic();

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    addSideArt(this, SideArt::Bottom, false);

    auto uiMenu = Build(CCMenu::create())
        .pos({ 0, 0 })
        .collect();

    auto infoButton = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::HelperSuggestionsGuide, GuidePopupState::FromOutside)->show();
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

    auto scrollLayer = m_listNode->getScrollLayer();
    m_scrollLayer = scrollLayer;

    auto contentLayer = scrollLayer->m_contentLayer;
    if (contentLayer) {
        auto layout = ColumnLayout::create();
        contentLayer->setLayout(layout);
        layout->setGap(0.f);
        layout->setAutoGrowAxis(220.f);
        layout->setAxisReverse(true);
        layout->setAxisAlignment(AxisAlignment::End);
    }

    this->addChild(uiMenu, 10);

    m_levelsLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_levelsLabel->setPosition({winSize.width - 5, winSize.height - 5});
    m_levelsLabel->setAnchorPoint({1.f, 1.f});
    m_levelsLabel->setScale(0.45f);
    this->addChild(m_levelsLabel, 10);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    if (pageBtnSpr) {
        m_pageButton = CCMenuItemSpriteExtra::create(
            pageBtnSpr, this, menu_selector(SuggestionsLayer::onPageButton)
        );
        if (m_pageButton) {
            auto pageMenu = CCMenu::create();
            pageMenu->setID("cb-page-menu");
            pageMenu->setContentSize({28, 110});
            pageMenu->setPosition({winSize.width - 7, winSize.height - 80});
            pageMenu->setAnchorPoint({1.f, 0.5f});
            pageMenu->setLayout(ColumnLayout::create()
                                    ->setGap(5.f)
                                    ->setAutoScale(true)
                                    ->setGrowCrossAxis(true)
                                    ->setCrossAxisOverflow(true)
                                    ->setAxisReverse(true)
                                    ->setAxisAlignment(AxisAlignment::End));
            pageMenu->addChild(m_pageButton);
            this->addChild(pageMenu, 10);

            if (DataManager::getInstance().getUserPosition() != GrindPosition::User) {
                auto filterBtnSpr = CCSprite::create("GJ_button_01.png");
                filterBtnSpr->setScale(0.7f);
                auto filterBtnSprTop = CCSprite::createWithSpriteFrameName("badge_helper.png"_spr);
                filterBtnSpr->addChild(filterBtnSprTop);
                filterBtnSprTop->setPosition({ 19.5f, 20.5f });
                filterBtnSprTop->setScale(1.2f);
                m_filterBtn = Build(filterBtnSpr)
                    .intoMenuItem([this] {
                        this->onFilterButton(nullptr);
                    })
                    .parent(pageMenu)
                    .collect();
            }

            pageMenu->updateLayout();

            m_pageButtonLabel = CCLabelBMFont::create(numToString(m_page + 1).c_str(), "bigFont.fnt");

            if (m_pageButtonLabel) {
                auto size = m_pageButton->getContentSize();
                m_pageButtonLabel->setPosition({size.width / 2.f, size.height / 2.f});
                m_pageButtonLabel->setAnchorPoint({0.5f, 0.5f});
                m_pageButtonLabel->setID("cb-page-label");
                m_pageButtonLabel->setScale(0.6f);
                m_pageButton->addChild(m_pageButtonLabel, 1);
            }
            this->updatePageButton();
        }
    }

    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    refreshSpr->setScale(0.75f);
    m_refreshBtn = CCMenuItemSpriteExtra::create(
        refreshSpr, this, menu_selector(SuggestionsLayer::onRefresh)
    );
    m_refreshBtn->setPosition({winSize.width - 35, 35});
    m_refreshBtn->setID("refresh-btn");

    uiMenu->addChild(m_refreshBtn);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");

    m_prevButton = CCMenuItemSpriteExtra::create(
        prevSpr,
        this,
        menu_selector(SuggestionsLayer::onPrevPage)
    );

    m_prevButton->setPosition({20, winSize.height / 2});
    m_prevButton->setID("prev-page-button");
    uiMenu->addChild(m_prevButton);

    if (m_prevButton) {
        m_prevButton->setVisible(false);
    }

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_nextButton = CCMenuItemSpriteExtra::create(
        nextSpr,
        this,
        menu_selector(SuggestionsLayer::onNextPage)
    );
    m_nextButton->setPosition({winSize.width - 20, winSize.height / 2});
    m_nextButton->setID("next-page-button");
    uiMenu->addChild(m_nextButton);

    if (m_nextButton) {
        m_nextButton->setVisible(false);
    }

    m_circle = nullptr;

    auto glm = GameLevelManager::get();
    if (glm) {
        glm->m_levelManagerDelegate = this;
    }

    this->scheduleUpdate();
    this->setKeypadEnabled(true);

    auto titleLabel = Build(CCLabelBMFont::create(m_title.c_str(), "bigFont.fnt"))
        .anchorPoint({ 0.5f, 0.5f })
        .pos({LIST_SIZE.width / 2.f, LIST_SIZE.height + 18.f})
        .limitLabelWidth(LIST_SIZE.width - 30.f, .8f, 0.5f)
        .parent(m_listNode)
        .zOrder(1)
        .collect();

    this->performFetchLevels();

    return true;
}

void SuggestionsLayer::onInfoButton(CCObject* sender) {
    FLAlertLayer::create(
        "Levels for grinding",
        "Here you can find <cp>levels</c> for <cy>grinding</c>!\n"
        "You can also <cr>configure</c> what levels to search for in the main <cj>Level Grind</c> layer.\n"
        "Thanks for using <cj>Level Grind</c> mod!",
        "OK"
    )->show();
}

void SuggestionsLayer::startLoading() {
    m_loading = true;

    if (m_circle) {
        m_circle->removeFromParent();
        m_circle = nullptr;
    }

    auto spinner = LoadingSpinner::create(64.f);
    if (spinner) {
        spinner->setID("cb-spinner");
        auto win = CCDirector::sharedDirector()->getWinSize();
        spinner->setPosition(win / 2);
        this->addChild(spinner, 1000);
        m_circle = spinner;
    }
}

void SuggestionsLayer::stopLoading() {
    m_loading = false;

    if (m_circle) {
        m_circle->removeFromParent();
        m_circle = nullptr;
    }

    if (m_scrollLayer) {
        m_scrollLayer->setVisible(true);
    }

    this->showUIElements();
}

void SuggestionsLayer::hideUIElements() {
    if (m_nextButton) m_nextButton->setVisible(false);
    if (m_refreshBtn) m_refreshBtn->setVisible(false);
    if (m_pageButton) m_pageButton->setVisible(false);
    if (m_levelsLabel) m_levelsLabel->setVisible(false);
}

void SuggestionsLayer::showUIElements() {
    if (m_refreshBtn) m_refreshBtn->setVisible(true);
    if (m_filterBtn) m_filterBtn->setVisible(true);
    if (m_levelsLabel) m_levelsLabel->setVisible(true);

    if (m_prevButton) m_prevButton->setVisible(m_page > 0);
    if (m_nextButton) m_nextButton->setVisible(m_page + 1 < m_totalPages);
    if (m_pageButton) m_pageButton->setVisible(m_totalPages > 1);
}

void SuggestionsLayer::refreshLevels() {
    if (m_loading) return;

    m_page = 0;
    m_allLevelIDs.clear();

    this->hideUIElements();

    if (m_scrollLayer) {
        m_scrollLayer->setVisible(false);
    }

    this->performFetchLevels();
}

void SuggestionsLayer::onNextPage(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    if (m_loading) return;
    if (m_page + 1 < m_totalPages) {
        m_page++;

        this->hideUIElements();

        this->loadPageFromStoredIDs();
    }
}

void SuggestionsLayer::onPrevPage(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    if (m_loading) return;
    if (m_page > 0) {
        m_page--;

        this->hideUIElements();

        this->loadPageFromStoredIDs();
    }
}

void SuggestionsLayer::onRefresh(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    this->refreshLevels();
}

void SuggestionsLayer::onFilterButton(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    auto popup = SuggestionsFilterPopup::create(this);
    if (popup) {
        popup->show();
    }
}

void SuggestionsLayer::setPointsFilter(PointsFilterOp op, int value) {
    m_pointsFilterOp = op;
    m_pointsFilterValue = value;
}

SuggestionsLayer::PointsFilterOp SuggestionsLayer::getPointsFilterOp() const {
    return m_pointsFilterOp;
}

int SuggestionsLayer::getPointsFilterValue() const {
    return m_pointsFilterValue;
}

int SuggestionsLayer::difficultyOf(LevelSuggestion const& s) {
    switch (s.difficulty) {
        case 1: return CustomDifficultyEnum::Auto;
        case 2: return CustomDifficultyEnum::Easy;
        case 3: return CustomDifficultyEnum::Normal;
        case 4: case 5: return CustomDifficultyEnum::Hard;
        case 6: case 7: return CustomDifficultyEnum::Harder;
        case 8: case 9: return CustomDifficultyEnum::Insane;
        case 10:
            switch (s.demonDifficulty) {
                case 3: return CustomDifficultyEnum::EasyDemon;
                case 4: return CustomDifficultyEnum::MediumDemon;
                case 5: return CustomDifficultyEnum::InsaneDemon;
                case 6: return CustomDifficultyEnum::ExtremeDemon;
                default: return CustomDifficultyEnum::HardDemon;
            }
    }
    return 0;
}

int SuggestionsLayer::countPointsFilter(PointsFilterOp op, int value, int difficulty) const {
    int count = 0;
    for (auto const& suggestion : m_allLevelSuggestions) {
        if (difficulty != 0 && difficultyOf(suggestion) != difficulty) continue;
        switch (op) {
            case PointsFilterOp::More: count += suggestion.points > value; break;
            case PointsFilterOp::Equals: count += suggestion.points == value; break;
            case PointsFilterOp::Less: count += suggestion.points < value; break;
        }
    }
    return count;
}

int SuggestionsLayer::getSuggestionsMode() const {
    return m_suggestionsMode;
}

void SuggestionsLayer::setSuggestionsMode(int mode) {
    if (mode == m_suggestionsMode) return;
    m_suggestionsMode = mode;
    m_page = 0;
    this->refreshLevels();
}

int SuggestionsLayer::getSuggestionsCount() const {
    return static_cast<int>(m_allLevelSuggestions.size());
}

void SuggestionsLayer::applyPointsFilter() {
    m_allLevelIDs.clear();
    for (auto const& suggestion : m_allLevelSuggestions) {
        if (m_difficultyFilter != 0 && difficultyOf(suggestion) != m_difficultyFilter) continue;
        if (m_bounties.contains(suggestion.id)) {
            m_allLevelIDs.push_back(suggestion.id);
            continue;
        }
        bool keep = false;
        switch (m_pointsFilterOp) {
            case PointsFilterOp::More:
                keep = suggestion.points > m_pointsFilterValue;
                break;
            case PointsFilterOp::Equals:
                keep = suggestion.points == m_pointsFilterValue;
                break;
            case PointsFilterOp::Less:
                keep = suggestion.points < m_pointsFilterValue;
                break;
        }
        if (keep) {
            m_allLevelIDs.push_back(suggestion.id);
        }
    }

    if (!m_bounties.empty()) {
        std::stable_partition(m_allLevelIDs.begin(), m_allLevelIDs.end(), [this](int id) {
            return m_bounties.contains(id);
        });
    }

    m_totalLevels = static_cast<int>(m_allLevelIDs.size());
    m_totalPages = (m_totalLevels + PER_PAGE - 1) / PER_PAGE;
    if (m_totalPages < 1) {
        m_totalPages = 1;
    }
    if (m_page >= m_totalPages) {
        m_page = std::max(0, m_totalPages - 1);
    }

    if (m_allLevelIDs.empty()) {
        if (m_listNode) {
            m_listNode->clear();
        }
        if (m_levelsLabel) {
            m_levelsLabel->setString("0 to 0 of 0");
        }
        this->updatePageButton();
        if (m_loading) {
            this->stopLoading();
        }
        return;
    }

    this->loadPageFromStoredIDs();
}

void SuggestionsLayer::loadPageFromStoredIDs() {
    if (m_allLevelIDs.empty()) return;

    if (m_listNode) m_listNode->clear();

    this->startLoading();

    int startIdx = m_page * PER_PAGE;
    int endIdx = std::min(startIdx + PER_PAGE, static_cast<int>(m_allLevelIDs.size()));

    if (startIdx >= static_cast<int>(m_allLevelIDs.size())) {
        stopLoading();
        return;
    }

    std::vector<int> pageIDs;
    for (int i = startIdx; i < endIdx; ++i) {
        pageIDs.push_back(m_allLevelIDs[i]);
    }

    if (pageIDs.size() > 99) {
        pageIDs.resize(99);
    }

    std::string levelIDs;
    for (size_t i = 0; i < pageIDs.size(); ++i) {
        if (i > 0) levelIDs += ",";
        levelIDs += numToString(pageIDs[i]);
    }

    m_searchObject = GJSearchObject::create(SearchType::Type19, levelIDs);

    auto glm = GameLevelManager::get();
    if (glm) {
        glm->m_searchType = SearchType::Type19;
        glm->m_levelManagerDelegate = this;
        glm->getOnlineLevels(m_searchObject);
    } else {
        stopLoading();
    }
}

void SuggestionsLayer::updatePageButton() {
    if (!this->getParent() || !this->isRunning()) return;

    if (m_prevButton) {
        m_prevButton->setVisible(m_page > 0);
    }
    if (m_nextButton) {
        m_nextButton->setVisible(m_page + 1 < m_totalPages);
    }
    if (m_pageButtonLabel) {
        m_pageButtonLabel->setString(numToString(m_page + 1).c_str());
    }
    if (m_pageButton) {
        m_pageButton->setVisible(m_totalPages > 1);
    }
}

void SuggestionsLayer::onPageButton(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;

    int current = std::clamp(m_page + 1, 1, std::max(1, m_totalPages));
    int begin = 1;
    int end = std::max(1, m_totalPages);
    auto popup = SetIDPopup::create(current, begin, end, "Go to Page", "Go",
                                    false, current, 60.f, true, true);
    if (popup) {
        popup->m_delegate = this;
        popup->show();
    }
}

void SuggestionsLayer::keyBackClicked() {
    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade
    );
}

void SuggestionsLayer::loadLevelsFinished(CCArray* levels, char const* key, int p2) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }

    if (!levels) {
        stopLoading();
        return;
    }

    populateFromArray(levels);
    if (m_scrollLayer) {
        m_scrollLayer->setVisible(true);
    }
    stopLoading();
}

void SuggestionsLayer::loadLevelsFailed(char const* key, int p1) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }
    Notification::create("Failed to load levels from GD servers", NotificationIcon::Error)->show();
    stopLoading();
}

void SuggestionsLayer::populateFromArray(CCArray* levels) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }

    if (!m_listNode || !levels) return;

    m_listNode->clear();

    const float cellH = 90.f;
    int index = 0;

    std::vector<GJGameLevel*> ordered;
    for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(levels)) {
        if (level) ordered.push_back(level);
    }
    std::unordered_map<int, size_t> rank;
    for (size_t r = 0; r < m_allLevelIDs.size(); r++) rank.emplace(m_allLevelIDs[r], r);
    std::stable_sort(ordered.begin(), ordered.end(), [&](GJGameLevel* a, GJGameLevel* b) {
        auto ra = rank.find(a->m_levelID.value());
        auto rb = rank.find(b->m_levelID.value());
        size_t va = ra == rank.end() ? SIZE_MAX : ra->second;
        size_t vb = rb == rank.end() ? SIZE_MAX : rb->second;
        return va < vb;
    });

    for (GJGameLevel* level : ordered) {
        auto cell = LevelCell::create(356.f, cellH);
        if (!cell) continue;
        cell->loadFromLevel(level);
        cell->setContentSize({356.f, cellH});
        cell->setAnchorPoint({0.0f, 1.0f});
        cell->updateBGColor(index);

        if (m_bounties.contains(level->m_levelID.value()) && cell->m_backgroundLayer) {
            cell->m_backgroundLayer->setColor(index % 2 == 0 ? ccColor3B{ 210, 160, 70 } : ccColor3B{ 230, 180, 85 });

            #ifndef GEODE_IS_IOS
            auto sparkles = CCParticleSnow::create();
            sparkles->setTotalParticles(24);
            sparkles->setPosition({ 178.f, 0.f });
            sparkles->setPosVar({ 178.f, 0.f });
            sparkles->setGravity({ 0.f, 18.f });
            sparkles->setSpeed(8.f);
            sparkles->setSpeedVar(4.f);
            sparkles->setAngle(90.f);
            sparkles->setAngleVar(20.f);
            sparkles->setLife(3.f);
            sparkles->setLifeVar(1.f);
            sparkles->setEmissionRate(6.f);
            sparkles->setStartSize(4.f);
            sparkles->setStartSizeVar(2.f);
            sparkles->setEndSize(0.f);
            sparkles->setStartColor({ 1.f, 0.9f, 0.5f, 0.9f });
            sparkles->setStartColorVar({ 0.f, 0.1f, 0.1f, 0.1f });
            sparkles->setEndColor({ 1.f, 0.8f, 0.3f, 0.f });
            sparkles->setEndColorVar({ 0.f, 0.f, 0.f, 0.f });
            sparkles->setBlendAdditive(true);
            sparkles->setPositionType(kCCPositionTypeGrouped);
            cell->addChild(sparkles, 1);
            #endif
        }

        m_listNode->addCell(cell);
        index++;
    }

    int returned = static_cast<int>(levels->count());
    int first = m_page * PER_PAGE + 1;
    int last = m_page * PER_PAGE + returned;
    if (returned == 0) {
        first = 0;
        last = 0;
    }
    int total = (m_totalLevels > 0) ? m_totalLevels : returned;

    if (m_levelsLabel) {
        m_levelsLabel->setString(
            fmt::format("{} to {} of {}", first, last, total).c_str()
        );
    }

    m_needsLayout = true;
    this->updatePageButton();

    m_scrollLayer->m_contentLayer->updateLayout();
    m_listNode->scrollToTop();
}

void SuggestionsLayer::setIDPopupClosed(SetIDPopup* popup, int value) {
    if (!this->getParent() || !this->isRunning()) return;
    if (!popup || popup->m_cancelled) return;

    if (value < 1) value = 1;
    if (m_totalPages > 0 && value > m_totalPages) value = m_totalPages;

    int newPage = value - 1;
    if (newPage != m_page) {
        m_page = newPage;
        this->updatePageButton();
        this->loadPageFromStoredIDs();
    }
}

void SuggestionsLayer::onEnter() {
    CCLayer::onEnter();
    this->setTouchEnabled(true);
    this->scheduleUpdate();
}

void SuggestionsLayer::onExit() {
    auto glm = GameLevelManager::get();
    if (glm && glm->m_levelManagerDelegate == this) {
        glm->m_levelManagerDelegate = nullptr;
    }
    CCLayer::onExit();
}

void SuggestionsLayer::update(float dt) {
    if (!this->getParent() || !this->isRunning()) return;

    if (m_needsLayout) {
        if (m_scrollLayer && m_scrollLayer->m_contentLayer) {
            auto contentLayer = m_scrollLayer->m_contentLayer;
            if (contentLayer->getChildren() &&
                contentLayer->getChildren()->count() > 0) {
                contentLayer->updateLayout();
                if (m_scrollLayer) {
                    m_scrollLayer->scrollToTop();
                }
            }
        }
        m_needsLayout = false;
    }

    if (m_pageButtonLabel) {
        m_pageButtonLabel->setString(numToString(m_page + 1).c_str());
    }
    if (m_pageButton) {
        m_pageButton->setVisible(m_totalPages > 1);
    }
}

void SuggestionsLayer::performFetchLevels() {
    if (m_loading) return;

    this->startLoading();

    auto req = web::WebRequest();
    req.timeout(std::chrono::seconds{20});
    WeakRef<SuggestionsLayer> weakSelf = this;

    auto url = fmt::format("https://api.delivel.tech/v2/get_suggestions?mode={}", m_suggestionsMode);
    m_searchTask.spawn(
        req.get(url),
        [weakSelf](web::WebResponse const& res) {
            auto self = weakSelf.lock();
            if (!self) return;
            if (!self->getParent() || !self->isRunning()) return;

            if (!res.ok()) {
                Notification::create(res.code() <= 0 ? "Server took too long, try Refresh" : "Failed to fetch levels", NotificationIcon::Error)->show();
                self->stopLoading();
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                Notification::create("Invalid response from server", NotificationIcon::Error)->show();
                self->stopLoading();
                return;
            }

            auto json = jsonRes.unwrap();

            if (!json["ok"].asBool().unwrapOrDefault()) {
                Notification::create("Invalid response from server", NotificationIcon::Error)->show();
                self->stopLoading();
                return;
            }

            auto suggestions = json["suggestions"].asArray();

            if (!suggestions) {
                Notification::create("Invalid response from server", NotificationIcon::Error)->show();
                self->stopLoading();
                return;
            }

            auto suggestionsArr = suggestions.unwrap();

            self->m_allLevelSuggestions.clear();

            self->m_bounties.clear();
            self->m_bountyGp = static_cast<int>(json["bountyGp"].asInt().unwrapOr(20));
            if (DataManager::getInstance().getUserPosition() != GrindPosition::User) {
                if (auto bounties = json["bounties"].asArray()) {
                    for (auto const& b : bounties.unwrap()) {
                        int id = static_cast<int>(b["levelID"].asInt().unwrapOr(0));
                        if (id > 0) self->m_bounties[id] = static_cast<int>(b["waitingDays"].asInt().unwrapOr(0));
                    }
                }
            }

            for (auto const& val : suggestionsArr) {
                LevelSuggestion suggestion;
                suggestion.id = val["levelID"].asInt().unwrapOrDefault();
                suggestion.points = val["points"].asInt().unwrapOrDefault();
                suggestion.difficulty = static_cast<int>(val["difficulty"].asInt().unwrapOr(0));
                suggestion.demonDifficulty = static_cast<int>(val["demonDifficulty"].asInt().unwrapOr(0));
                self->m_allLevelSuggestions.push_back(suggestion);
            }

            if (self->m_allLevelSuggestions.empty()) {
                Notification::create("No levels found", NotificationIcon::Info)->show();
                self->stopLoading();

                if (self->m_listNode) self->m_listNode->clear();

                if (self->m_levelsLabel) {
                    self->m_levelsLabel->setString("0 to 0 of 0");
                }
                self->m_totalLevels = 0;
                self->m_totalPages = 1;
                self->updatePageButton();
                return;
            }

            self->applyPointsFilter();
        }
    );
}

}