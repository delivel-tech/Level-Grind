#include "../../managers/Auth.hpp"
#include "ClanPopups.hpp"
#include <Geode/ui/ColorPickPopup.hpp>
#include <Geode/ui/MDPopup.hpp>
#include <Geode/ui/Label.hpp>
#include "Geode/ui/Label.hpp"
#include "ReportPopups.hpp"
#include "../ClanUI.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include "../../managers/DataManager.hpp"
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <UIBuilder.hpp>
#include <ctime>
#include <fmt/format.h>
#include "GrindProfilePopup.hpp"
#include "../../managers/PetManager.hpp"

#include <UIBuilder.hpp>

namespace levelgrind {

using namespace clanui;
using M = GrindClansManager;

namespace {

constexpr int kMinGpSteps[] = { 0, 50, 100, 250, 500, 1000, 2500, 5000 };

int stepMinGp(int current, int dir) {
    int n = static_cast<int>(std::size(kMinGpSteps));
    int idx = 0;
    for (int i = 0; i < n; i++) {
        if (kMinGpSteps[i] <= current) idx = i;
    }
    idx = std::clamp(idx + dir, 0, n - 1);
    return kMinGpSteps[idx];
}

geode::Label* label(CCNode* parent, const char* text, CCPoint pos, float scale = 0.45f, const char* font = "goldFont.fnt", CCPoint anchor = { 0.f, 0.5f }) {
    auto l = geode::Label::create(text, font);
    l->setScale(scale);
    l->setAnchorPoint(anchor);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

CCMenuItemSpriteExtra* spriteButton(CCMenu* menu, const char* frame, float scale, CCPoint pos, geode::Function<void()> cb) {
    auto spr = CCSprite::createWithSpriteFrameName(frame);
    spr->setScale(scale);
    auto btn = CCMenuItemExt::createSpriteExtra(spr, [cb = std::move(cb)](CCObject*) mutable { cb(); });
    btn->setPosition(pos);
    menu->addChild(btn);
    return btn;
}

CCMenuItemSpriteExtra* nodeButton(CCMenu* menu, CCNode* node, CCPoint pos, geode::Function<void()> cb) {
    auto btn = CCMenuItemExt::createSpriteExtra(node, [cb = std::move(cb)](CCObject*) mutable { cb(); });
    btn->setPosition(pos);
    menu->addChild(btn);
    return btn;
}

std::vector<CCSprite*> buildSwatches(CCNode* parent, CCPoint pos, geode::Function<void(int)> onPick) {
    std::vector<CCSprite*> out;
    auto row = CCMenu::create();
    row->setLayout(RowLayout::create()->setGap(2.f)->setAutoScale(false));
    row->setContentSize({ 230.f, 22.f });
    row->setPosition(pos);
    parent->addChild(row, 5);

    auto pick = std::make_shared<geode::Function<void(int)>>(std::move(onPick));
    auto const& pal = M::palette();
    for (int i = 0; i < static_cast<int>(pal.size()); i++) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
        spr->setScale(0.4f);
        spr->setColor(pal[i].color);
        out.push_back(spr);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [pick, i](CCObject*) { (*pick)(i); });
        row->addChild(btn);
    }
    row->updateLayout();
    return out;
}

void highlightSwatches(std::vector<CCSprite*> const& swatches, int selected) {
    for (int i = 0; i < static_cast<int>(swatches.size()); i++) {
        bool on = i == selected;
        swatches[i]->setScale(on ? 0.46f : 0.38f);
        swatches[i]->setOpacity(on ? 255 : 150);
    }
}

std::string joinText(int joinType) {
    return joinType == 0 ? "Open" : joinType == 1 ? "By request" : "Invite only";
}

std::string minGpText(int minGp) {
    return minGp == 0 ? "Anyone" : fmt::format("{} GP", minGp);
}

}

