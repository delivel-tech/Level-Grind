#include "LGLevelBrowserLayer.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/ScrollLayer.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>

#include <cue/RepeatingBackground.hpp>

using namespace geode::prelude;

static constexpr CCSize LIST_SIZE{356.f, 220.f};
static constexpr int PER_PAGE = 10;

LGLevelBrowserLayer* LGLevelBrowserLayer::create(
    std::vector<int> difficulties, 
    std::vector<int> lengths, 
    std::vector<std::string> grindTypes, 
    std::vector<int> demonDifficulties, 
    std::vector<int> versions
) {
    auto ret = new LGLevelBrowserLayer;

    ret->m_difficulties = difficulties;
    ret->m_lengths = lengths;
    ret->m_grindTypes = grindTypes;
    ret->m_demonDifficulties = demonDifficulties;
    ret->m_versions = versions;

    if (ret && ret->init(nullptr)) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool LGLevelBrowserLayer::init(GJSearchObject* object) {
    if (!CCLayer::init()) return false;

    
    m_searchObject = nullptr;
    m_listLayer = nullptr;
    m_scrollLayer = nullptr;
    m_circle = nullptr;
    m_levelsLabel = nullptr;
    m_pageButtonLabel = nullptr;
    m_pageButton = nullptr;
    m_refreshBtn = nullptr;
    m_prevButton = nullptr;
    m_nextButton = nullptr;
    
    m_searchObject = object;

    auto bg = cue::RepeatingBackground::create("game_bg_01_001.png", 1.0f, cue::RepeatMode::X);
    bg->setColor({34, 60, 110});
    
    this->addChild(bg);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    const char* title = "Grinding Levels";

    auto uiMenu = CCMenu::create();
    uiMenu->setPosition({ 0, 0 });

    if (!Mod::get()->getSettingValue<bool>("disable-star-particles")) {
        auto grindParticles = CCParticleSnow::create();
        auto texture = CCTextureCache::sharedTextureCache()->addImage("GJ_bigStar_noShadow.png"_spr, true);
        grindParticles->m_fStartSpin = 0.f;
        grindParticles->m_fEndSpin = 180.f;
        grindParticles->m_fStartSize = 6.f;
        grindParticles->m_fEndSize = 3.f;
        grindParticles->setTexture(texture);

        this->addChild(grindParticles);
    }

    addBackButton(this, BackButtonStyle::Green);

    auto cornerLeft = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    auto cornerRight = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    cornerRight->setFlipX(true);

    this->addChild(cornerLeft);
    this->addChild(cornerRight);

    cornerLeft->setPosition({ 35.5, 35.5 });
    cornerRight->setPosition({ winSize.width - 35.5f, 35.5f });

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");

    auto infoButton = CCMenuItemSpriteExtra::create(
        infoSpr,
        this,
        menu_selector(LGLevelBrowserLayer::onInfoButton)
    );
    infoButton->setPosition({ 25, 25 });
    uiMenu->addChild(infoButton);

    m_listLayer = GJListLayer::create(
        nullptr, title, {191, 114, 62, 255}, LIST_SIZE.width, LIST_SIZE.height, 0
    );

    m_listLayer->setPosition(
        { winSize / 2 - m_listLayer->getScaledContentSize() / 2 - 5 }
    );

    auto scrollLayer = ScrollLayer::create({
        m_listLayer->getContentSize().width, m_listLayer->getContentSize().height
    });
    scrollLayer->setPosition({ 0, 0 });
    m_listLayer->addChild(scrollLayer);

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

    this->addChild(m_listLayer);
    this->addChild(uiMenu, 10);

    m_levelsLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_levelsLabel->setPosition({ winSize.width - 5, winSize.height - 5 });
    m_levelsLabel->setAnchorPoint({ 1.f, 1.f });
    m_levelsLabel->setScale(0.45f);
    this->addChild(m_levelsLabel, 10);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    if (pageBtnSpr) {
        m_pageButton = CCMenuItemSpriteExtra::create(
            pageBtnSpr, this, menu_selector(LGLevelBrowserLayer::onPageButton)
        );
        if (m_pageButton) {
            auto pageMenu = CCMenu::create();
            pageMenu->setID("lg-page-menu");
            pageMenu->setContentSize({ 28, 110 });
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
            pageMenu->updateLayout();

            m_pageButtonLabel = CCLabelBMFont::create(numToString(m_page + 1).c_str(), "bigFont.fnt");

            if (m_pageButtonLabel) {
                auto size = m_pageButton->getContentSize();
                m_pageButtonLabel->setPosition({size.width / 2.f, size.height / 2.f});
                m_pageButtonLabel->setAnchorPoint({0.5f, 0.5f});
                m_pageButtonLabel->setID("lg-page-label");
                m_pageButtonLabel->setScale(0.6f);
                m_pageButton->addChild(m_pageButtonLabel, 1);
            }
            this->updatePageButton();
        }
    }

    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    refreshSpr->setScale(0.75f);
    m_refreshBtn = CCMenuItemSpriteExtra::create(
        refreshSpr, this, menu_selector(LGLevelBrowserLayer::onRefresh));
    m_refreshBtn->setPosition({winSize.width - 35, 35});
    m_refreshBtn->setID("refresh-btn");

    uiMenu->addChild(m_refreshBtn);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");

    m_prevButton = CCMenuItemSpriteExtra::create(
        prevSpr, 
        this, 
        menu_selector(LGLevelBrowserLayer::onPrevPage)
    );

    m_prevButton->setPosition({20, winSize.height / 2});
    uiMenu->addChild(m_prevButton);

    if (m_prevButton) {
        m_prevButton->setVisible(false);
    }

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_nextButton = CCMenuItemSpriteExtra::create(
        nextSpr, 
        this, 
        menu_selector(LGLevelBrowserLayer::onNextPage)
    );
    m_nextButton->setPosition({winSize.width - 20, winSize.height / 2});
    uiMenu->addChild(m_nextButton);

    if (m_nextButton) m_nextButton->setVisible(false);

    m_circle = nullptr;

    auto glm = GameLevelManager::get();
    if (glm) {
        glm->m_levelManagerDelegate = this;
    }

    this->scheduleUpdate();
    this->setKeypadEnabled(true);
    
    this->performFetchLevels();

    return true;
}

void LGLevelBrowserLayer::onInfoButton(CCObject* sender) {
    FLAlertLayer::create(
        "Levels for grinding",
        "Here you can find <cp>levels</c> for <cy>grinding</c>!\n" \
        "You can also <cr>configure</c> what levels to search for in the main <cj>Level Grind</c> layer.\n" \
        "Thanks for using <cj>Level Grind</c> mod!",
        "OK"
    )->show();
}

void LGLevelBrowserLayer::startLoading() {
    m_loading = true;
    
    if (m_circle) {
        m_circle->removeFromParent();
        m_circle = nullptr;
    }
    
    auto spinner = LoadingSpinner::create(64.f);
    if (spinner) {
        spinner->setID("lg-spinner");
        auto win = CCDirector::sharedDirector()->getWinSize();
        spinner->setPosition(win / 2);
        this->addChild(spinner, 1000);
        m_circle = spinner;
    } else {
        log::error("failed to create loading spinner");
    }
}

void LGLevelBrowserLayer::stopLoading() {
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

void LGLevelBrowserLayer::hideUIElements() {
    if (m_nextButton) m_nextButton->setVisible(false);
    if (m_refreshBtn) m_refreshBtn->setVisible(false);
    if (m_pageButton) m_pageButton->setVisible(false);
    if (m_levelsLabel) m_levelsLabel->setVisible(false);
}

void LGLevelBrowserLayer::showUIElements() {
    if (m_refreshBtn) m_refreshBtn->setVisible(true);
    if (m_levelsLabel) m_levelsLabel->setVisible(true);
    
    if (m_prevButton) m_prevButton->setVisible(m_page > 0);
    if (m_nextButton) m_nextButton->setVisible(m_page + 1 < m_totalPages);
    if (m_pageButton) m_pageButton->setVisible(m_totalPages > 1);
}

void LGLevelBrowserLayer::refreshLevels(bool force) {
    if (m_loading) return;
    
    m_allLevelIDs.clear(); 
    m_levelCache.clear();
    
    this->hideUIElements();
    
    if (m_scrollLayer) {
        m_scrollLayer->setVisible(false);
    }
    
    this->performFetchLevels();
}

void LGLevelBrowserLayer::onNextPage(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    if (m_loading) return;
    if (m_page + 1 < m_totalPages) {
        m_page++;
        m_levelCache.clear();
        
        this->hideUIElements();
        
        this->loadPageFromStoredIDs(); 
    }
}

void LGLevelBrowserLayer::onPrevPage(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    if (m_loading) return;
    if (m_page > 0) {
        m_page--;
        m_levelCache.clear();
        
        this->hideUIElements();
        
        this->loadPageFromStoredIDs(); 
    }
}

void LGLevelBrowserLayer::onRefresh(CCObject* sender) {
    if (!this->getParent() || !this->isRunning()) return;
    this->refreshLevels(false);
}

void LGLevelBrowserLayer::loadPageFromStoredIDs() {
    if (m_allLevelIDs.empty() || m_loading) return;
    
    if (m_scrollLayer && m_scrollLayer->m_contentLayer) {
        m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);
    }
    
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
        glm->m_levelManagerDelegate = this;
        glm->getOnlineLevels(m_searchObject);
    } else {
        stopLoading();
    }
}

