#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/EmoteDef.hpp>
#include <Geode/binding/EmoteSprite.hpp>
#include <Geode/binding/EmoteToken.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/TextArea.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>

#include "../managers/GrindClansManager.hpp"
#include "components/RainbowLabel.hpp"

using namespace geode::prelude;

namespace levelgrind::clanui {

inline CCSprite* gpIcon(float scale = 0.5f, float boost = 1.15f) {
    auto spr = CCSprite::create("grind_point_icon.png"_spr);
    spr->setScale(scale * boost);
    return spr;
}

inline NineSlice* darkBox(CCSize size, GLubyte opacity = 80) {
    auto box = NineSlice::create("square02_small.png");
    box->setContentSize(size);
    box->setOpacity(opacity);
    return box;
}

struct Panel {
    CCNode* node = nullptr;
    CCMenu* menu = nullptr;
};

inline Panel makePanel(CCNode* parent, CCPoint pos, CCSize size, GLubyte opacity = 80) {
    Panel p;
    p.node = CCNode::create();
    p.node->setContentSize(size);
    p.node->setAnchorPoint({ 0.5f, 0.5f });
    p.node->ignoreAnchorPointForPosition(false);
    p.node->setPosition(pos);
    parent->addChild(p.node);

    auto box = darkBox(size, opacity);
    box->setAnchorPoint({ 0.f, 0.f });
    box->setPosition({ 0.f, 0.f });
    p.node->addChild(box, -1);

    p.menu = CCMenu::create();
    p.menu->setContentSize(size);
    p.menu->ignoreAnchorPointForPosition(false);
    p.menu->setAnchorPoint({ 0.f, 0.f });
    p.menu->setPosition({ 0.f, 0.f });
    p.node->addChild(p.menu, 5);
    return p;
}

inline CCNode* gpAmount(long long gp, float scale = 0.5f, const char* font = "goldFont.fnt") {
    auto node = CCNode::create();
    auto icon = gpIcon(scale * 0.9f);
    auto label = geode::Label::create(GrindClansManager::formatGP(gp).c_str(), font);
    label->setScale(scale);
    float w = icon->getScaledContentWidth() + 3.f + label->getScaledContentWidth();
    float h = std::max(icon->getScaledContentHeight(), label->getScaledContentHeight());
    node->setContentSize({ w, h });
    node->setAnchorPoint({ 0.5f, 0.5f });
    icon->setPosition({ icon->getScaledContentWidth() / 2.f, h / 2.f });
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ icon->getScaledContentWidth() + 3.f, h / 2.f });
    node->addChild(icon);
    node->addChild(label);
    return node;
}

struct ClanBadge {
    int background;
    int symbol;
    int ink;

    static constexpr ccColor3B inks[] = {
        {255,255,255}, {48,52,65}, {255,205,90}, {110,210,255}, {245,115,125}, {140,235,155}
    };
    static ClanBadge decode(int id) {
        int index = (std::clamp(id, 1, 1000) - 1) % (70 * std::size(inks));
        return {index % 10 + 1, index / 10 % 7 + 1, index / 70};
    }
    int encode() const { return background + 10 * (symbol - 1) + 70 * ink; }
};

inline int rgbValue(ccColor3B c) { return (c.r << 16) | (c.g << 8) | c.b; }

inline ccColor3B badgeColor(long long rgb) {
    return {static_cast<GLubyte>(rgb >> 16), static_cast<GLubyte>(rgb >> 8), static_cast<GLubyte>(rgb)};
}

enum class EmblemPart { Both, Background, Symbol };

inline CCNode* emblem(int emblemId, int colorId, float size, long long colors, EmblemPart part = EmblemPart::Both) {
    auto badge = ClanBadge::decode(emblemId);
    auto node = CCNode::create();
    node->setContentSize({size, size});
    node->setAnchorPoint({0.5f, 0.5f});
    auto addPart = [&](const char* prefix, int id, ccColor3B color, GLubyte highlight) {
        for (std::string_view suffix : {"", "_extra", "_highlight"}) {
            auto name = fmt::format("{}_{}{}.png"_spr, prefix, id, suffix);
            auto frame = CCSpriteFrameCache::get()->spriteFrameByName(name.c_str());
            if (!frame) continue;
            auto sprite = CCSprite::createWithSpriteFrame(frame);
            sprite->setScale(size / 125.f);
            sprite->setPosition({size / 2.f, size / 2.f});
            if (suffix.empty()) sprite->setColor(color);
            else if (suffix == "_extra") sprite->setBlendFunc({GL_DST_COLOR, GL_ONE});
            else {
                sprite->setBlendFunc({GL_ONE, GL_ONE});
                sprite->setOpacity(highlight);
            }
            node->addChild(sprite);
        }
    };
    if (part != EmblemPart::Symbol) addPart("bg", badge.background, colors < 0 ? GrindClansManager::clanColor(colorId) : badgeColor(colors >> 24), 90);
    if (part != EmblemPart::Background) addPart("icon", badge.symbol, colors < 0 ? ClanBadge::inks[badge.ink] : badgeColor(colors), 127);
    return node;
}