EarnGPPopup* EarnGPPopup::create(GPWallet const& wallet) {
    auto ret = new EarnGPPopup;
    if (ret->init(wallet)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EarnGPPopup::init(GPWallet const& wallet) {
    if (!BasePopup::init(330.f, 265.f)) return false;
    this->setTitle("Grind Points");

    auto bal = gpAmount(wallet.balance, 0.9f, "bigFont.fnt");
    bal->setPosition({ centerX(), top() - 52.f });
    m_mainLayer->addChild(bal);
    label(m_mainLayer, fmt::format("{} earned in total", M::formatGP(wallet.lifetime)).c_str(), { centerX(), top() - 72.f }, 0.5f, "chatFont.fnt", { 0.5f, 0.5f })
        ->setOpacity(180);

    auto today = makePanel(m_mainLayer, { centerX(), top() - 102.f }, { 290.f, 34.f });
    label(today.node, "Today", { 10.f, 17.f }, 0.45f);
    auto bar = ProgressBar::create(ProgressBar::Style::Solid);
    bar->setFillColor({ 255, 210, 70 });
    bar->setScale(0.6f);
    bar->setAnchorPoint({ 0.5f, 0.5f });
    bar->setPosition({ 160.f, 17.f });
    float pct = wallet.dailyCap > 0 ? 100.f * wallet.todayEarned / wallet.dailyCap : 0.f;
    bar->updateProgress(std::clamp(pct, 0.f, 100.f));
    today.node->addChild(bar);
    label(today.node, fmt::format("{}/{}", wallet.todayEarned, wallet.dailyCap).c_str(), { 282.f, 17.f }, 0.5f, "chatFont.fnt", { 1.f, 0.5f });

    auto rules = makePanel(m_mainLayer, { centerX(), 72.f }, { 290.f, 118.f });
    std::string text =
        "<cy>Beat any rated level</c>: 1 GP per star/moon, once per level\n"
        "<cg>Level Grind levels</c>: 2 GP per star/moon\n"
        "<co>Live daily / weekly / monthly</c>: x2 / x3 / x4\n"
        "<cp>Finish a whole Grind Pack</c>: +50 GP\n"
        "<cl>Clan weekly tasks</c>: GP for everyone who helped\n"
        "<cj>Clan level</c>: up to +20% GP for members\n"
        "Up to 400 GP a day from levels.";
    auto area = richText(text, 270.f, 0.5f, { 0.f, 1.f });
    area->setPosition({ 10.f, 110.f });
    rules.node->addChild(area);

    return true;
}

namespace {

struct LookEditor {
    CCNode* holder = nullptr;
    std::vector<CCSprite*> swatches;
    geode::Label* joinLabel = nullptr;
    geode::Label* minLabel = nullptr;
};

geode::Label* stepperRow(CCNode* parent, CCPoint pos, std::string const& text, geode::Function<void(int)> onStep) {
    auto row = CCMenu::create();
    row->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));
    row->setContentSize({ 200.f, 24.f });
    row->setPosition(pos);
    parent->addChild(row, 5);

    auto step = std::make_shared<geode::Function<void(int)>>(std::move(onStep));
    auto left = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    left->setScale(0.6f);
    row->addChild(CCMenuItemExt::createSpriteExtra(left, [step](CCObject*) { (*step)(-1); }));

    auto box = CCNode::create();
    box->setContentSize({ 110.f, 20.f });
    auto value = geode::Label::create(text, "bigFont.fnt");
    value->setScale(0.42f);
    value->setPosition({ 55.f, 10.f });
    box->addChild(value);
    row->addChild(box);

    auto right = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    right->setScale(0.6f);
    row->addChild(CCMenuItemExt::createSpriteExtra(right, [step](CCObject*) { (*step)(1); }));

    row->updateLayout();
    return value;
}

class BadgeEditor : public BasePopup {
public:
    using Save = geode::Function<void(int, long long)>;
    static void open(int id, int colorId, long long colors, Save save) {
        auto popup = new BadgeEditor;
        if (popup->init(id, colorId, colors, std::move(save))) {
            popup->autorelease();
            popup->show();
        } else delete popup;
    }

private:
    ClanBadge badge{};
    ccColor3B baseColor{}, symbolColor{};
    CCSprite* swatches[2]{};
    CCNode* previews = nullptr;

    long long colors() const {
        return (static_cast<long long>(rgbValue(baseColor)) << 24) | rgbValue(symbolColor);
    }

    void refresh() {
        previews->removeAllChildren();
        auto draw = [&](EmblemPart part, float size, CCPoint pos) {
            auto image = emblem(badge.encode(), 0, size, colors(), part);
            image->setPosition(pos);
            previews->addChild(image);
        };
        draw(EmblemPart::Both, 88.f, {73.f, 108.f});
        draw(EmblemPart::Background, 46.f, {207.f, 125.f});
        draw(EmblemPart::Symbol, 68.f, {207.f, 69.f});
        swatches[0]->setColor(baseColor);
        swatches[1]->setColor(symbolColor);
    }

    void pickColor(bool symbol) {
        auto popup = ColorPickPopup::create(symbol ? symbolColor : baseColor);
        if (!popup) return;
        Ref<BadgeEditor> self = this;
        popup->setCallback([self, symbol](ccColor4B const& c) {
            (symbol ? self->symbolColor : self->baseColor) = {c.r, c.g, c.b};
            self->refresh();
        });
        popup->show();
    }

    bool init(int id, int colorId, long long savedColors, Save save) {
        if (!BasePopup::init(300.f, 185.f, "GJ_square01.png")) return false;
        setTitle("Change Emblem");
        badge = ClanBadge::decode(id);
        baseColor = savedColors < 0 ? M::clanColor(colorId) : badgeColor(savedColors >> 24);
        symbolColor = savedColors < 0 ? ClanBadge::inks[badge.ink] : badgeColor(savedColors);
        auto originalColors = colors();
        previews = CCNode::create();
        m_mainLayer->addChild(previews);
        for (int row = 0; row < 2; ++row) {
            bool symbol = row == 1;
            for (int dir : {-1, 1}) {
                auto arrow = CCSprite::createWithSpriteFrameName(dir < 0 ? "edit_leftBtn_001.png" : "edit_rightBtn_001.png");
                arrow->setScale(0.7f);
                nodeButton(m_buttonMenu, arrow, {207.f + dir * 43.f, symbol ? 69.f : 125.f}, [this, symbol, dir] {
                    int& part = symbol ? badge.symbol : badge.background;
                    int count = symbol ? 7 : 10;
                    part = (part - 1 + dir + count) % count + 1;
                    refresh();
                });
            }
            auto swatch = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
            swatch->setScale(0.65f);
            swatches[row] = swatch;
            nodeButton(m_buttonMenu, swatch, {56.f + row * 34.f, 50.f}, [this, symbol] { pickColor(symbol); });
        }
        nodeButton(m_buttonMenu, textButton("OK", "GJ_button_01.png", 0.85f), {centerX(), 21.f},
            [this, id, savedColors, originalColors, save = std::move(save)]() mutable {
                int selected = badge.encode();
                save(selected == ClanBadge::decode(id).encode() ? id : selected,
                    colors() == originalColors ? savedColors : colors());
                onClose(nullptr);
            });
        refresh();
        return true;
    }
};

LookEditor buildLookEditor(
    BasePopup* owner, CCNode* parent, CCPoint emblemPos, float labelX, float controlX, float topY,
    int* color, int* emblemId, long long* colors, int* joinType, int* minGp, geode::Function<void()> onRefresh
) {
    LookEditor e;
    auto refresh = std::make_shared<geode::Function<void()>>(std::move(onRefresh));

    auto emblemRow = CCMenu::create();
    emblemRow->setPosition(emblemPos);
    parent->addChild(emblemRow, 5);
    e.holder = CCNode::create();
    e.holder->setContentSize({44.f, 44.f});
    nodeButton(emblemRow, e.holder, {0.f, 0.f}, [owner, color, emblemId, colors, refresh] {
        BadgeEditor::open(*emblemId, *color, *colors, [editor = Ref<BasePopup>(owner), colors, emblemId, refresh](int id, long long tint) {
            *emblemId = id;
            *colors = tint;
            (*refresh)();
        });
    });
    label(parent, "Edit Emblem", emblemPos + CCPoint{0.f, 28.f}, 0.42f, "goldFont.fnt", {0.5f, 0.5f});

    float const gap = 30.f;

    label(parent, "Clan color", { labelX, topY }, 0.45f);
    e.swatches = buildSwatches(parent, { controlX, topY }, [color, refresh](int i) {
        *color = i;
        (*refresh)();
    });

    float const stepX = controlX + 18.f;

    label(parent, "Joining", { labelX, topY - gap }, 0.45f);
    e.joinLabel = stepperRow(parent, { stepX, topY - gap }, joinText(*joinType), [joinType, refresh](int) {
        *joinType = (*joinType + 1) % 3;
        (*refresh)();
    });

    label(parent, "Min. GP to join", { labelX, topY - gap * 2.f }, 0.45f);
    e.minLabel = stepperRow(parent, { stepX, topY - gap * 2.f }, minGpText(*minGp), [minGp, refresh](int dir) {
        *minGp = stepMinGp(*minGp, dir);
        (*refresh)();
    });

    return e;
}

void refreshLookEditor(LookEditor& e, int color, int emblemId, long long colors, int joinType, int minGp) {
    e.holder->removeAllChildren();
    auto em = emblem(emblemId, color, 36.f, colors);
    em->setPosition(e.holder->getContentSize() / 2.f);
    e.holder->addChild(em);
    highlightSwatches(e.swatches, color);
    e.joinLabel->setString(joinText(joinType).c_str());
    e.joinLabel->setLimitLabelWidth(90.f, 0.45f, 0.2f);
    e.minLabel->setString(minGpText(minGp).c_str());
    e.minLabel->setLimitLabelWidth(90.f, 0.45f, 0.2f);
}

}

CreateClanPopup* CreateClanPopup::create(int cost, GPWallet const& wallet, ClanChanged onCreated) {
    auto ret = new CreateClanPopup;
    if (ret->init(cost, wallet, std::move(onCreated))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CreateClanPopup::init(int cost, GPWallet const& wallet, ClanChanged onCreated) {
    if (!BasePopup::init(400.f, 280.f)) return false;
    m_cost = cost;
    m_onCreated = std::move(onCreated);
    m_emblem = 1;
    m_badgeColors = 0xffffffffffffLL;
    this->setTitle("Create a Grind Clan");

    CCSize const ps { 370.f, 180.f };
    auto panel = makePanel(m_mainLayer, { centerX(), centerY() + 14.f }, ps);
    auto p = panel.node;

    m_name = TextInput::create(260.f, "Clan name (3-20)", "bigFont.fnt");
    m_name->setScale(0.8f);
    m_name->setMaxCharCount(20);
    m_name->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_.");
    m_name->setPosition({ 250.f, ps.height - 24.f });
    p->addChild(m_name);

    m_desc = TextInput::create(260.f, "Short description (optional)", "chatFont.fnt");
    m_desc->setScale(0.8f);
    m_desc->setMaxCharCount(120);
    m_desc->setCommonFilter(CommonFilter::Any);
    m_desc->setPosition({ 250.f, ps.height - 52.f });
    p->addChild(m_desc);

    auto line = divider(ps.width - 20.f);
    line->setPosition({ 10.f, ps.height - 72.f });
    p->addChild(line);

    auto editor = buildLookEditor(
        this, p, { 62.f, ps.height - 44.f }, 16.f, 250.f, ps.height - 92.f,
        &m_color, &m_emblem, &m_badgeColors, &m_joinType, &m_minGp, [this] { this->refreshLook(); }
    );
    m_previewHolder = editor.holder;
    m_swatches = editor.swatches;
    m_joinLabel = editor.joinLabel;
    m_minLabel = editor.minLabel;

    auto costRow = CCNode::create();
    costRow->setLayout(RowLayout::create()->setGap(5.f)->setAutoScale(false));
    costRow->setContentSize({ 300.f, 20.f });
    costRow->setAnchorPoint({ 0.5f, 0.5f });
    costRow->setPosition({ centerX(), 46.f });
    auto costLbl = geode::Label::create("Cost:", "goldFont.fnt");
    costLbl->setScale(0.5f);
    costRow->addChild(costLbl);
    costRow->addChild(gpAmount(cost, 0.5f));
    auto have = geode::Label::create(fmt::format("(you have {})", M::formatGP(wallet.balance)).c_str(), "chatFont.fnt");
    have->setScale(0.5f);
    have->setColor(wallet.balance >= cost ? ccColor3B{ 140, 255, 140 } : ccColor3B{ 255, 120, 120 });
    costRow->addChild(have);
    costRow->updateLayout();
    m_mainLayer->addChild(costRow);

    auto btn = CCMenuItemExt::createSpriteExtra(textButton("Create", "GJ_button_01.png", 0.75f), [this](CCObject*) { this->onCreate(); });
    btn->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 20.f });
    m_buttonMenu->addChild(btn);

    this->refreshLook();
    return true;
}

