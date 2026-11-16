#include "AchievementsPopup.hpp"
#include "../../managers/NotifyManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

constexpr CCSize kListSize { 300.f, 210.f };
constexpr float kCellH = 50.f;

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

    auto tabsBg = NineSlice::create("square02_small.png");
    tabsBg->setContentSize({ 100.f, kListSize.height });
    tabsBg->setOpacity(80);
    tabsBg->setPosition({ 64.f, centerY() - 12.f });
    m_mainLayer->addChild(tabsBg);

    m_tabs = CCMenu::create();
    m_tabs->setLayout(ColumnLayout::create()->setGap(4.f)->setAxisReverse(true)->setAutoScale(false)->setAxisAlignment(AxisAlignment::End));
    m_tabs->setContentSize({ 96.f, kListSize.height - 10.f });
    m_tabs->setPosition({ 64.f, centerY() - 12.f });
    m_mainLayer->addChild(m_tabs);

    m_list = cue::ListNode::create(kListSize);
    m_list->setAutoUpdate(false);
    m_list->setPosition({ 64.f + 50.f + 12.f + kListSize.width / 2.f, centerY() - 12.f });
    m_mainLayer->addChild(m_list);

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
    m_tabs->removeAllChildren();
    Ref<AchievementsPopup> self = this;
    for (auto const& c : achievements::categories()) {
        int done = 0, total = 0;
        for (auto const& a : m_all) {
            if (a.category != c.id) continue;
            total++;
            done += a.unlocked ? 1 : 0;
        }
        bool on = m_category == c.id;
        auto spr = ButtonSprite::create(c.name, 82, true, "bigFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 22.f, 0.4f);
        spr->setScale(0.85f);
        if (total > 0 && done == total) spr->setColor({ 160, 255, 160 });
        std::string id = c.id;
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, id](CCObject*) {
            if (!self || self->m_category == id) return;
            self->m_category = id;
            Loader::get()->queueInMainThread([self] {
                if (!self) return;
                self->buildTabs();
                self->buildList();
            });
        });
        m_tabs->addChild(btn);
    }
    m_tabs->updateLayout();
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
    m_summary->setString(fmt::format("{} / {} unlocked", done, m_all.size()).c_str());
}

NotificationsPopup* NotificationsPopup::create() {
    auto ret = new NotificationsPopup;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NotificationsPopup::init() {
    if (!BasePopup::init(320.f, 260.f)) return false;
    this->setTitle("Notifications");
    m_list = cue::ListNode::create({ 280.f, 190.f });
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(28.f);
    m_list->setPosition({ centerX(), centerY() - 10.f });
    m_mainLayer->addChild(m_list);
    this->build();
    return true;
}

void NotificationsPopup::build() {
    m_list->clear();
    Ref<NotificationsPopup> self = this;

    auto row = [&](std::string const& title, std::string const& desc, bool on, geode::Function<void(bool)> set, bool bold) {
        auto cell = CCMenu::create();
        cell->ignoreAnchorPointForPosition(false);
        cell->setContentSize({ 280.f, 28.f });
        auto label = CCLabelBMFont::create(title.c_str(), bold ? "goldFont.fnt" : "bigFont.fnt");
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ 10.f, 14.f });
        label->limitLabelWidth(190.f, bold ? 0.5f : 0.38f, 0.15f);
        cell->addChild(label);
        if (!desc.empty()) {
            auto info = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), [title, desc](CCObject*) {
                FLAlertLayer::create(title.c_str(), desc, "OK")->show();
            });
            info->setScale(0.45f);
            info->m_baseScale = 0.45f;
            info->setPosition({ 18.f + label->getScaledContentWidth(), 14.f });
            cell->addChild(info);
        }
        auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.6f, [set = std::move(set)](CCMenuItemToggler* t) mutable {
            set(!t->isToggled());
        });
        toggler->toggle(on);
        toggler->setPosition({ 280.f - 18.f, 14.f });
        cell->addChild(toggler);
        m_list->addCell(cell);
    };

    bool allOn = true;
    for (auto const& t : notify::types()) allOn = allOn && notify::enabled(t.type);
    row("All notifications", "", allOn, [self](bool on) {
        for (auto const& t : notify::types()) notify::setEnabled(t.type, on);
        Loader::get()->queueInMainThread([self] { if (self) self->build(); });
    }, true);
    for (auto const& t : notify::types()) {
        auto type = t.type;
        row(t.name, t.desc, notify::enabled(type), [self, type](bool on) {
            notify::setEnabled(type, on);
            Loader::get()->queueInMainThread([self] { if (self) self->build(); });
        }, false);
    }
    m_list->updateLayout(false);
    m_list->scrollToTop();
}

}
