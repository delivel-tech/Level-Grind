#include "MainLayer.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <UIBuilder.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

namespace {

ButtonSprite* pill(char const* text, int width, bool on) {
    return ButtonSprite::create(text, width, true, "bigFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 30.f, 0.6f);
}

CCSprite* iconPill(char const* icon, bool on) {
    auto bg = CCSprite::create(on ? "GJ_button_02.png" : "GJ_button_04.png");
    auto top = CCSprite::createWithSpriteFrameName(icon);
    top->setPosition(bg->getContentSize() / 2);
    bg->addChild(top);
    return bg;
}

struct Option { char const* text; int value; };

constexpr Option kDiamonds[] = { { "Any", 0 }, { "10+", 10 }, { "25+", 25 }, { "50+", 50 }, { "MAX", 99 } };
constexpr Option kRequired[] = { { "Any", 0 }, { "5", 5 }, { "10", 10 }, { "20", 20 }, { "30", 30 } };
constexpr Option kLeft[] = { { "Any", 0 }, { "1", 1 }, { "3", 3 }, { "5", 5 }, { "10", 10 } };
constexpr Option kSorts[] = { { "Best Value", 0 }, { "Almost Done", 1 }, { "Most Diamonds", 2 }, { "Newest", 3 } };

}

bool MainLayer::listFiltersDefault() const {
    lists::Filters d;
    auto const& f = m_listFilters;
    return f.classic == d.classic && f.platformer == d.platformer && f.demons == d.demons && f.coins == d.coins
        && f.minDiamonds == d.minDiamonds && f.maxRequired == d.maxRequired && f.maxLeft == d.maxLeft
        && f.hideFinished == d.hideFinished && f.featuredOnly == d.featuredOnly && f.unratedOnly == d.unratedOnly
        && f.diffs == d.diffs
        && f.sort == d.sort;
}

void MainLayer::initListsMode() {
    m_listFilters = lists::Filters::load();

    m_listsPanel = Build<CCNode>::create()
        .contentSize(m_searchPanel->getContentSize())
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos(m_searchPanel->getPosition())
        .scale(m_searchPanel->getScale())
        .id("lists-panel"_spr)
        .parent(this)
        .collect();

    auto size = m_listsPanel->getContentSize();
    Build(NineSlice::create("GJ_square02.png"))
        .contentSize({ size.width + 20.f, size.height + 20.f })
        .anchorPoint({ 0.5f, 0.5f })
        .pos({ size.width / 2, size.height / 2 })
        .zOrder(-1)
        .id("panel-background")
        .parent(m_listsPanel);

    auto toggleMenu = Build(CCMenu::create())
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.5f })
        .contentSize({ 120.f, 30.f })
        .pos({ m_searchPanel->getPositionX() - (m_searchPanel->getContentWidth() + 20.f) * m_searchPanel->getScale() / 2 + 6.f, 26.f })
        .layout(RowLayout::create()->setGap(4.f)->setAutoScale(false)->setAxisAlignment(AxisAlignment::Start))
        .id("lists-toggle-menu"_spr)
        .parent(this)
        .collect();

    m_listsToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.55f, [this](CCMenuItemToggler* t) {
        lists::setListsMode(!t->isToggled());
        this->applyListsMode();
    });
    m_listsToggle->toggle(lists::listsMode());
    m_listsToggle->setID("lists-toggle"_spr);
    toggleMenu->addChild(m_listsToggle);

    auto toggleLabel = CCLabelBMFont::create("Lists (Beta)", "bigFont.fnt");
    toggleLabel->setScale(0.34f);
    toggleLabel->setID("lists-toggle-label"_spr);
    toggleMenu->addChild(toggleLabel);
    toggleMenu->updateLayout();

    buildListsPanel();
    applyListsMode();
}

void MainLayer::applyListsMode() {
    bool on = lists::listsMode();
    if (m_searchPanel) m_searchPanel->setVisible(!on);
    if (m_listsPanel) m_listsPanel->setVisible(on);
    if (m_randomBtn) m_randomBtn->setVisible(!on);
    if (auto menu = this->getChildByID("search-menu")) {
        if (m_actionScale < 0.f && m_resetBtn) m_actionScale = m_resetBtn->getScale();
        if (auto layout = typeinfo_cast<AxisLayout*>(menu->getLayout())) {
            layout->setAutoScale(false);
            layout->ignoreInvisibleChildren(true);
        }
        for (auto child : CCArrayExt<CCNode*>(menu->getChildren())) {
            if (child != m_randomBtn) child->setScale(m_actionScale);
        }
        menu->updateLayout();
    }
    if (m_countLabel) m_countLabel->setVisible(!on);
    if (m_starsRow && on) m_starsRow->setVisible(false);
    if (!on) scheduleCountRefresh();
    updateResetButton();
}

