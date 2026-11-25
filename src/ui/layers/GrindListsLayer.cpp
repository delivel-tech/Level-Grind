#include "GrindListsLayer.hpp"
#include "../popups/GuidePopup.hpp"
#include <Geode/ui/Label.hpp>

#include <Geode/binding/LevelListCell.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

namespace levelgrind {

static constexpr CCSize LIST_SIZE { 356.f, 220.f };
static constexpr float CELL_H = 90.f;
static constexpr int PER_PAGE = 10;

GrindListsLayer* GrindListsLayer::create() {
    auto ret = new GrindListsLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GrindListsLayer::init() {
    if (!BaseLayer::init()) return false;
    replaceBgToClassic();
    addSideArt(this, SideArt::Bottom, SideArtStyle::LayerGray, false);
    m_filters = lists::Filters::load();
    auto winSize = m_winSize;
    GuidePopup::addInfoButton(this, GuidePage::GrindListsGuide, { 25.f, 25.f });

    m_listNode = Build(cue::ListNode::create(LIST_SIZE, { 191, 114, 62, 255 }, cue::ListBorderStyle::Levels))
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ winSize.width / 2, winSize.height / 2 - 5.f })
        .zOrder(5)
        .parent(this)
        .collect();

    Build(geode::Label::create("Grind Lists (Beta)", "bigFont.fnt"))
        .pos({ LIST_SIZE.width / 2, LIST_SIZE.height + 18.f })
        .with([&](auto label) { label->setLimitLabelWidth(LIST_SIZE.width - 30.f, 0.8f, 0.4f); })
        .zOrder(1)
        .parent(m_listNode);

    m_countLabel = Build(geode::Label::create("", "goldFont.fnt"))
        .anchorPoint({ 1.f, 1.f })
        .pos({ winSize.width - 5.f, winSize.height - 5.f })
        .scale(0.45f)
        .zOrder(10)
        .parent(this)
        .collect();

    m_status = Build(geode::Label::create("", "bigFont.fnt"))
        .pos({ winSize.width / 2, winSize.height / 2 - 5.f })
        .scale(0.45f)
        .zOrder(10)
        .visible(false)
        .parent(this)
        .collect();

    auto menu = Build(CCMenu::create()).pos({ 0.f, 0.f }).zOrder(10).parent(this).collect();

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prev = CCMenuItemExt::createSpriteExtra(prevSpr, [this](CCObject*) {
        if (m_page > 0) { m_page--; this->showPage(); }
    });
    m_prev->setPosition({ 24.f, winSize.height / 2 });
    menu->addChild(m_prev);

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_next = CCMenuItemExt::createSpriteExtra(nextSpr, [this](CCObject*) {
        m_page++;
        this->showPage();
    });
    m_next->setPosition({ winSize.width - 24.f, winSize.height / 2 });
    menu->addChild(m_next);

    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    refreshSpr->setScale(0.75f);
    auto refresh = CCMenuItemExt::createSpriteExtra(refreshSpr, [this](CCObject*) { this->load(); });
    refresh->setPosition({ winSize.width - 35.f, 35.f });
    refresh->setID("refresh-btn");
    menu->addChild(refresh);

    this->load();
    return true;
}

void GrindListsLayer::setStatus(std::string const& text) {
    m_status->setString(text.c_str());
    m_status->setLimitLabelWidth(LIST_SIZE.width - 30.f, 0.45f, 0.2f);
    m_status->setVisible(!text.empty());
}

void GrindListsLayer::load() {
    m_lists.clear();
    m_page = 0;
    m_listNode->clear();
    m_prev->setVisible(false);
    m_next->setVisible(false);
    m_countLabel->setString("");
    this->setStatus("");

    if (m_spinner) m_spinner->removeFromParent();
    m_spinner = LoadingSpinner::create(50.f);
    m_spinner->setPosition({ m_winSize.width / 2, m_winSize.height / 2 - 5.f });
    this->addChild(m_spinner, 20);

    m_task.spawn(lists::fetchLists(m_filters), [this](web::WebResponse res) {
        if (m_spinner) {
            m_spinner->removeFromParent();
            m_spinner = nullptr;
        }
        std::string error;
        m_lists = lists::parseLists(res, m_filters, error);
        if (!error.empty()) {
            this->setStatus(error);
            return;
        }
        this->showPage();
    });
}

void GrindListsLayer::showPage() {
    m_listNode->clear();
    int total = static_cast<int>(m_lists.size());
    int pages = std::max(1, (total + PER_PAGE - 1) / PER_PAGE);
    m_page = std::clamp(m_page, 0, pages - 1);

    if (total == 0) {
        this->setStatus("No lists match your filters");
        m_countLabel->setString("");
        m_prev->setVisible(false);
        m_next->setVisible(false);
        return;
    }
    this->setStatus("");

    int start = m_page * PER_PAGE;
    int end = std::min(start + PER_PAGE, total);
    for (int i = start; i < end; i++) {
        auto const& data = m_lists[i];
        auto cell = LevelListCell::create(LIST_SIZE.width, CELL_H);
        if (!cell) continue;

        cell->loadFromList(lists::makeLevelList(data));
        cell->setContentSize({ LIST_SIZE.width, CELL_H });
        cell->setAnchorPoint({ 0.f, 1.f });
        cell->updateBGColor(i - start);

        auto progress = geode::Label::create(
            data.finished ? "Done!" : fmt::format("{} left", data.left).c_str(), "goldFont.fnt");
        progress->setScale(0.45f);
        progress->setAnchorPoint({ 1.f, 0.f });
        progress->setPosition({ LIST_SIZE.width - 8.f, 4.f });
        if (data.finished) progress->setColor({ 120, 255, 120 });
        cell->addChild(progress, 10);

        m_listNode->addCell(cell);
    }

    m_countLabel->setString(fmt::format("{} to {} of {}", start + 1, end, total).c_str());
    m_prev->setVisible(m_page > 0);
    m_next->setVisible(m_page < pages - 1);
}

}