inline CCNode* playerIcon(int icon, int c1, int c2, int c3, float scale) {
    auto spr = cue::PlayerIcon::create(cue::Icons{ IconType::Cube, std::max(1, icon), c1, c2, c3 == 0 ? -1 : c3 });
    spr->setScale(scale);
    return spr;
}

inline ButtonSprite* textButton(const char* text, const char* bg = "GJ_button_01.png", float scale = 0.6f) {
    auto spr = ButtonSprite::create(text, "goldFont.fnt", bg, 0.8f);
    spr->setScale(scale);
    return spr;
}

inline TextArea* richText(std::string const& text, float width, float scale = 0.5f, CCPoint anchor = { 0.f, 0.5f }) {
    return TextArea::create(text, "chatFont.fnt", scale, width / scale, anchor, 20.f, false, 0);
}

inline CCLayerColor* divider(float width, GLubyte opacity = 60) {
    return CCLayerColor::create({ 0, 0, 0, opacity }, width, 1.f);
}

inline std::string countdown(long long seconds) {
    if (seconds <= 0) return "soon";
    long long d = seconds / 86400, h = (seconds % 86400) / 3600, m = (seconds % 3600) / 60;
    if (d > 0) return fmt::format("{}d {}h", d, h);
    if (h > 0) return fmt::format("{}h {}m", h, m);
    return fmt::format("{}m", std::max(1LL, m));
}

inline CCNode* coloredLine(std::string const& rich, float scale, float maxWidth, const char* font = "chatFont.fnt") {
    auto colorFor = [](char c) -> ccColor3B {
        switch (c) {
            case 'y': return { 255, 255, 0 };
            case 'g': return { 64, 227, 72 };
            case 'r': return { 255, 90, 90 };
            case 'l': return { 96, 171, 239 };
            case 'j': return { 50, 200, 255 };
            case 'o': return { 255, 165, 75 };
            case 'p': return { 255, 0, 255 };
        }
        return { 255, 255, 255 };
    };

    auto node = CCNode::create();
    node->setAnchorPoint({ 0.f, 0.5f });
    float x = 0.f, h = 0.f;
    auto add = [&](std::string const& text, ccColor3B color) {
        if (text.empty()) return;
        auto l = geode::Label::create(text, font);
        l->setScale(scale);
        l->setColor(color);
        l->setAnchorPoint({ 0.f, 0.5f });
        l->setPosition({ x, 0.f });
        node->addChild(l);
        x += l->getScaledContentWidth();
        h = std::max(h, l->getScaledContentHeight());
    };

    size_t pos = 0;
    while (pos < rich.size()) {
        auto open = rich.find("<c", pos);
        if (open == std::string::npos || open + 3 >= rich.size() || rich[open + 3] != '>') {
            add(rich.substr(pos), { 255, 255, 255 });
            break;
        }
        add(rich.substr(pos, open - pos), { 255, 255, 255 });
        auto close = rich.find("</c>", open);
        auto inner = rich.substr(open + 4, close == std::string::npos ? std::string::npos : close - open - 4);
        add(inner, colorFor(rich[open + 2]));
        if (close == std::string::npos) break;
        pos = close + 4;
    }

    node->setContentSize({ x, h });
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) child->setPositionY(h / 2.f);
    if (x > maxWidth && x > 0.f) node->setScale(maxWidth / x);
    return node;
}

inline std::string levelToken(int id, std::string const& name) {
    std::string clean;
    for (char ch : name) {
        if (ch != '[' && ch != ']' && ch != ':') clean += ch;
    }
    return fmt::format("[lvl:{}:{}]", id, clean);
}