void MainLayer::buildListsPanel() {
    if (auto old = m_listsPanel->getChildByID("content")) old->removeFromParent();
    auto panelSize = m_listsPanel->getContentSize();
    auto content = Build<CCNode>::create()
        .contentSize(panelSize)
        .id("content")
        .parent(m_listsPanel)
        .collect();

    auto changed = [this]() {
        m_listFilters.save();
        Ref<MainLayer> self = this;
        Loader::get()->queueInMainThread([self] {
            self->buildListsPanel();
            self->updateResetButton();
        });
    };
    auto button = [&](CCMenu* menu, CCNode* spr, geode::Function<void()> apply) {
        menu->addChild(CCMenuItemExt::createSpriteExtra(spr, [apply = std::move(apply), changed](CCObject*) mutable {
            apply();
            changed();
        }));
    };
    auto radio = [&](CCMenu* menu, auto const& options, int current, int width, auto set) {
        for (auto const& o : options) {
            int value = o.value;
            button(menu, pill(o.text, width, value == current), [set, value] { set(value); });
        }
        menu->updateLayout();
    };
    auto faceBtn = [&](CCMenu* menu, char const* frame, bool* flag) {
        auto spr = CCSprite::createWithSpriteFrameName(frame);
        if (!*flag) spr->setColor({ 110, 110, 110 });
        button(menu, spr, [flag] { *flag = !*flag; });
    };

    float const leftWidth = 282.f;
    float const rightWidth = panelSize.width - leftWidth - 8.f;
    float const rightX = leftWidth + 8.f + rightWidth / 2;
    float const rowH = 30.f;
    float const rowGap = (panelSize.height - rowH * 5) / 4;
    auto rowY = [&](int row) { return panelSize.height - rowH / 2 - row * (rowH + rowGap); };
    float const leftTitle = 70.f;
    float const rightTitle = 56.f;
    float const colH = rowH * 4 + rowGap * 3;
    float const rightH = (colH - rowGap * 2) / 3;
    auto rightY = [&](int row) { return panelSize.height - rightH / 2 - row * (rightH + rowGap); };
    auto& f = m_listFilters;

    auto typeMenu = addSection(content, "Type", { leftWidth / 2, rowY(0) }, { leftWidth, rowH }, leftTitle, 0.6f, 10.f);
    struct { char const* icon; bool* flag; } types[] = {
        { "GJ_starsIcon_001.png", &f.classic },
        { "GJ_moonsIcon_001.png", &f.platformer },
        { "GJ_coinsIcon2_001.png", &f.coins },
        { "GJ_demonIcon_001.png", &f.demons },
    };
    for (auto const& t : types) {
        bool* flag = t.flag;
        button(typeMenu, iconPill(t.icon, *flag), [flag] { *flag = !*flag; });
    }
    typeMenu->updateLayout();

    auto diffMenu = addSection(content, "Difficulty", { leftWidth / 2, rowY(1) }, { leftWidth, rowH }, leftTitle, 0.68f, 8.f);
    char const* diffFrames[] = {
        "diffIcon_auto_btn_001.png", "diffIcon_01_btn_001.png", "diffIcon_02_btn_001.png",
        "diffIcon_03_btn_001.png", "diffIcon_04_btn_001.png", "diffIcon_05_btn_001.png",
    };
    for (int i = 0; i <= 5; i++) faceBtn(diffMenu, diffFrames[i], &f.diffs[i]);
    diffMenu->updateLayout();

    auto demonsMenu = addSection(content, "Demons", { leftWidth / 2, rowY(2) }, { leftWidth, rowH }, leftTitle, 0.6f, 12.f);
    if (f.demons) {
        char const* demonFrames[] = {
            "diffIcon_07_btn_001.png", "diffIcon_08_btn_001.png", "diffIcon_06_btn_001.png", "diffIcon_09_btn_001.png", "diffIcon_10_btn_001.png",
        };
        for (int i = 0; i < 5; i++) faceBtn(demonsMenu, demonFrames[i], &f.diffs[6 + i]);
        demonsMenu->updateLayout();
    } else {
        auto hint = Build<CCNode>::create()
            .anchorPoint({ 0.5f, 0.5f })
            .ignoreAnchorPointForPos(false)
            .contentSize({ 120.f, 20.f })
            .pos(demonsMenu->getPosition())
            .layout(RowLayout::create()->setGap(4.f)->setAutoScale(false))
            .id("demons-hint")
            .parent(demonsMenu->getParent())
            .collect();
        Build(CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png")).scale(0.6f).parent(hint);
        Build(CCLabelBMFont::create("only", "chatFont.fnt")).color({ 200, 200, 200 }).scale(0.7f).parent(hint);
        hint->updateLayout();
    }

    auto diamondsMenu = addSection(content, "Diamonds", { leftWidth / 2, rowY(3) }, { leftWidth, rowH }, leftTitle, 0.55f, 5.f);
    radio(diamondsMenu, kDiamonds, f.minDiamonds, 54, [&f](int v) { f.minDiamonds = v; });

    auto requiredMenu = addSection(content, "Max Levels", { rightX, rightY(0) }, { rightWidth, rightH }, rightTitle, 0.5f, 4.f);
    radio(requiredMenu, kRequired, f.maxRequired, 44, [&f](int v) { f.maxRequired = v; });

    auto leftMenu = addSection(content, "Max Left", { rightX, rightY(1) }, { rightWidth, rightH }, rightTitle, 0.5f, 4.f);
    radio(leftMenu, kLeft, f.maxLeft, 44, [&f](int v) { f.maxLeft = v; });

    auto showMenu = addSection(content, "Show", { rightX, rightY(2) }, { rightWidth, rightH }, rightTitle, 0.5f, 6.f);
    button(showMenu, pill("Hide Done", 110, f.hideFinished), [&f] { f.hideFinished = !f.hideFinished; });

    button(showMenu, pill("Rated", 70, f.featuredOnly), [&f] {
        f.featuredOnly = !f.featuredOnly;
        if (f.featuredOnly) f.unratedOnly = false;
    });
    button(showMenu, pill("Unrated", 84, f.unratedOnly), [&f] {
        f.unratedOnly = !f.unratedOnly;
        if (f.unratedOnly) f.featuredOnly = false;
    });
    showMenu->updateLayout();

    auto sortMenu = addSection(content, "Sort", { panelSize.width / 2, rowY(4) }, { panelSize.width, rowH }, leftTitle, 0.55f, 6.f);
    radio(sortMenu, kSorts, static_cast<int>(f.sort), 128, [&f](int v) { f.sort = static_cast<lists::Sort>(v); });
}

}