void LGLevelBrowserLayer::updatePageButton() {
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

void LGLevelBrowserLayer::onPageButton(CCObject* sender) {
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

void LGLevelBrowserLayer::keyBackClicked() {
    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade);
}

void LGLevelBrowserLayer::loadLevelsFinished(CCArray* levels, char const* key, int p2) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }
    
    if (!levels) {
        stopLoading();
        return;
    }
    
    for (auto lvlObj : CCArrayExt<GJGameLevel*>(levels)) {
        if (!lvlObj) continue;
        m_levelCache[lvlObj->m_levelID] = lvlObj;
    }
    
    populateFromArray(levels);
    m_scrollLayer->setVisible(true);
    stopLoading();
}

void LGLevelBrowserLayer::loadLevelsFailed(char const* key, int p1) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }
    Notification::create("Failed to load levels from GD servers", NotificationIcon::Error)->show();
    stopLoading();
}

void LGLevelBrowserLayer::populateFromArray(CCArray* levels) {
    if (!this->getParent() || !this->isRunning()) {
        return;
    }
    
    if (!m_scrollLayer || !m_scrollLayer->m_contentLayer || !levels) return;
    
    auto contentLayer = m_scrollLayer->m_contentLayer;
    contentLayer->removeAllChildrenWithCleanup(true);
    
    const float cellH = 90.f;
    int index = 0;
    
    for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(levels)) {
        if (!level) continue;
        
        auto cell = new LevelCell("LGLevelCell", 356.f, cellH);
        cell->autorelease();
        cell->loadFromLevel(level);
        cell->setContentSize({356.f, cellH});
        cell->setAnchorPoint({0.0f, 1.0f});
        cell->updateBGColor(index);
        contentLayer->addChild(cell);
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
}