struct ChatLinks {
    std::shared_ptr<geode::Function<void(int)>> onLevel;
    std::shared_ptr<geode::Function<void(std::string const&)>> onMention;
};

inline CCNode* chatText(std::string const& raw, float scale, float maxWidth, ChatLinks const& links = {}, const char* font = "chatFont.fnt") {
    struct Piece { std::string text; EmoteDef const* def = nullptr; int levelId = 0; };
    std::vector<Piece> pieces;

    auto addText = [&](std::string const& part) {
        if (part.empty()) return;
        std::string text = EmoteSprite::translateUnicodeEmoji(part);
        std::vector<EmoteToken> tokens;
        EmoteSprite::parseTokens(text, &tokens, EmoteSprite::maxEmotesMessage());
        size_t pos = 0;
        for (auto const& t : tokens) {
            if (!t.m_def || t.m_start < pos || t.m_start >= text.size()) continue;

            size_t end = t.m_end;
            if (t.m_def->m_name) {
                std::string name = t.m_def->m_name;
                auto at = text.find(name, t.m_start);
                if (!name.empty() && at != std::string::npos && at <= t.m_start + 1) {
                    end = at + name.size();
                    if (at > t.m_start && end < text.size() && text[end] == text[t.m_start]) end++;
                }
            }
            end = std::clamp(end, t.m_start + 1, text.size());
            if (t.m_start > pos) pieces.push_back({ text.substr(pos, t.m_start - pos) });
            pieces.push_back({ "", t.m_def });
            pos = end;
        }
        if (pos < text.size()) pieces.push_back({ text.substr(pos) });
    };
    size_t pos = 0;
    while (pos < raw.size()) {
        auto open = raw.find("[lvl:", pos);
        auto close = open == std::string::npos ? std::string::npos : raw.find(']', open);
        if (close == std::string::npos) {
            addText(raw.substr(pos));
            break;
        }
        auto inner = raw.substr(open + 5, close - open - 5);
        auto colon = inner.find(':');
        int id = 0;
        if (colon != std::string::npos) id = utils::numFromString<int>(inner.substr(0, colon)).unwrapOr(0);
        if (id <= 0) {
            addText(raw.substr(pos, close + 1 - pos));
        } else {
            addText(raw.substr(pos, open - pos));
            auto name = inner.substr(colon + 1);
            pieces.push_back({ name.empty() ? fmt::format("Level {}", id) : name, nullptr, id });
        }
        pos = close + 1;
    }

    auto probe = geode::Label::create("Ag", font);
    float const lineH = probe->getContentHeight() * scale;
    auto spaced = geode::Label::create("a a", font);
    auto tight = geode::Label::create("aa", font);
    float const spaceW = std::max(1.f, (spaced->getContentWidth() - tight->getContentWidth()) * scale);
    float const emoteH = lineH * 1.05f;

    auto node = CCNode::create();
    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    node->addChild(menu, 1);

    struct Item { CCNode* node; float x; int line; };
    std::vector<Item> items;
    float x = 0.f, widest = 0.f;
    int line = 0;
    auto place = [&](CCNode* n, float w) {
        if (x > 0.f && x + w > maxWidth) {
            line++;
            x = 0.f;
        }
        items.push_back({ n, x, line });
        x += w;
        widest = std::max(widest, x);
    };

    auto link = [&](std::string const& text, ccColor3B color, geode::Function<void()> cb) {
        auto l = geode::Label::create(text, font);
        l->setScale(scale);
        l->setColor(color);
        auto holder = CCNode::create();
        holder->setContentSize(l->getScaledContentSize());
        l->setAnchorPoint({ 0.f, 0.f });
        holder->addChild(l);
        auto item = CCMenuItemExt::createSpriteExtra(holder, [cb = std::move(cb)](CCObject*) mutable { cb(); });
        item->m_scaleMultiplier = 1.08f;
        place(item, holder->getContentWidth());
    };

    for (auto const& p : pieces) {
        if (p.def) {
            auto spr = EmoteSprite::createWithDef(p.def);
            if (!spr) continue;
            spr->setScale(emoteH / std::max(1.f, spr->getContentHeight()));
            place(spr, spr->getScaledContentWidth() + 1.f);
            continue;
        }
        if (p.levelId > 0) {
            int id = p.levelId;
            auto cb = links.onLevel;
            link(p.text, { 255, 230, 60 }, [cb, id] { if (cb && *cb) (*cb)(id); });
            continue;
        }
        size_t i = 0;
        while (i < p.text.size()) {
            if (p.text[i] == ' ') {
                x += spaceW;
                i++;
                continue;
            }
            auto next = p.text.find(' ', i);
            auto word = p.text.substr(i, next == std::string::npos ? std::string::npos : next - i);
            i += word.size();
            if (word.size() > 1 && word[0] == '@') {
                auto name = word.substr(1);
                while (!name.empty() && std::string(".,!?;:").find(name.back()) != std::string::npos) name.pop_back();
                auto cb = links.onMention;
                link(word, { 120, 200, 255 }, [cb, name] { if (cb && *cb) (*cb)(name); });
                continue;
            }
            while (!word.empty()) {
                auto l = geode::Label::create(word, font);
                l->setScale(scale);
                if (l->getScaledContentWidth() <= maxWidth || word.size() == 1) {
                    place(l, l->getScaledContentWidth());
                    break;
                }
                size_t fit = word.size();
                do {
                    size_t prev = fit - 1;
                    while (prev && (static_cast<unsigned char>(word[prev]) & 0xc0) == 0x80) --prev;
                    if (!prev) break;
                    fit = prev;
                    l->setText(word.substr(0, fit));
                } while (l->getScaledContentWidth() > maxWidth);
                if (x > 0.f) { line++; x = 0.f; }
                place(l, l->getScaledContentWidth());
                word = word.substr(fit);
            }
        }
    }

    float const height = (line + 1) * lineH;
    node->setContentSize({ std::min(widest, maxWidth), height });
    node->setAnchorPoint({ 0.f, 1.f });
    menu->setContentSize(node->getContentSize());
    for (auto const& it : items) {
        it.node->setAnchorPoint({ 0.f, 0.5f });
        it.node->setPosition({ it.x, height - (it.line + 0.5f) * lineH });
        if (typeinfo_cast<CCMenuItem*>(it.node)) {
            it.node->setAnchorPoint({ 0.5f, 0.5f });
            it.node->setPosition({ it.x + it.node->getContentWidth() / 2.f, height - (it.line + 0.5f) * lineH });
            menu->addChild(it.node);
        } else {
            node->addChild(it.node);
        }
    }
    return node;
}

