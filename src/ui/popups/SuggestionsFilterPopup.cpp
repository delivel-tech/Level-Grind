#include "SuggestionsFilterPopup.hpp"
#include <Geode/ui/Label.hpp>
#include "../../managers/DataManager.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/general.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>
#include <algorithm>

namespace levelgrind {

namespace {

using Op = SuggestionsLayer::PointsFilterOp;

constexpr int kMinPoints = -999;
constexpr int kMaxPoints = 999;

struct OpDef {
    Op op;
    const char* caption;
    const char* symbol;
};

constexpr OpDef kOps[] = {
    { Op::More,   "Above",   ">" },
    { Op::Equals, "Exactly", "=" },
    { Op::Less,   "Below",   "<" },
};

const char* symbolFor(Op op) {
    for (auto const& def : kOps) {
        if (def.op == op) return def.symbol;
    }
    return "?";
}

}

SuggestionsFilterPopup* SuggestionsFilterPopup::create(SuggestionsLayer *owner) {
    auto ret = new SuggestionsFilterPopup;
    if (ret && ret->init(owner)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SuggestionsFilterPopup::init(SuggestionsLayer* owner) {
    if (!BasePopup::init(300.f, 282.f)) return false;

    m_owner = owner;
    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::User) setTitle("Unknown Role: Filter");
    else if (pos == GrindPosition::Helper) setTitle("Grind Helper: Filter");
    else if (pos == GrindPosition::Admin) setTitle("Grind Admin: Filter");
    else if (pos == GrindPosition::Owner) setTitle("Grind Coordinator: Filter");

    Build(geode::Label::create("Show suggestions by their point total.", "chatFont.fnt"))
        .scale(0.55f)
        .pos(fromTop(40.f))
        .opacity(170)
        .parent(m_mainLayer);

    CCSize const panelSize { 260.f, 172.f };

    auto panel = Build<CCNode>::create()
        .contentSize(panelSize)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos({ centerX(), centerY() + 6.f })
        .parent(m_mainLayer)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(panelSize)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(75)
        .parent(panel);

    auto panelMenu = Build<CCMenu>::create()
        .contentSize(panelSize)
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(panel)
        .collect();

    auto makeSegment = [](const char* caption, bool on) {
        return ButtonSprite::create(caption, 70, true, "goldFont.fnt", on ? "GJ_button_02.png" : "GJ_button_04.png", 26.f, 0.6f);
    };

    float const modeY = panelSize.height - 24.f;

    Build(geode::Label::create("Levels", "goldFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 16.f, modeY })
        .parent(panel);

    auto layoutRow = [&](std::vector<CCMenuItemToggler*> const& items, float y, float rightEdge) {
        float const gap = 5.f;
        float total = -gap;
        for (auto item : items) total += item->getScaledContentWidth() + gap;

        float x = rightEdge < 0.f ? (panelSize.width - total) / 2.f : rightEdge - total;
        for (auto item : items) {
            float w = item->getScaledContentWidth();
            item->setPosition({ x + w / 2.f, y });
            panelMenu->addChild(item);
            x += w + gap;
        }
    };

    constexpr std::pair<int, const char*> kModes[] = { { 0, "Not added" }, { 1, "Added" } };
    std::vector<CCMenuItemToggler*> modeItems;
    for (auto const& [mode, caption] : kModes) {
        auto toggler = m_modeRadio.createToggler(mode, makeSegment(caption, true), makeSegment(caption, false));
        toggler->setScale(0.8f);
        modeItems.push_back(toggler);
    }
    layoutRow(modeItems, modeY, panelSize.width - 14.f);

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, panelSize.width - 20.f, 1.f))
        .pos({ 10.f, panelSize.height - 45.f })
        .parent(panel);

    float const segmentY = panelSize.height - 68.f;
    std::vector<CCMenuItemToggler*> opItems;
    for (auto const& def : kOps) {
        auto toggler = m_radio.createToggler(def.op, makeSegment(def.caption, true), makeSegment(def.caption, false));
        toggler->setScale(0.8f);
        opItems.push_back(toggler);
    }
    layoutRow(opItems, segmentY, -1.f);

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, panelSize.width - 20.f, 1.f))
        .pos({ 10.f, panelSize.height - 91.f })
        .parent(panel);