void LGLevelBrowserLayer::setIDPopupClosed(SetIDPopup* popup, int value) {
    if (!this->getParent() || !this->isRunning()) return;
    if (!popup || popup->m_cancelled) return;
    
    if (value < 1) value = 1;
    if (m_totalPages > 0 && value > m_totalPages) value = m_totalPages;
    
    int newPage = value - 1;
    if (newPage != m_page) {
        m_page = newPage;
        m_levelCache.clear();
        this->updatePageButton();
        this->loadPageFromStoredIDs(); 
    }
}

void LGLevelBrowserLayer::onEnter() {
    CCLayer::onEnter();
    this->setTouchEnabled(true);
    this->scheduleUpdate();
}

void LGLevelBrowserLayer::onExit() {
    auto glm = GameLevelManager::get();
    if (glm && glm->m_levelManagerDelegate == this) {
        glm->m_levelManagerDelegate = nullptr;
    }
    CCLayer::onExit();
}

void LGLevelBrowserLayer::update(float dt) {
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

void LGLevelBrowserLayer::performFetchLevels() {
    if (m_loading) return;
    
    this->startLoading();
    
    matjson::Value body;
    if (!m_difficulties.empty()) body["difficulties"] = m_difficulties;
    if (!m_lengths.empty()) body["lengths"] = m_lengths;
    if (!m_demonDifficulties.empty()) body["demonDifficulties"] = m_demonDifficulties;
    if (!m_grindTypes.empty()) body["grindTypes"] = m_grindTypes;
    if (!m_versions.empty()) body["versions"] = m_versions;
    
    auto req = web::WebRequest();
    req.bodyJSON(body);
    
    WeakRef<LGLevelBrowserLayer> weakSelf = this;
    
    m_searchTask.spawn(
        req.post("https://delivel.tech/grindapi/get_levels"),
        [weakSelf](web::WebResponse const& res) {
            
            auto self = weakSelf.lock();
            if (!self) return;
            if (!self->getParent() || !self->isRunning()) return;
            
            if (!res.ok()) {
                Notification::create("Failed to fetch levels", NotificationIcon::Error)->show();
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
            
            int totalCount = 0;
            if (json.contains("count")) {
                if (auto count = json["count"].as<int>(); count) {
                    totalCount = count.unwrap();
                }
            }
            
            if (totalCount == 0) {
                Notification::create("No levels found", NotificationIcon::Info)->show();
                self->stopLoading();
                
                
                if (self->m_scrollLayer && self->m_scrollLayer->m_contentLayer) {
                    self->m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);
                }
                
                self->m_levelsLabel->setString("0 to 0 of 0");
                self->m_totalLevels = 0;
                self->m_totalPages = 1;
                self->updatePageButton();
                return;
            }
            
            std::vector<int> allIDs;
            if (json.contains("ids")) {
                auto arrRes = json["ids"].asArray();
                if (arrRes) {
                    for (auto id : arrRes.unwrap()) {
                        if (auto idVal = id.asInt(); idVal) {
                            allIDs.push_back(idVal.unwrap());
                        }
                    }
                }
            }
            
            if (allIDs.empty()) {
                Notification::create("No levels found", NotificationIcon::Info)->show();
                self->stopLoading();
                
                
                if (self->m_scrollLayer && self->m_scrollLayer->m_contentLayer) {
                    self->m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);
                }
                
                self->m_levelsLabel->setString("0 to 0 of 0");
                self->m_totalLevels = 0;
                self->m_totalPages = 1;
                self->updatePageButton();
                return;
            }
            
            std::vector<int> filteredIDs;
            bool onlyUncompleted = false;
            
            if (auto mod = Mod::get()) {
                onlyUncompleted = mod->getSettingValue<bool>("only-uncompleted");
            }
            
            if (onlyUncompleted) {
                auto gsm = GameStatsManager::sharedState();
                if (gsm) {
                    for (auto id : allIDs) {
                        auto isCompleted = gsm->hasCompletedOnlineLevel(id);
                        if (!isCompleted) {
                            filteredIDs.push_back(id);
                        }
                    }
                } else {
                    filteredIDs = allIDs;
                }
                
                if (filteredIDs.empty()) {
                    Notification::create("No uncompleted levels found", NotificationIcon::Info)->show();
                    self->stopLoading();
                    
                    if (self->m_scrollLayer && self->m_scrollLayer->m_contentLayer) {
                        self->m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);
                    }
                    
                    self->m_levelsLabel->setString("0 to 0 of 0");
                    self->m_totalLevels = 0;
                    self->m_totalPages = 1;
                    self->updatePageButton();
                    return;
                }
            } else {
                filteredIDs = allIDs;
            }
            
            self->m_allLevelIDs = filteredIDs;
            self->m_totalLevels = static_cast<int>(filteredIDs.size());
            self->m_totalPages = (self->m_totalLevels + PER_PAGE - 1) / PER_PAGE;
            if (self->m_totalPages < 1) {
                self->m_totalPages = 1;
            }
            
            if (self->m_page >= self->m_totalPages) {
                self->m_page = std::max(0, self->m_totalPages - 1);
            }
            
            int startIdx = self->m_page * PER_PAGE;
            int endIdx = std::min(startIdx + PER_PAGE, static_cast<int>(filteredIDs.size()));
            
            std::vector<int> pageIDs;
            for (int i = startIdx; i < endIdx; ++i) {
                pageIDs.push_back(filteredIDs[i]);
            }
            
            if (pageIDs.size() > 99) {
                pageIDs.resize(99);
            }
            
            std::string levelIDs;
            for (size_t i = 0; i < pageIDs.size(); ++i) {
                if (i > 0) levelIDs += ",";
                levelIDs += numToString(pageIDs[i]);
            }
            
            self->m_searchObject = GJSearchObject::create(SearchType::Type19, levelIDs);
            
            auto glm = GameLevelManager::get();
            if (glm && self->getParent() && self->isRunning()) {
                glm->m_levelManagerDelegate = self;
                glm->getOnlineLevels(self->m_searchObject);
            } else {
                self->stopLoading();
            }
        }
    );
}