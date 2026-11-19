#include "AchievementsPopup.hpp"
#include "../../managers/NotifyManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

constexpr CCSize kListSize { 290.f, 210.f };
constexpr float kCellH = 50.f;
constexpr float kTabsW = 100.f;
constexpr float kTabH = 34.f;

class WheelRouter : public CCLayer {
public:
    std::vector<geode::ScrollLayer*> m_targets;

    static WheelRouter* create(std::vector<geode::ScrollLayer*> targets) {
        auto ret = new WheelRouter;
        if (ret->init()) {
            ret->m_targets = std::move(targets);
            ret->setMouseEnabled(true);
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool scrollWheel(float y, float x) override {
        auto mouse = geode::cocos::getMousePos();
        for (auto layer : m_targets) {
            if (!layer || !nodeIsVisible(layer)) continue;
            auto local = layer->convertToNodeSpace(mouse);
            auto size = layer->getContentSize();
            if (local.x >= 0.f && local.y >= 0.f && local.x <= size.width && local.y <= size.height) {
                layer->scrollWheel(y, x);
                break;
            }
        }
        return true;
    }
};

class TapCatcher : public CCLayer {
public:
    geode::Function<void(CCPoint)> m_onTap;
    CCPoint m_start;
    bool m_moved = false;

    static TapCatcher* create(geode::Function<void(CCPoint)> onTap) {
        auto ret = new TapCatcher;
        if (ret->init()) {
            ret->m_onTap = std::move(onTap);
            ret->setTouchEnabled(true);
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void registerWithTouchDispatcher() override {
        int prio = -600;
        for (auto node = this->getParent(); node; node = node->getParent()) {
            if (auto delegate = typeinfo_cast<CCTouchDelegate*>(node)) {
                if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
                    prio = std::min(prio, handler->m_nPriority - 10);
                    break;
                }
            }
        }
        CCTouchDispatcher::get()->addTargetedDelegate(this, prio, false);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        m_start = touch->getLocation();
        m_moved = false;
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (touch->getLocation().getDistance(m_start) > 8.f) m_moved = true;
    }

    void ccTouchEnded(CCTouch* touch, CCEvent*) override {
        if (!m_moved && m_onTap) m_onTap(touch->getLocation());
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        m_moved = true;
    }
};

CCSprite* categoryIcon(achievements::Category const* c) {
    if (!c) return CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    auto spr = c->modSprite ? CCSprite::create(c->icon) : CCSprite::createWithSpriteFrameName(c->icon);
    return spr ? spr : CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
}

}

AchievementsPopup* AchievementsPopup::create() {
    auto ret = new AchievementsPopup;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AchievementsPopup::init() {
    if (!BasePopup::init(440.f, 280.f)) return false;
    this->setTitle("Achievements");

    m_tabs = cue::ListNode::create({ kTabsW, kListSize.height });
    m_tabs->setAutoUpdate(false);
    m_tabs->setPosition({ 64.f, centerY() - 12.f });
    m_mainLayer->addChild(m_tabs);

    m_list = cue::ListNode::create(kListSize);
    m_list->setAutoUpdate(false);
    m_list->setPosition({ 64.f + kTabsW / 2.f + 18.f + kListSize.width / 2.f, centerY() - 12.f });
    m_mainLayer->addChild(m_list);

    auto tabsBar = Scrollbar::create(m_tabs->getScrollLayer());
    tabsBar->setPosition({ 64.f + kTabsW / 2.f + 6.f, centerY() - 12.f });
    tabsBar->setScale(0.7f);
    m_mainLayer->addChild(tabsBar);
    auto listBar = Scrollbar::create(m_list->getScrollLayer());
    listBar->setPosition({ m_list->getPositionX() + kListSize.width / 2.f + 7.f, centerY() - 12.f });
    listBar->setScale(0.7f);
    m_mainLayer->addChild(listBar);
    m_mainLayer->addChild(WheelRouter::create({ m_tabs->getScrollLayer(), m_list->getScrollLayer() }), 100);

    WeakRef<AchievementsPopup> weak = this;
    m_mainLayer->addChild(TapCatcher::create([weak](CCPoint p) {
        auto self = weak.lock();
        if (!self || !self->m_tabs) return;
        auto local = self->m_tabs->convertToNodeSpace(p);
        auto size = self->m_tabs->getContentSize();
        if (local.x < 0.f || local.y < 0.f || local.x > size.width || local.y > size.height) return;
        auto content = self->m_tabs->getScrollLayer()->m_contentLayer;
        auto inContent = content->convertToNodeSpace(p);
        auto const& cats = achievements::categories();
        for (size_t i = 0; i < self->m_tabs->size() && i < cats.size(); i++) {
            auto cell = self->m_tabs->getCell(i);
            if (!cell || !cell->boundingBox().containsPoint(inContent)) continue;
            std::string id = cats[i].id;
            if (self->m_category == id) return;
            self->m_category = id;
            Loader::get()->queueInMainThread([weak] {
                auto self = weak.lock();
                if (!self) return;
                self->buildTabs();
                self->buildList();
            });
            return;
        }
    }), 101);

    m_summary = CCLabelBMFont::create("", "bigFont.fnt");
    m_summary->setScale(0.3f);
    m_summary->setPosition({ centerX(), 14.f });
    m_mainLayer->addChild(m_summary);

    auto spinner = LoadingSpinner::create(40.f);
    spinner->setPosition(m_list->getPosition());
    m_mainLayer->addChild(spinner, 10);
    m_spinner = spinner;

    this->buildTabs();

    Ref<AchievementsPopup> self = this;
    achievements::check([self](std::vector<achievements::Achievement> const& all) {
        if (!self) return;
        if (self->m_spinner) {
            self->m_spinner->removeFromParent();
            self->m_spinner = nullptr;
        }
        self->m_all = all;
        if (all.empty()) {
            self->m_summary->setString("Log in (Argon) to see your achievements");
            return;
        }
        self->buildTabs();
        self->buildList();
    });
    return true;
}

void AchievementsPopup::buildTabs() {
    bool const redraw = m_tabs->size() > 0;
    auto const scroll = m_tabs->getScrollPos();
    m_tabs->clear();
    Ref<AchievementsPopup> self = this;
    for (auto const& c : achievements::categories()) {
        int done = 0, total = 0;
        for (auto const& a : m_all) {
            if (a.category != c.id) continue;
            total++;
            done += a.unlocked ? 1 : 0;
        }
        bool on = m_category == c.id;

        auto body = CCNode::create();
        body->setContentSize({ kTabsW, kTabH });
        if (on) body->addChild(CCLayerColor::create({ 255, 255, 255, 45 }, kTabsW, kTabH));

        auto icon = categoryIcon(&c);
        icon->setScale(16.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
        icon->setPosition({ 14.f, kTabH / 2.f });
        body->addChild(icon);

        auto name = CCLabelBMFont::create(c.name, "bigFont.fnt");
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 26.f, kTabH / 2.f + 5.f });
        name->limitLabelWidth(kTabsW - 32.f, 0.34f, 0.1f);
        if (on) name->setColor({ 255, 220, 80 });
        body->addChild(name);

        auto count = CCLabelBMFont::create(fmt::format("{}/{}", done, total).c_str(), "bigFont.fnt");
        count->setAnchorPoint({ 0.f, 0.5f });
        count->setPosition({ 26.f, kTabH / 2.f - 7.f });
        count->setScale(0.24f);
        count->setOpacity(190);
        if (total > 0 && done == total) count->setColor({ 140, 255, 140 });
        body->addChild(count);

        m_tabs->addCell(body);
    }
    m_tabs->updateLayout(false);
    if (redraw) m_tabs->setScrollPos(scroll);
    else m_tabs->scrollToTop();
}

CCNode* AchievementsPopup::makeCell(achievements::Achievement const& a, float width) {
    auto cell = CCNode::create();
    cell->setContentSize({ width, kCellH });

    CCSprite* icon = nullptr;
    if (!a.unlocked) icon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
    if (!icon) {
        icon = categoryIcon(achievements::category(a.category));
        if (!a.unlocked) icon->setColor({ 90, 90, 90 });
    }
    icon->setScale(21.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
    icon->setPosition({ 24.f, kCellH / 2.f });
    cell->addChild(icon);

    auto title = CCLabelBMFont::create(a.title.c_str(), "goldFont.fnt");
    title->setAnchorPoint({ 0.f, 0.5f });
    title->setPosition({ 46.f, kCellH - 15.f });
    title->limitLabelWidth(width - 120.f, 0.5f, 0.2f);
    if (!a.unlocked) title->setColor({ 200, 200, 200 });
    cell->addChild(title);

    auto desc = CCLabelBMFont::create(a.description.c_str(), "chatFont.fnt");
    desc->setAnchorPoint({ 0.f, 0.5f });
    desc->setPosition({ 46.f, 15.f });
    desc->limitLabelWidth(width - 120.f, 0.55f, 0.25f);
    cell->addChild(desc);

    if (a.unlocked) {
        auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        check->setScale(0.6f);
        check->setPosition({ width - 30.f, kCellH / 2.f + 6.f });
        cell->addChild(check);
    } else {
        auto progress = CCLabelBMFont::create(fmt::format("{}/{}", a.progress, a.target).c_str(), "bigFont.fnt");
        progress->setPosition({ width - 30.f, kCellH / 2.f + 6.f });
        progress->limitLabelWidth(50.f, 0.3f, 0.15f);
        cell->addChild(progress);
    }
    if (a.gp > 0) {
        auto row = CCNode::create();
        row->setLayout(RowLayout::create()->setGap(2.f)->setAutoScale(false));
        row->setContentSize({ 50.f, 12.f });
        row->setAnchorPoint({ 0.5f, 0.5f });
        row->setPosition({ width - 30.f, kCellH / 2.f - 12.f });
        auto gp = CCLabelBMFont::create(fmt::format("+{}", a.gp).c_str(), "bigFont.fnt");
        gp->setScale(0.26f);
        gp->setColor(a.unlocked ? ccColor3B{ 140, 255, 140 } : ccColor3B{ 255, 220, 80 });
        row->addChild(gp);
        auto icon = CCSprite::create("grind_point_icon.png"_spr);
        if (icon) {
            icon->setScale(10.f / std::max(1.f, icon->getContentHeight()));
            row->addChild(icon);
        }
        row->updateLayout();
        cell->addChild(row);
    }
    return cell;
}

void AchievementsPopup::buildList() {
    m_list->clear();
    int done = 0;
    for (auto const& a : m_all) {
        done += a.unlocked ? 1 : 0;
        if (a.category != m_category) continue;
        m_list->addCell(this->makeCell(a, kListSize.width));
    }
    m_list->updateLayout(false);
    m_list->scrollToTop();
    handleTouchPriority(this);
    m_summary->setString(fmt::format("{} / {} unlocked", done, m_all.size()).c_str());
}

}