    float const valueY = panelSize.height - 114.f;

    Build(geode::Label::create("Points", "goldFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 16.f, valueY })
        .parent(panel);

    float const stepperX = panelSize.width - 62.f;

    m_input = Build(TextInput::create(60.f, "0", "bigFont.fnt"))
        .scale(0.7f)
        .pos({ stepperX, valueY })
        .parent(panel)
        .collect();
    m_input->setMaxCharCount(4);
    m_input->setFilter("-0123456789");
    m_input->setCallback([this](auto const&) { this->updatePreview(); });

    Build<CCSprite>::createSpriteName("edit_leftBtn_001.png")
        .scale(0.8f)
        .intoMenuItem([this] { this->step(-1); })
        .pos({ stepperX - 38.f, valueY })
        .parent(panelMenu);

    Build<CCSprite>::createSpriteName("edit_rightBtn_001.png")
        .scale(0.8f)
        .intoMenuItem([this] { this->step(1); })
        .pos({ stepperX + 38.f, valueY })
        .parent(panelMenu);
    
    Build(CCLayerColor::create({ 0, 0, 0, 60 }, panelSize.width - 20.f, 1.f))
        .pos({ 10.f, panelSize.height - 136.f })
        .parent(panel);

    float const diffY = 18.f;
    Build(geode::Label::create("Difficulty", "goldFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 16.f, diffY })
        .parent(panel);

    m_diffHolder = CCNode::create();
    m_diffHolder->setContentSize({ 28.f, 28.f });
    auto diffBtn = CCMenuItemExt::createSpriteExtra(m_diffHolder, [this](CCObject*) {
        m_difficulty = (m_difficulty + 1) % 12;
        this->refreshDifficulty();
        this->updatePreview();
    });
    diffBtn->setPosition({ stepperX + 38.f, diffY });
    panelMenu->addChild(diffBtn);

    m_diffLabel = Build(geode::Label::create("All", "bigFont.fnt"))
        .scale(0.4f)
        .anchorPoint({ 1.f, 0.5f })
        .pos({ stepperX + 18.f, diffY })
        .parent(panel)
        .collect();

    m_preview = Build(geode::Label::create("", "bigFont.fnt"))
        .scale(0.32f)
        .pos({ centerX(), 47.f })
        .color({ 255, 230, 140 })
        .parent(m_mainLayer)
        .collect();

    if (m_owner) {
        m_radio.select(m_owner->getPointsFilterOp());
        m_modeRadio.select(m_owner->getSuggestionsMode());
        m_difficulty = m_owner->getDifficultyFilter();
        m_input->setString(numToString(m_owner->getPointsFilterValue()));
    } else {
        m_radio.select(Op::More);
        m_modeRadio.select(0);
    }
    m_radio.setCallback([this](Op) { this->updatePreview(); });
    m_modeRadio.setCallback([this](int) { this->updatePreview(); });
    this->refreshDifficulty();
    this->updatePreview();

    Build(ButtonSprite::create("Reset", "goldFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.7f)
        .intoMenuItem([this] {
            m_radio.select(Op::More);
            m_modeRadio.select(0);
            m_input->setString("0");
            m_difficulty = 0;
            this->refreshDifficulty();
            this->updatePreview();
        })
        .pos({ m_buttonMenu->getContentWidth() / 2.f - 55.f, 24.f })
        .id("reset-btn")
        .parent(m_buttonMenu);

    Build(ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 0.8f))
        .scale(0.8f)
        .intoMenuItem([this] { this->onApply(); })
        .pos({ m_buttonMenu->getContentWidth() / 2.f + 45.f, 24.f })
        .id("apply-btn")
        .parent(m_buttonMenu);

    return true;
}

void SuggestionsFilterPopup::step(int delta) {
    int value = numFromString<int>(m_input->getString()).unwrapOr(0);
    m_input->setString(numToString(std::clamp(value + delta, kMinPoints, kMaxPoints)));
    this->updatePreview();
}

void SuggestionsFilterPopup::refreshDifficulty() {
    static const char* frames[] = {
        "diffIcon_00_btn_001.png", "diffIcon_auto_btn_001.png", "diffIcon_01_btn_001.png", "diffIcon_02_btn_001.png",
        "diffIcon_03_btn_001.png", "diffIcon_04_btn_001.png", "diffIcon_05_btn_001.png", "diffIcon_07_btn_001.png",
        "diffIcon_08_btn_001.png", "diffIcon_06_btn_001.png", "diffIcon_09_btn_001.png", "diffIcon_10_btn_001.png",
    };
    static const char* names[] = {
        "All", "Auto", "Easy", "Normal", "Hard", "Harder", "Insane",
        "Easy Demon", "Medium Demon", "Hard Demon", "Insane Demon", "Extreme Demon",
    };
    int d = std::clamp(m_difficulty, 0, 11);
    if (m_diffHolder) {
        m_diffHolder->removeAllChildren();
        auto face = CCSprite::createWithSpriteFrameName(frames[d]);
        face->setScale(0.6f);
        face->setOpacity(d == 0 ? 160 : 255);
        face->setPosition(m_diffHolder->getContentSize() / 2.f);
        m_diffHolder->addChild(face);
    }
    if (m_diffLabel) {
        m_diffLabel->setString(names[d]);
        m_diffLabel->setLimitLabelWidth(110.f, 0.4f, 0.15f);
        m_diffLabel->setColor(d == 0 ? ccColor3B{ 200, 200, 200 } : ccColor3B{ 255, 255, 255 });
    }
}

void SuggestionsFilterPopup::updatePreview() {
    if (!m_preview) return;

    auto value = numFromString<int>(m_input->getString());
    if (!value) {
        m_preview->setString("Enter a number");
        m_preview->setLimitLabelWidth(270.f, 0.32f, 0.15f);
        m_preview->setColor({ 255, 120, 120 });
        return;
    }

    Op op = m_radio.getSelected();
    m_preview->setColor({ 255, 230, 140 });

    if (!m_owner) {
        m_preview->setString(fmt::format("Points {} {}", symbolFor(op), value.unwrap()).c_str());
        m_preview->setLimitLabelWidth(270.f, 0.32f, 0.15f);
        return;
    }

    if (m_modeRadio.getSelected() != m_owner->getSuggestionsMode()) {
        m_preview->setString(fmt::format(
            "Points {} {} (list reloads on Apply)", symbolFor(op), value.unwrap()
        ).c_str());
        m_preview->setLimitLabelWidth(270.f, 0.32f, 0.15f);
        return;
    }

    int matching = m_owner->countPointsFilter(op, value.unwrap(), m_difficulty);
    int total = m_owner->getSuggestionsCount();
    m_preview->setString(fmt::format(
        "Points {} {}: {} of {} suggestions", symbolFor(op), value.unwrap(), matching, total
    ).c_str());
    m_preview->setLimitLabelWidth(270.f, 0.32f, 0.15f);
}

void SuggestionsFilterPopup::onApply() {
    if (!m_owner) return;

    auto value = numFromString<int>(m_input->getString());
    if (!value) {
        Notification::create("Please input a valid number!", NotificationIcon::Error)->show();
        return;
    }

    m_owner->setPointsFilter(m_radio.getSelected(), value.unwrap());
    m_owner->setDifficultyFilter(m_difficulty);
    if (m_modeRadio.getSelected() != m_owner->getSuggestionsMode()) {
        m_owner->setSuggestionsMode(m_modeRadio.getSelected());
    } else {
        m_owner->applyPointsFilter();
    }
    this->onClose(nullptr);
}

}