constexpr int kPrestigeLevel = 25;
constexpr int kRainbowLevel = 30;

inline void addPrestige(CCNode* row, CCSize size, ccColor3B color) {
    #ifndef GEODE_IS_IOS
    auto sparkles = CCParticleSnow::create();
    sparkles->setTotalParticles(18);
    sparkles->setPosition({ size.width / 2.f, 0.f });
    sparkles->setPosVar({ size.width / 2.f, 0.f });
    sparkles->setGravity({ 0.f, 14.f });
    sparkles->setSpeed(6.f);
    sparkles->setSpeedVar(3.f);
    sparkles->setAngle(90.f);
    sparkles->setAngleVar(20.f);
    sparkles->setLife(2.f);
    sparkles->setLifeVar(0.8f);
    sparkles->setEmissionRate(6.f);
    sparkles->setStartSize(3.f);
    sparkles->setStartSizeVar(1.5f);
    sparkles->setEndSize(0.f);
    sparkles->setStartColor({ color.r / 255.f, color.g / 255.f, color.b / 255.f, 0.9f });
    sparkles->setStartColorVar({ 0.1f, 0.1f, 0.1f, 0.1f });
    sparkles->setEndColor({ 1.f, 1.f, 1.f, 0.f });
    sparkles->setEndColorVar({ 0.f, 0.f, 0.f, 0.f });
    sparkles->setBlendAdditive(true);
    sparkles->setPositionType(kCCPositionTypeGrouped);
    row->addChild(sparkles, 1);
    #endif
}

inline void makeRainbow(geode::Label* name, const char* font = "bigFont.fnt") {
    if (!name || !name->getParent()) return;
    auto rainbow = RainbowLabel::create(name->getString(), font);
    rainbow->setScale(name->getScale());
    rainbow->setAnchorPoint(name->getAnchorPoint());
    rainbow->setPosition(name->getPosition());
    name->getParent()->addChild(rainbow, name->getZOrder());
    name->setVisible(false);
}

}