void CreateClanPopup::refreshLook() {
    LookEditor e { m_previewHolder, m_swatches, m_joinLabel, m_minLabel };
    refreshLookEditor(e, m_color, m_emblem, m_badgeColors, m_joinType, m_minGp);
}

void CreateClanPopup::onCreate() {
    std::string name = m_name->getString();
    while (!name.empty() && name.back() == ' ') name.pop_back();
    while (!name.empty() && name.front() == ' ') name.erase(name.begin());
    if (name.size() < 3) {
        Notification::create("The name needs at least 3 characters", NotificationIcon::Error)->show();
        return;
    }

    auto body = M::authBody();
    body["name"] = name;
    body["description"] = m_desc->getString();
    body["colorId"] = m_color;
    body["emblemId"] = m_emblem;
    body["badgeColors"] = static_cast<int64_t>(m_badgeColors);
    body["joinType"] = m_joinType;
    body["minGp"] = m_minGp;

    Ref<CreateClanPopup> self = this;
    M::getInstance().runAction("clan_create", body, "Creating clan...", "Success! Your clan is ready.", [self](matjson::Value const&) {
        if (!self) return;
        auto cb = std::move(self->m_onCreated);
        self->onClose(nullptr);
        if (cb) cb();
    });
}

ClanSettingsPopup* ClanSettingsPopup::create(ClanInfo const& clan, ClanChanged onChanged) {
    auto ret = new ClanSettingsPopup;
    if (ret->init(clan, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanSettingsPopup::init(ClanInfo const& clan, ClanChanged onChanged) {
    if (!BasePopup::init(400.f, 280.f)) return false;
    m_clan = clan;
    m_onChanged = std::move(onChanged);
    m_color = clan.colorId;
    m_emblem = clan.emblemId;
    m_badgeColors = clan.badgeColors;
    if (m_badgeColors < 0) {
        auto symbol = ClanBadge::inks[ClanBadge::decode(clan.emblemId).ink];
        m_badgeColors = (static_cast<long long>(rgbValue(M::clanColor(clan.colorId))) << 24) | rgbValue(symbol);
    }
    m_initialBadgeColors = m_badgeColors;
    m_joinType = clan.joinType;
    m_minGp = clan.minGp;
    this->setTitle("Clan Settings");

    CCSize const ps { 370.f, 180.f };
    auto panel = makePanel(m_mainLayer, { centerX(), centerY() + 14.f }, ps);
    auto p = panel.node;

    auto name = label(p, clan.name.c_str(), { 250.f, ps.height - 24.f }, 0.6f, "bigFont.fnt", { 0.5f, 0.5f });
    name->setLimitLabelWidth(240.f, 0.6f, 0.2f);
    m_clanName = name;
    name->setColor(M::clanColor(clan.colorId));

    m_desc = TextInput::create(260.f, "Short description", "chatFont.fnt");
    m_desc->setScale(0.8f);
    m_desc->setMaxCharCount(120);
    m_desc->setCommonFilter(CommonFilter::Any);
    m_desc->setString(clan.description);
    m_desc->setPosition({ 250.f, ps.height - 52.f });
    p->addChild(m_desc);

    auto line = divider(ps.width - 20.f);
    line->setPosition({ 10.f, ps.height - 72.f });
    p->addChild(line);

    auto editor = buildLookEditor(
        this, p, { 62.f, ps.height - 44.f }, 16.f, 250.f, ps.height - 92.f,
        &m_color, &m_emblem, &m_badgeColors, &m_joinType, &m_minGp, [this] { this->refreshLook(); }
    );
    m_previewHolder = editor.holder;
    m_swatches = editor.swatches;
    m_joinLabel = editor.joinLabel;
    m_minLabel = editor.minLabel;

    m_costLabel = label(m_mainLayer, "", { centerX(), 46.f }, 0.5f, "chatFont.fnt", { 0.5f, 0.5f });

    auto buttons = CCMenu::create();
    buttons->setLayout(RowLayout::create()->setGap(14.f)->setAutoScale(false));
    buttons->setContentSize({ 300.f, 30.f });
    buttons->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 20.f });
    m_buttonMenu->addChild(buttons);

    Ref<ClanSettingsPopup> self = this;
    auto disband = CCMenuItemExt::createSpriteExtra(textButton("Disband", "GJ_button_06.png", 0.6f), [self](CCObject*) {
        createQuickPopup(
            "Disband clan?",
            fmt::format("<cr>{}</c> will be deleted for every member. This can't be undone.", self->m_clan.name),
            "Cancel", "Disband",
            [self](auto, bool yes) {
                if (!yes || !self) return;
                M::getInstance().runAction("clan_disband", M::authBody(), "Disbanding...", "The clan was disbanded.", [self](matjson::Value const&) {
                    if (!self) return;
                    auto cb = std::move(self->m_onChanged);
                    self->onClose(nullptr);
                    if (cb) cb();
                });
            }
        );
    });
    buttons->addChild(disband);

    auto save = CCMenuItemExt::createSpriteExtra(textButton("Save", "GJ_button_01.png", 0.7f), [this](CCObject*) { this->onSave(); });
    buttons->addChild(save);
    buttons->updateLayout();

    this->refreshLook();
    return true;
}

void ClanSettingsPopup::refreshLook() {
    m_clanName->setColor(M::clanColor(m_color));
    LookEditor e { m_previewHolder, m_swatches, m_joinLabel, m_minLabel };
    refreshLookEditor(e, m_color, m_emblem, m_badgeColors, m_joinType, m_minGp);
    bool newLook = m_color != m_clan.colorId || m_emblem != m_clan.emblemId || m_badgeColors != m_initialBadgeColors;
    if (m_costLabel) {
        m_costLabel->setString(newLook ? "A new color or emblem costs 50 GP" : "Description and joining are free to change");
        m_costLabel->setColor(newLook ? ccColor3B{ 255, 210, 90 } : ccColor3B{ 200, 200, 200 });
    }
}

void ClanSettingsPopup::onSave() {
    auto body = M::authBody();
    body["description"] = m_desc->getString();
    body["colorId"] = m_color;
    body["emblemId"] = m_emblem;
    bool sameLook = m_color == m_clan.colorId && m_emblem == m_clan.emblemId && m_badgeColors == m_initialBadgeColors;
    body["badgeColors"] = static_cast<int64_t>(sameLook ? m_clan.badgeColors : m_badgeColors);
    body["joinType"] = m_joinType;
    body["minGp"] = m_minGp;

    Ref<ClanSettingsPopup> self = this;
    M::getInstance().runAction("clan_edit", body, "Saving...", "Success! Clan updated.", [self](matjson::Value const&) {
        if (!self) return;
        auto cb = std::move(self->m_onChanged);
        self->onClose(nullptr);
        if (cb) cb();
    });
}

ClanViewPopup* ClanViewPopup::create(int clanId, geode::Function<void(ClanInfo const&)> onJoin) {
    auto ret = new ClanViewPopup;
    if (ret->init(clanId, std::move(onJoin))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanViewPopup::init(int clanId, geode::Function<void(ClanInfo const&)> onJoin) {
    if (!BasePopup::init(400.f, 280.f)) return false;
    m_onJoin = std::move(onJoin);
    this->setTitle("Grind Clan");

    auto spinner = LoadingSpinner::create(40.f);
    spinner->setPosition(center());
    spinner->setID("loading");
    m_mainLayer->addChild(spinner);

    matjson::Value body;
    body["clanId"] = clanId;
    Ref<ClanViewPopup> self = this;
    m_task.spawn(M::post("clan_view", body), [self](web::WebResponse res) {
        if (!self) return;
        if (auto s = self->m_mainLayer->getChildByID("loading")) s->removeFromParent();
        std::string err;
        if (!M::parseOk(res, err)) {
            Notification::create(err, NotificationIcon::Error)->show();
            return;
        }
        self->build(M::parseDetails(res.json().unwrap()));
    });
    return true;
}

void ClanViewPopup::build(ClanDetails const& d) {
    auto const& c = d.info;
    this->setTitle(c.name.c_str());

    auto head = makePanel(m_mainLayer, { centerX(), top() - 70.f }, { 370.f, 64.f });
    auto em = emblem(c.emblemId, c.colorId, 40.f, c.badgeColors);
    em->setPosition({ 32.f, 32.f });
    head.node->addChild(em);

    label(head.node, fmt::format("Level {}  {}/{} members{}", c.level, c.members, c.maxMembers, c.featured ? "  - Featured" : "").c_str(),
        { 62.f, 46.f }, 0.45f);
    auto desc = label(head.node, c.description.empty() ? "No description" : c.description.c_str(), { 62.f, 28.f }, 0.5f, "chatFont.fnt");
    desc->setLimitLabelWidth(290.f, 0.5f, 0.2f);
    desc->setOpacity(c.description.empty() ? 120 : 220);

    auto week = gpAmount(c.weekXp, 0.4f);
    week->setAnchorPoint({ 0.f, 0.5f });
    week->setPosition({ 62.f, 12.f });
    head.node->addChild(week);
    auto stats = label(head.node, fmt::format("this week (#{})   {} all time   +{}%", d.weekRank, M::formatGP(c.xp), c.gpBonus).c_str(),
        { 62.f + week->getContentWidth() + 4.f, 12.f }, 0.45f, "chatFont.fnt");
    stats->setOpacity(200);
    auto bonusIcon = gpIcon(0.42f, 1.f);
    bonusIcon->setAnchorPoint({ 0.f, 0.5f });
    bonusIcon->setPosition({ stats->getPositionX() + stats->getScaledContentWidth() + 2.f, 12.f });
    head.node->addChild(bonusIcon);

    auto list = cue::ListNode::create({ 370.f, 120.f });
    list->setAutoUpdate(false);
    list->setCellHeight(26.f);
    list->setPosition({ centerX(), centerY() - 30.f });
    m_mainLayer->addChild(list);

    for (auto const& m : d.members) {
        auto row = CCNode::create();
        row->setContentSize({ 370.f, 26.f });
        auto ic = playerIcon(m.icon, m.color1, m.color2, m.color3, 0.5f);
        ic->setPosition({ 18.f, 13.f });
        row->addChild(ic);

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setContentSize(row->getContentSize());
        row->addChild(menu);

        auto name = Build(geode::Label::create(m.username, "bigFont.fnt"))
            .scale(0.5f)
            .with([](geode::Label* l) {
                l->setLimitLabelWidth(140, 0.5f, 0.2f);
            })
            .color(M::roleColor(m.role))
            .intoMenuItem([m] { GrindProfilePopup::create(m.accountId)->show(); })
            .anchorPoint({ 0.f, 0.5f })
            .pos(36, 13)
            .parent(menu)
            .collect();

        if (m.role > 0) {
            label(row, M::roleName(m.role).c_str(), { 182.f, 13.f }, 0.45f, "chatFont.fnt")->setColor(M::roleColor(m.role));
        }
        if (m.hasPet) {
            auto pet = PetManager::createMiniPet(m.petLevel, m.petAppearance, m.icon, m.color1, m.color2, m.color3);
            pet->setScale(0.32f);
            pet->setPosition({ 250.f, 12.f });
            row->addChild(pet);
        }
        auto gp = gpAmount(m.contributed, 0.4f);
        gp->setAnchorPoint({ 1.f, 0.5f });
        gp->setPosition({ 360.f, 13.f });
        row->addChild(gp);
        list->addCell(row);
    }
    list->updateLayout(false);
    list->scrollToTop();

    auto flag = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
    flag->setScale(0.55f);
    auto flagBtn = CCMenuItemExt::createSpriteExtra(flag, [c](CCObject*) {
        ReportPopup::create(ReportTarget::Clan, c.id, c.name)->show();
    });
    flagBtn->setPosition({ m_buttonMenu->getContentWidth() - 26.f, 22.f });
    m_buttonMenu->addChild(flagBtn);

    if (m_onJoin) {
        bool full = c.members >= c.maxMembers;
        bool closed = c.joinType == 2;
        auto txt = full ? "Full" : closed ? "Invite only" : (c.joinType == 0 ? "Join" : "Request");
        auto btn = CCMenuItemExt::createSpriteExtra(textButton(txt, full || closed ? "GJ_button_04.png" : "GJ_button_01.png", 0.7f),
            [this, c, closed](CCObject*) {
                if (closed) {
                    Notification::create("This clan is invite only", NotificationIcon::Info)->show();
                    return;
                }
                auto cb = std::move(m_onJoin);
                this->onClose(nullptr);
                if (cb) cb(c);
            });
        btn->setEnabled(!full);
        btn->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 22.f });
        m_buttonMenu->addChild(btn);
    }
}

namespace {

class MemberActionsPopup : public BasePopup {
public:
    static MemberActionsPopup* create(ClanMemberInfo const& m, int myRole, ClanChanged onChanged) {
        auto ret = new MemberActionsPopup;
        if (ret->init(m, myRole, std::move(onChanged))) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    bool init(ClanMemberInfo const& m, int myRole, ClanChanged onChanged) {
        if (!BasePopup::init(260.f, 200.f)) return false;
        this->setTitle(m.username.c_str());

        auto ic = playerIcon(m.icon, m.color1, m.color2, m.color3, 0.8f);
        ic->setPosition({ centerX(), top() - 55.f });
        m_mainLayer->addChild(ic);
        label(m_mainLayer, M::roleName(m.role).c_str(), { centerX(), top() - 80.f }, 0.5f, "goldFont.fnt", { 0.5f, 0.5f })
            ->setColor(M::roleColor(m.role));

        auto menu = CCMenu::create();
        menu->setLayout(ColumnLayout::create()->setGap(6.f)->setAxisReverse(true));
        menu->setContentSize({ 200.f, 110.f });
        menu->setPosition({ centerX(), 60.f });
        m_mainLayer->addChild(menu);

        Ref<MemberActionsPopup> self = this;
        auto changed = std::make_shared<ClanChanged>(std::move(onChanged));
        auto act = [self, m, changed](std::string endpoint, int role, std::string loading, std::string done) {
            auto body = M::authBody();
            body["targetId"] = m.accountId;
            if (role >= 0) body["role"] = role;
            M::getInstance().runAction(endpoint, body, loading, done, [self, changed](matjson::Value const&) {
                if (self) self->onClose(nullptr);
                if (*changed) (*changed)();
            });
        };

        auto add = [&](const char* text, const char* bg, geode::Function<void()> cb) {
            auto btn = CCMenuItemExt::createSpriteExtra(textButton(text, bg, 0.6f), [cb = std::move(cb)](CCObject*) mutable { cb(); });
            menu->addChild(btn);
        };

        add("View Profile", "GJ_button_04.png", [m] { GrindProfilePopup::create(m.accountId)->show(); });

        if (myRole == 2) {
            if (m.role == 0) {
                add("Make Officer", "GJ_button_01.png", [act] {
                    act("clan_set_role", 1, "Promoting...", "Success! They are an Officer now.");
                });
            } else if (m.role == 1) {
                add("Make Member", "GJ_button_04.png", [act] {
                    act("clan_set_role", 0, "Updating...", "Success! They are a Member now.");
                });
            }
            add("Make Leader", "GJ_button_02.png", [act, m] {
                createQuickPopup("Hand over the clan?",
                    fmt::format("<cy>{}</c> becomes the Leader and you become an Officer.", m.username),
                    "Cancel", "Hand over", [act](auto, bool yes) {
                        if (!yes) return;
                        act("clan_set_role", 2, "Handing over...", "Success! New leader set.");
                    });
            });
        }
        if (myRole > m.role) {
            add("Remove", "GJ_button_06.png", [act, m] {
                createQuickPopup("Remove member?",
                    fmt::format("Remove <cr>{}</c> from the clan? Their Grind Points stop counting for it.", m.username),
                    "Cancel", "Remove", [act](auto, bool yes) {
                        if (yes) act("clan_kick", -1, "Removing...", "Done! They were removed.");
                    });
            });
        }
        menu->updateLayout();
        return true;
    }
};

}

ClanMembersPopup* ClanMembersPopup::create(ClanDetails const& d, ClanChanged onChanged) {
    auto ret = new ClanMembersPopup;
    if (ret->init(d, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanMembersPopup::init(ClanDetails const& d, ClanChanged onChanged) {
    if (!BasePopup::init(400.f, 270.f)) return false;
    m_details = d;
    m_onChanged = std::move(onChanged);
    this->setTitle(fmt::format("Members ({}/{})", d.info.members, d.info.maxMembers).c_str());

    label(m_mainLayer, "This week", { 15.f + 270.f, top() - 46.f }, 0.4f, "goldFont.fnt", { 0.5f, 0.5f })->setOpacity(200);
    label(m_mainLayer, "In clan", { 15.f + 325.f, top() - 46.f }, 0.4f, "goldFont.fnt", { 0.5f, 0.5f })->setOpacity(200);

    auto list = cue::ListNode::create({ 370.f, 190.f });
    list->setAutoUpdate(false);
    list->setCellHeight(30.f);
    list->setPosition({ centerX(), centerY() - 18.f });
    m_mainLayer->addChild(list);

    int myId = GJAccountManager::get()->m_accountID;
    long long now = static_cast<long long>(time(nullptr));

    for (auto const& m : d.members) {
        auto row = CCNode::create();
        row->setContentSize({ 370.f, 30.f });

        auto menu = CCMenu::create();
        menu->setContentSize(row->getContentSize());
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint({ 0.f, 0.f });
        menu->setPosition({ 0.f, 0.f });
        row->addChild(menu);

        auto ic = playerIcon(m.icon, m.color1, m.color2, m.color3, 0.55f);
        ic->setPosition({ 18.f, 15.f });
        row->addChild(ic);

        auto nameNode = CCNode::create();
        auto name = label(nameNode, m.username.c_str(), { 0.f, 0.f }, 0.5f, "bigFont.fnt");
        name->setLimitLabelWidth(130.f, 0.5f, 0.2f);
        name->setColor(M::roleColor(m.role));
        nameNode->setContentSize({ name->getScaledContentWidth(), 16.f });
        name->setPosition({ 0.f, 8.f });

        bool canManage = d.myRole > m.role && m.accountId != myId;
        auto details = d;
        Ref<ClanMembersPopup> self = this;
        auto btn = nodeButton(menu, nameNode, { 36.f + nameNode->getContentWidth() / 2.f, 19.f }, [self, m, details, canManage] {
            if (canManage) {
                MemberActionsPopup::create(m, details.myRole, [self] {
                    if (!self) return;
                    auto cb = std::move(self->m_onChanged);
                    self->onClose(nullptr);
                    if (cb) cb();
                })->show();
            } else {
                GrindProfilePopup::create(m.accountId)->show();
            }
        });
        (void)btn;

        std::string sub = m.role > 0 ? M::roleName(m.role) + " - " : "";
        sub += "active " + M::timeAgo(m.lastActive, now) + (M::timeAgo(m.lastActive, now) == "now" ? "" : " ago");
        label(row, sub.c_str(), { 36.f, 7.f }, 0.4f, "chatFont.fnt")->setOpacity(170);

        auto wk = gpAmount(m.weekContributed, 0.42f);
        wk->setPosition({ 270.f, 15.f });
        row->addChild(wk);
        auto all = gpAmount(m.contributed, 0.42f);
        all->setPosition({ 325.f, 15.f });
        row->addChild(all);

        list->addCell(row);
    }
    list->updateLayout(false);
    list->scrollToTop();

    if (d.myRole > 0) {
        label(m_mainLayer, "Tap a member's name to manage them", { centerX(), 14.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f })->setOpacity(190);
    }
    return true;
}


ClanTasksPopup* ClanTasksPopup::create(ClanDetails const& d, long long weekEndsAt, long long now) {
    auto ret = new ClanTasksPopup;
    if (ret->init(d, weekEndsAt, now)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanTasksPopup::init(ClanDetails const& d, long long weekEndsAt, long long now) {
    if (!BasePopup::init(380.f, 290.f)) return false;
    m_details = d;
    this->setTitle("Weekly Clan Tasks");

    label(m_mainLayer, fmt::format("New tasks in {}", countdown(weekEndsAt - now)).c_str(),
        { centerX(), top() - 42.f }, 0.5f, "chatFont.fnt", { 0.5f, 0.5f })->setColor({ 255, 220, 120 });

    m_tierLabel = label(m_mainLayer, "", { centerX(), top() - 62.f }, 0.4f, "bigFont.fnt", { 0.5f, 0.5f });
    int const tiers = M::tierCount(d.tasks);
    if (tiers > 1) {
        Ref<ClanTasksPopup> self = this;
        for (int dir : { -1, 1 }) {
            auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
            spr->setScale(0.4f);
            spr->setFlipX(dir > 0);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [self, dir, tiers](CCObject*) {
                if (!self) return;
                self->m_tier = (self->m_tier + dir + tiers) % tiers;
                self->buildCards();
            });
            btn->setPosition({ m_buttonMenu->getContentWidth() / 2.f + dir * 70.f, top() - 62.f });
            m_buttonMenu->addChild(btn);
        }
    }

    m_cards = CCNode::create();
    m_mainLayer->addChild(m_cards);
    this->buildCards();

    label(m_mainLayer, "Everyone who earned GP this week gets the reward", { centerX(), 26.f }, 0.28f, "bigFont.fnt", { 0.5f, 0.5f })
        ->setLimitLabelWidth(340.f, 0.28f, 0.12f);
    label(m_mainLayer, "Task GP counts for the clan too", { centerX(), 13.f }, 0.28f, "bigFont.fnt", { 0.5f, 0.5f })
        ->setColor({ 255, 230, 90 });
    return true;
}

void ClanTasksPopup::buildCards() {
    m_cards->removeAllChildren();
    m_tierLabel->setString(M::tierName(m_tier));
    m_tierLabel->setColor(M::tierColor(m_tier));

    float y = top() - 98.f;
    for (auto const& t : m_details.tasks) {
        if (t.tier != m_tier) continue;
        auto card = makePanel(m_cards, { centerX(), y }, { 340.f, 52.f }, t.completed ? 110 : 80);

        CCSprite* icon = M::taskIcon(t.type) ? CCSprite::createWithSpriteFrameName(M::taskIcon(t.type)) : gpIcon(1.f);
        if (icon) {
            icon->setScale(22.f / std::max(icon->getContentWidth(), icon->getContentHeight()));
            icon->setPosition({ 24.f, 32.f });
            card.node->addChild(icon);
        }

        auto title = label(card.node, M::taskTitle(t).c_str(), { 46.f, 37.f }, 0.48f);
        title->setLimitLabelWidth(200.f, 0.48f, 0.2f);
        if (t.completed) title->setColor({ 140, 255, 140 });

        auto bar = ProgressBar::create(ProgressBar::Style::Solid);
        bar->setFillColor(t.completed ? ccColor3B{ 90, 230, 90 } : ccColor3B{ 255, 200, 60 });
        bar->setScale(0.45f);
        bar->setAnchorPoint({ 0.f, 0.5f });
        bar->setPosition({ 46.f, 16.f });
        bar->updateProgress(std::clamp(100.f * t.progress / t.target, 0.f, 100.f));
        card.node->addChild(bar);
        label(card.node, fmt::format("{}/{}", t.progress, t.target).c_str(), { 24.f, 12.f }, 0.42f, "chatFont.fnt", { 0.5f, 0.5f })
            ->setLimitLabelWidth(40.f, 0.42f, 0.15f);

        auto reward = gpAmount(t.rewardGp, 0.5f);
        reward->setPosition({ 300.f, 34.f });
        card.node->addChild(reward);
        label(card.node, t.completed ? "Done!" : "each", { 300.f, 15.f }, 0.42f, "chatFont.fnt", { 0.5f, 0.5f })
            ->setColor(t.completed ? ccColor3B{ 140, 255, 140 } : ccColor3B{ 220, 220, 220 });

        if (t.completed) {
            auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            check->setScale(0.6f);
            check->setPosition({ 330.f, 44.f });
            card.node->addChild(check);
        }
        y -= 58.f;
    }
}

ClanLeaderboardPopup* ClanLeaderboardPopup::create(int myClanId) {
    auto ret = new ClanLeaderboardPopup;
    if (ret->init(myClanId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanLeaderboardPopup::init(int myClanId) {
    if (!BasePopup::init(400.f, 280.f)) return false;
    m_myClanId = myClanId;
    this->setTitle("Clan Leaderboards");

    auto tabs = CCMenu::create();
    tabs->setPosition({ centerX(), top() - 46.f });
    tabs->setContentSize({ 0.f, 0.f });
    m_mainLayer->addChild(tabs);

    Ref<ClanLeaderboardPopup> self = this;
    auto addTab = [&](const char* text, std::string sort, float x) {
        auto spr = ButtonSprite::create(text, 80, true, "goldFont.fnt", "GJ_button_04.png", 26.f, 0.6f);
        spr->setScale(0.8f);
        m_tabSprites.push_back(spr);
        nodeButton(tabs, spr, { x, 0.f }, [self, sort] {
            if (!self || self->m_sort == sort) return;
            self->m_sort = sort;
            self->m_page = 0;
            self->load();
        });
    };
    addTab("This Week", "week", -48.f);
    addTab("All Time", "all", 48.f);

    m_list = cue::ListNode::create({ 340.f, 180.f });
    m_list->setAutoUpdate(false);
    m_list->setCellHeight(30.f);
    m_list->setPosition({ centerX(), centerY() - 16.f });
    m_mainLayer->addChild(m_list);

    spriteButton(m_buttonMenu, "GJ_arrow_03_001.png", 0.5f, { 15.f, centerY() - 16.f }, [self] {
        if (self && self->m_page > 0) { self->m_page--; self->load(); }
    });
    auto next = spriteButton(m_buttonMenu, "GJ_arrow_03_001.png", 0.5f, { right() - 15.f, centerY() - 16.f }, [self] {
        if (self && self->m_hasMore) { self->m_page++; self->load(); }
    });
    static_cast<CCSprite*>(next->getNormalImage())->setFlipX(true);

    m_pageLabel = label(m_mainLayer, "", { centerX(), 14.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f });
    this->load();
    return true;
}

void ClanLeaderboardPopup::load() {
    for (size_t i = 0; i < m_tabSprites.size(); i++) {
        bool on = (i == 0) == (m_sort == "week");
        static_cast<ButtonSprite*>(m_tabSprites[i])->updateBGImage(on ? "GJ_button_02.png" : "GJ_button_04.png");
    }
    m_list->clear();
    m_list->updateLayout(false);
    m_pageLabel->setString("Loading...");

    matjson::Value body;
    body["sort"] = m_sort;
    body["page"] = m_page;
    Ref<ClanLeaderboardPopup> self = this;
    m_task.spawn(M::post("clan_browse", body), [self](web::WebResponse res) {
        if (!self) return;
        auto b = M::parseBrowse(res);
        if (!b.ok) {
            self->m_pageLabel->setString(b.error.c_str());
            return;
        }
        self->m_hasMore = b.hasMore;
        for (auto const& c : b.clans) {
            auto row = CCNode::create();
            row->setContentSize({ 340.f, 30.f });
            if (c.id == self->m_myClanId) {
                auto hl = CCLayerColor::create({ 120, 255, 120, 40 }, 340.f, 30.f);
                row->addChild(hl, -1);
            }
            ccColor3B rankColor = c.rank == 1 ? ccColor3B{ 255, 210, 60 } : c.rank == 2 ? ccColor3B{ 210, 220, 235 } : c.rank == 3 ? ccColor3B{ 230, 150, 90 } : ccColor3B{ 255, 255, 255 };
            label(row, fmt::format("#{}", c.rank).c_str(), { 22.f, 15.f }, 0.5f, "goldFont.fnt", { 0.5f, 0.5f })->setColor(rankColor);
            auto em = emblem(c.emblemId, c.colorId, 18.f, c.badgeColors);
            em->setPosition({ 54.f, 15.f });
            row->addChild(em);
            auto name = label(row, c.name.c_str(), { 74.f, 18.f }, 0.4f, "bigFont.fnt");
            name->setLimitLabelWidth(160.f, 0.4f, 0.15f);
            name->setColor(c.golden ? ccColor3B{ 255, 210, 60 } : M::clanColor(c.colorId));
            if (c.level >= kRainbowLevel) makeRainbow(name);
            if (c.level >= kPrestigeLevel) addPrestige(row, row->getContentSize(), M::clanColor(c.colorId));
            label(row, fmt::format("Level {} - {}/{} members", c.level, c.members, c.maxMembers).c_str(), { 74.f, 6.f }, 0.4f, "chatFont.fnt")->setOpacity(170);
            auto score = gpAmount(self->m_sort == "week" ? c.weekXp : c.xp, 0.5f);
            score->setAnchorPoint({ 1.f, 0.5f });
            score->setPosition({ 330.f, 15.f });
            row->addChild(score);
            self->m_list->addCell(row);
        }
        self->m_list->updateLayout(false);
        self->m_list->scrollToTop();
        if (b.clans.empty()) {
            self->m_pageLabel->setString("No clans yet - be the first!");
        } else {
            self->m_pageLabel->setString(fmt::format("Page {}  -  {}", self->m_page + 1,
                self->m_sort == "week" ? "this week" : "all time").c_str());
        }
    });
}

ClanFeedPopup* ClanFeedPopup::create(ClanDetails const& d, long long now) {
    auto ret = new ClanFeedPopup;
    if (ret->init(d, now)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanFeedPopup::init(ClanDetails const& d, long long now) {
    if (!BasePopup::init(380.f, 260.f)) return false;
    this->setTitle("Clan Activity");

    auto list = cue::ListNode::create({ 350.f, 200.f });
    list->setAutoUpdate(false);
    list->setPosition({ centerX(), centerY() - 12.f });
    m_mainLayer->addChild(list);

    for (auto const& e : d.feed) {
        auto row = CCNode::create();
        row->setContentSize({ 350.f, 24.f });
        auto text = coloredLine(M::feedText(e), 0.55f, 290.f);
        text->setPosition({ 10.f, 12.f });
        row->addChild(text);
        label(row, M::timeAgo(e.at, now).c_str(), { 340.f, 12.f }, 0.45f, "chatFont.fnt", { 1.f, 0.5f })->setOpacity(150);
        list->addCell(row);
    }
    list->updateLayout(false);
    list->scrollToTop();

    if (d.feed.empty()) {
        label(m_mainLayer, "Nothing yet - go beat some levels!", center(), 0.55f, "chatFont.fnt", { 0.5f, 0.5f })->setOpacity(170);
    }
    return true;
}

ClanRequestsPopup* ClanRequestsPopup::create(ClanDetails const& d, ClanChanged onChanged) {
    auto ret = new ClanRequestsPopup;
    if (ret->init(d, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

ClanInvitesPopup* ClanInvitesPopup::create(std::vector<ClanInviteInfo> const& invites, ClanChanged onChanged) {
    auto ret = new ClanInvitesPopup;
    if (ret->init(invites, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanInvitesPopup::init(std::vector<ClanInviteInfo> const& invites, ClanChanged onChanged) {
    if (!BasePopup::init(380.f, 250.f)) return false;
    m_onChanged = std::move(onChanged);
    this->setTitle(fmt::format("Clan Invites ({})", invites.size()).c_str());

    auto list = cue::ListNode::create({ 350.f, 180.f });
    list->setAutoUpdate(false);
    list->setCellHeight(36.f);
    list->setPosition({ centerX(), centerY() - 14.f });
    m_mainLayer->addChild(list);

    Ref<ClanInvitesPopup> self = this;
    for (auto const& inv : invites) {
        auto row = CCNode::create();
        row->setContentSize({ 350.f, 36.f });
        auto menu = CCMenu::create();
        menu->setContentSize(row->getContentSize());
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint({ 0.f, 0.f });
        menu->setPosition({ 0.f, 0.f });
        row->addChild(menu);

        auto em = emblem(inv.clan.emblemId, inv.clan.colorId, 22.f, inv.clan.badgeColors);
        em->setPosition({ 18.f, 18.f });
        row->addChild(em);
        auto name = label(row, inv.clan.name.c_str(), { 38.f, 23.f }, 0.4f, "bigFont.fnt");
        name->setLimitLabelWidth(170.f, 0.4f, 0.15f);
        name->setColor(inv.clan.golden ? ccColor3B{ 255, 210, 60 } : M::clanColor(inv.clan.colorId));
        if (inv.clan.level >= kRainbowLevel) makeRainbow(name);
        label(row, fmt::format("Lv {} - {}/{} - from {}", inv.clan.level, inv.clan.members, inv.clan.maxMembers, inv.invitedBy).c_str(),
            { 38.f, 9.f }, 0.42f, "chatFont.fnt")->setOpacity(200);

        int clanId = inv.clan.id;
        std::string clanName = inv.clan.name;
        auto respond = [self, clanId, clanName](bool accept) {
            auto body = M::authBody();
            body["clanId"] = clanId;
            body["accept"] = accept;
            M::getInstance().runAction("clan_invite_respond", body,
                accept ? "Joining..." : "Declining...",
                accept ? fmt::format("Welcome to {}!", clanName) : "Invite declined.",
                [self](matjson::Value const&) {
                    if (!self) return;
                    auto cb = std::move(self->m_onChanged);
                    self->onClose(nullptr);
                    if (cb) cb();
                });
        };
        spriteButton(menu, "GJ_completesIcon_001.png", 0.75f, { 290.f, 18.f }, [respond] { respond(true); });
        spriteButton(menu, "GJ_deleteIcon_001.png", 0.75f, { 325.f, 18.f }, [respond] { respond(false); });
        list->addCell(row);
    }
    list->updateLayout(false);
    list->scrollToTop();
    handleTouchPriority(this);
    return true;
}

bool ClanRequestsPopup::init(ClanDetails const& d, ClanChanged onChanged) {
    if (!BasePopup::init(380.f, 250.f)) return false;
    m_onChanged = std::move(onChanged);
    this->setTitle(fmt::format("Join Requests ({})", d.requests.size()).c_str());

    auto list = cue::ListNode::create({ 350.f, 180.f });
    list->setAutoUpdate(false);
    list->setCellHeight(32.f);
    list->setPosition({ centerX(), centerY() - 14.f });
    m_mainLayer->addChild(list);

    Ref<ClanRequestsPopup> self = this;
    for (auto const& r : d.requests) {
        auto row = CCNode::create();
        row->setContentSize({ 350.f, 32.f });
        auto menu = CCMenu::create();
        menu->setContentSize(row->getContentSize());
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint({ 0.f, 0.f });
        menu->setPosition({ 0.f, 0.f });
        row->addChild(menu);

        auto ic = playerIcon(r.icon, r.color1, r.color2, r.color3, 0.55f);
        ic->setPosition({ 18.f, 16.f });
        row->addChild(ic);
        Build(geode::Label::create(r.username, "bigFont.fnt"))
            .scale(0.5f)
            .with([](geode::Label* l) {
                l->setLimitLabelWidth(150.f, 0.5f, 0.2f);
            })
            .intoMenuItem([r] {
                GrindProfilePopup::create(r.accountId)->show();
            })
            .anchorPoint({ 0.f, 0.5f })
            .pos(36, 20)
            .parent(menu)
            .collect();

        auto gp = gpAmount(r.lifetimeGp, 0.38f, "chatFont.fnt");
        gp->setAnchorPoint({ 0.f, 0.5f });
        gp->setPosition({ 36.f, 8.f });
        row->addChild(gp);

        auto respond = [self, r](bool accept) {
            auto body = M::authBody();
            body["targetId"] = r.accountId;
            body["accept"] = accept;
            M::getInstance().runAction("clan_respond_request", body,
                accept ? "Accepting..." : "Declining...",
                accept ? fmt::format("Welcome, {}!", r.username) : "Request declined.",
                [self](matjson::Value const&) {
                    if (!self) return;
                    auto cb = std::move(self->m_onChanged);
                    self->onClose(nullptr);
                    if (cb) cb();
                });
        };
        spriteButton(menu, "GJ_completesIcon_001.png", 0.75f, { 290.f, 16.f }, [respond] { respond(true); });
        spriteButton(menu, "GJ_deleteIcon_001.png", 0.75f, { 325.f, 16.f }, [respond] { respond(false); });
        list->addCell(row);
    }
    list->updateLayout(false);
    list->scrollToTop();

    if (d.requests.empty()) {
        label(m_mainLayer, "No requests right now", center(), 0.55f, "chatFont.fnt", { 0.5f, 0.5f })->setOpacity(170);
    }
    return true;
}

namespace {
class ClanTextEditor : public BasePopup {
public:
    static void open(int clanID, ClanChatMessage const* parent, std::string const& initial, geode::Function<void()> saved) {
        auto popup = new ClanTextEditor;
        if (!popup->init(clanID, parent, initial, std::move(saved))) {
            delete popup;
            return;
        }
        popup->autorelease();
        popup->show();
    }

private:
    geode::Function<void()> m_onSaved;

    bool init(int clanID, ClanChatMessage const* parent, std::string const& initial, geode::Function<void()> saved) {
        bool reply = parent != nullptr;
        if (!BasePopup::init(330.f, reply ? 160.f : 220.f)) return false;
        m_onSaved = std::move(saved);
        setTitle(reply ? "Reply" : "Edit Clan Rules");
        if (reply) {
            auto quote = geode::Label::create(fmt::format("{}: {}", parent->username, parent->message), "chatFont.fnt");
            quote->setLimitLabelWidth(294.f, 0.5f, 0.18f);
            quote->setPosition({centerX(), top() - 43.f});
            m_mainLayer->addChild(quote);
        }
        auto input = reply ? TextInput::create(294.f, "Your reply", "chatFont.fnt")
            : TextInput::createMultiline({294.f, 128.f}, "# Rules", "chatFont.fnt");
        input->setMaxCharCount(reply ? 150 : 2000);
        if (reply) {
            input->setCommonFilter(CommonFilter::TextEmojis);
            input->setDisplayEmojis(true);
        } else input->setWrap(true);
        input->setString(initial);
        input->setPosition({centerX(), centerY() - 2.f});
        m_mainLayer->addChild(input);
        nodeButton(m_buttonMenu, textButton(reply ? "Send" : "Save", "GJ_button_01.png", 0.7f), {centerX(), 22.f},
            [this, input, clanID, replyID = parent ? parent->id : 0LL] {
                auto body = M::authBody();
                if (replyID) {
                    if (string::trim(input->getString()).empty()) return;
                    body["replyTo"] = static_cast<int64_t>(replyID);
                    body["message"] = input->getString();
                } else {
                    body["clanId"] = clanID;
                    body["rules"] = input->getString();
                }
                Ref<ClanTextEditor> self = this;
                M::getInstance().runAction(replyID ? "clan_chat_send" : "clan_rules_set", body,
                    replyID ? "Sending reply..." : "Saving rules...", replyID ? "Reply sent." : "Rules saved.",
                    [self](matjson::Value const&) {
                        self->onClose(nullptr);
                        if (self->m_onSaved) self->m_onSaved();
                    });
            });
        return true;
    }
};
}

void showClanRules(ClanDetails const& clan, geode::Function<void()> onSaved) {
    auto content = clan.rules.empty() ? std::string("No rules have been posted yet.") : clan.rules;
    if (clan.myRole != 2) {
        MDPopup::create("Clan Rules", content, "Close")->show();
        return;
    }
    MDPopup::create("Clan Rules", content, "Close", "Edit",
        [id = clan.info.id, rules = clan.rules, saved = std::move(onSaved)](bool edit) mutable {
            if (edit) ClanTextEditor::open(id, nullptr, rules, std::move(saved));
        })->show();
}

ClanMessagePopup* ClanMessagePopup::create(ClanChatMessage const& msg, bool canDelete, bool canPin, bool isPinned, geode::Function<void()> onChanged) {
    auto ret = new ClanMessagePopup;
    if (ret->init(msg, canDelete, canPin, isPinned, std::move(onChanged))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClanMessagePopup::init(ClanChatMessage const& msg, bool canDelete, bool canPin, bool isPinned, geode::Function<void()> onDeleted) {
    int myId = GJAccountManager::get()->m_accountID;
    bool mine = msg.accountId == myId;
    int buttons = 2 + (mine ? 0 : 1) + (canDelete ? 1 : 0) + (canPin ? 1 : 0);
    float const rowH = 30.f;
    if (!BasePopup::init(260.f, 96.f + buttons * rowH)) return false;
    m_msg = msg;
    m_onDeleted = std::move(onDeleted);
    this->setTitle(msg.username.c_str());

    auto preview = geode::Label::create(msg.message, "chatFont.fnt");
    preview->setScale(0.5f);
    preview->setLimitLabelWidth(220.f, 0.5f, 0.2f);
    preview->setPosition({ centerX(), top() - 44.f });
    preview->setOpacity(210);
    m_mainLayer->addChild(preview);

    auto menu = CCMenu::create();
    menu->setLayout(ColumnLayout::create()->setGap(6.f)->setAxisReverse(true)->setAutoScale(false));
    menu->setContentSize({ 200.f, buttons * rowH });
    menu->setPosition({ centerX(), (top() - 58.f) / 2.f + 4.f });
    m_mainLayer->addChild(menu);

    Ref<ClanMessagePopup> self = this;
    auto add = [&](const char* text, const char* bg, geode::Function<void()> cb) {
        auto spr = ButtonSprite::create(text, 150, true, "goldFont.fnt", bg, 26.f, 0.65f);
        spr->setScale(0.75f);
        menu->addChild(CCMenuItemExt::createSpriteExtra(spr, [cb = std::move(cb)](CCObject*) mutable { cb(); }));
    };

    add("Reply", "GJ_button_01.png", [this] {
        ClanTextEditor::open(0, &m_msg, "", nullptr);
        onClose(nullptr);
    });
    add("View Profile", "GJ_button_04.png", [id = msg.accountId] { GrindProfilePopup::create(id)->show(); });
    if (!mine) {
        add("Report", "GJ_button_05.png", [self] {
            if (!self) return;
            auto const& m = self->m_msg;
            ReportPopup::create(ReportTarget::Message, static_cast<int>(m.id), fmt::format("{}: {}", m.username, m.message))->show();
        });
    }
    if (canPin) {
        add(isPinned ? "Unpin" : "Pin", "GJ_button_02.png", [self, isPinned] {
            if (!self) return;
            auto body = M::authBody();
            body["messageId"] = static_cast<int64_t>(isPinned ? 0 : self->m_msg.id);
            M::getInstance().runAction("clan_chat_pin", body, isPinned ? "Unpinning..." : "Pinning...",
                isPinned ? "Message unpinned." : "Message pinned!", [self](matjson::Value const&) {
                    if (!self) return;
                    auto cb = std::move(self->m_onDeleted);
                    self->onClose(nullptr);
                    if (cb) cb();
                });
        });
    }
    if (canDelete) {
        add("Delete", "GJ_button_06.png", [self] {
            if (!self) return;
            createQuickPopup("Delete message?", "Remove this message from the clan chat?", "Keep", "Delete", [self](auto, bool yes) {
                if (!yes || !self) return;
                auto body = M::authBody();
                body["messageId"] = static_cast<int64_t>(self->m_msg.id);
                M::getInstance().runAction("clan_chat_delete", body, "Deleting...", "Message deleted.", [self](matjson::Value const&) {
                    if (!self) return;
                    auto cb = std::move(self->m_onDeleted);
                    self->onClose(nullptr);
                    if (cb) cb();
                });
            });
        });
    }
    menu->updateLayout();
    return true;
}

ShareLevelPopup* ShareLevelPopup::create(geode::Function<void(int, std::string const&)> onPicked) {
    auto ret = new ShareLevelPopup;
    if (ret->init(std::move(onPicked))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ShareLevelPopup::init(geode::Function<void(int, std::string const&)> onPicked) {
    if (!BasePopup::init(240.f, 130.f)) return false;
    m_onPicked = std::move(onPicked);
    this->setTitle("Share Level");

    m_input = TextInput::create(150.f, "Level ID", "bigFont.fnt");
    m_input->setCommonFilter(CommonFilter::Uint);
    m_input->setMaxCharCount(10);
    m_input->setPosition({ centerX(), centerY() + 4.f });
    m_mainLayer->addChild(m_input);

    m_status = label(m_mainLayer, "", { centerX(), centerY() - 22.f }, 0.3f, "bigFont.fnt", { 0.5f, 0.5f });

    Ref<ShareLevelPopup> self = this;
    auto share = CCMenuItemExt::createSpriteExtra(textButton("Share", "GJ_button_01.png", 0.7f), [self](CCObject*) {
        if (self) self->onShare();
    });
    share->setPosition({ m_buttonMenu->getContentWidth() / 2.f, 22.f });
    m_buttonMenu->addChild(share);
    return true;
}

void ShareLevelPopup::onShare() {
    if (!m_pendingKey.empty()) return;
    int id = utils::numFromString<int>(m_input->getString()).unwrapOr(0);
    if (id <= 0) {
        m_status->setString("Type a level ID");
        m_status->setColor({ 255, 120, 90 });
        return;
    }
    auto glm = GameLevelManager::get();
    m_levelId = id;
    if (auto saved = glm->getSavedLevel(id)) {
        this->finish(saved);
        return;
    }
    auto search = GJSearchObject::create(SearchType::Type19, numToString(id));
    m_pendingKey = search->getKey();
    if (auto stored = glm->getStoredOnlineLevels(m_pendingKey.c_str()); stored && stored->count() > 0) {
        this->finish(static_cast<GJGameLevel*>(stored->objectAtIndex(0)));
        return;
    }
    m_status->setString("Looking for the level...");
    m_status->setColor({ 255, 255, 255 });
    m_waited = 0.f;

    glm->m_levelManagerDelegate = nullptr;
    glm->getOnlineLevels(search);
    this->scheduleUpdate();
}

void ShareLevelPopup::update(float dt) {
    if (m_pendingKey.empty()) return;
    m_waited += dt;
    if (auto stored = GameLevelManager::get()->getStoredOnlineLevels(m_pendingKey.c_str()); stored && stored->count() > 0) {
        this->finish(static_cast<GJGameLevel*>(stored->objectAtIndex(0)));
        return;
    }
    if (m_waited > 10.f) {
        m_pendingKey.clear();
        this->unscheduleUpdate();
        m_status->setString("Level not found");
        m_status->setColor({ 255, 120, 90 });
    }
}

void ShareLevelPopup::finish(GJGameLevel* level) {
    m_pendingKey.clear();
    this->unscheduleUpdate();
    if (!level) return;
    auto cb = std::move(m_onPicked);
    int id = level->m_levelID.value() > 0 ? level->m_levelID.value() : m_levelId;
    std::string name = level->m_levelName;
    this->onClose(nullptr);
    if (cb) cb(id, name);
}

}
