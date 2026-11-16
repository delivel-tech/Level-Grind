#include "AddAnnouncementPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <cue/DropdownNode.hpp>
#include <fmt/format.h>

#include "../../managers/DataManager.hpp"
#include "../../managers/APIClient.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"

using namespace geode::prelude;

namespace levelgrind {

namespace {

using LineKind = AddAnnouncementPopup::LineKind;

constexpr const char* kDraftKey = "announcement-draft";

struct PaletteColor {
    const char* name;
    char tag;
    ccColor3B color;
};

constexpr PaletteColor kPalette[] = {
    { "Default",    0,   { 255, 255, 255 } },
    { "Yellow",     'y', { 255, 255,   0 } },
    { "Orange",     'o', { 255, 165,  75 } },
    { "Red",        'r', { 255,  90,  90 } },
    { "Pink",       'p', { 255, 150, 255 } },
    { "Purple",     'a', { 150,  80, 255 } },
    { "Blue",       'b', {  74,  82, 225 } },
    { "Light Blue", 'l', {  96, 171, 239 } },
    { "Cyan",       'j', {  50, 200, 255 } },
    { "Green",      'g', {  64, 227,  72 } },
};
constexpr int kPaletteSize = static_cast<int>(std::size(kPalette));

struct KindDef {
    LineKind kind;
    const char* name;
    const char* tag;
};

constexpr KindDef kKinds[] = {
    { LineKind::Text,       "Text",       "T"  },
    { LineKind::Heading,    "Heading",    "H1" },
    { LineKind::Subheading, "Subheading", "H2" },
    { LineKind::Bullet,     "Bullet",     "-"  },
    { LineKind::Divider,    "Divider",    "--" },
};

KindDef const& kindDef(LineKind kind) {
    for (auto const& def : kKinds) {
        if (def.kind == kind) return def;
    }
    return kKinds[0];
}

constexpr float kLeftX = 15.f;
constexpr float kLeftW = 250.f;
constexpr float kRowH = 22.f;

std::string spanMarkdown(AddAnnouncementPopup::Span const& span) {
    std::string s = span.text;
    if (span.bold) s = "**" + s + "**";
    if (span.italic) s = "*" + s + "*";
    char tag = kPalette[std::clamp(span.color, 0, kPaletteSize - 1)].tag;
    if (tag) s = fmt::format("<c{}>{}</c>", tag, s);
    return s;
}

std::string plainText(AddAnnouncementPopup::Line const& line) {
    if (line.kind == LineKind::Divider) return "Divider";
    std::string out;
    for (auto const& span : line.spans) {
        if (!out.empty()) out += " ";
        out += span.text;
    }
    return out;
}

CCNode* dropdownCell(float width, const char* text, std::optional<ccColor3B> swatch) {
    auto cell = CCNode::create();
    cell->setContentSize({ width, kRowH });

    float x = 8.f;
    if (swatch) {
        auto sw = CCLayerColor::create({ swatch->r, swatch->g, swatch->b, 255 }, 10.f, 10.f);
        sw->setPosition({ x, kRowH / 2.f - 5.f });
        cell->addChild(sw);
        x += 15.f;
    }

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition({ x, kRowH / 2.f });
    label->limitLabelWidth(width - x - 28.f, 0.4f, 0.1f);
    if (swatch) label->setColor(*swatch);
    cell->addChild(label);

    return cell;
}

CCMenuItemSpriteExtra* smallButton(const char* text, const char* bg, float scale, geode::Function<void()> cb) {
    return Build(ButtonSprite::create(text, "goldFont.fnt", bg, 0.8f))
        .scale(scale)
        .intoMenuItem([cb = std::move(cb)]() mutable { cb(); })
        .collect();
}

}

AddAnnouncementPopup* AddAnnouncementPopup::create() {
    auto ret = new AddAnnouncementPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AddAnnouncementPopup::init() {
    if (!BasePopup::init({ 460.f, 290.f }, "GJ_square01.png")) return false;

    if (DataManager::getInstance().getUserPosition() == GrindPosition::Admin) {
        setTitle("Grind Admin: Add Announcement");
    } else if (DataManager::getInstance().getUserPosition() == GrindPosition::Owner) {
        setTitle("Grind Coordinator: Add Announcement");
    } else {
        setTitle("Unknown Role: Add Announcement");
        Notification::create("You are not allowed to add announcements.", NotificationIcon::Error)->show();
        return true;
    }

    m_titleTextInput = Build(TextInput::create(420.f, "Announcement title", "chatFont.fnt"))
        .scale(0.8f)
        .pos({ centerX(), top() - 46.f })
        .parent(m_mainLayer)
        .id("title-text-input")
        .collect();
    m_titleTextInput->setCommonFilter(CommonFilter::Any);
    m_titleTextInput->setMaxCharCount(100);
    m_titleTextInput->setCallback([this](auto const&) { this->refreshPreview(); });

    float const listTop = top() - 66.f;
    float const listH = 96.f;

    m_lineList = Build(cue::ListNode::create({ kLeftW, listH }))
        .anchorPoint({ 0.5f, 1.f })
        .pos({ kLeftX + kLeftW / 2.f, listTop })
        .parent(m_mainLayer)
        .id("line-list")
        .collect();
    m_lineList->setAutoUpdate(false);
    m_lineList->setCellHeight(kRowH);

    m_emptyLabel = Build(CCLabelBMFont::create("Type below and press Add to start", "chatFont.fnt"))
        .scale(0.55f)
        .opacity(150)
        .pos({ kLeftX + kLeftW / 2.f, listTop - listH / 2.f })
        .parent(m_mainLayer)
        .collect();

    float const previewX = kLeftX + kLeftW + 10.f;
    CCSize const previewSize { right() - previewX - 15.f, listTop - 50.f };

    Build(NineSlice::create("square02_small.png"))
        .contentSize(previewSize)
        .anchorPoint({ 0.f, 1.f })
        .pos({ previewX, listTop })
        .opacity(80)
        .parent(m_mainLayer);

    Build(CCLabelBMFont::create("Preview", "goldFont.fnt"))
        .scale(0.4f)
        .anchorPoint({ 1.f, 0.f })
        .pos({ previewX + previewSize.width - 6.f, listTop + 2.f })
        .opacity(200)
        .parent(m_mainLayer);

    m_preview = Build(MDTextArea::create("", { previewSize.width - 8.f, previewSize.height - 8.f }))
        .pos({ previewX + previewSize.width / 2.f, listTop - previewSize.height / 2.f })
        .parent(m_mainLayer)
        .id("preview")
        .collect();

    this->buildComposer(m_mainLayer, 80.f);
    this->buildToolbar(m_mainLayer, listTop - listH - 14.f);

    auto previewBtn = smallButton("Full Preview", "GJ_button_04.png", 0.6f, [this] { this->onPreview(); });
    previewBtn->setPosition({ m_buttonMenu->getContentWidth() / 2.f - 60.f, 22.f });
    m_buttonMenu->addChild(previewBtn);

    auto postBtn = smallButton("Post", "GJ_button_01.png", 0.75f, [this] { this->onPost(); });
    postBtn->setPosition({ m_buttonMenu->getContentWidth() / 2.f + 60.f, 22.f });
    m_buttonMenu->addChild(postBtn);

    this->loadDraft();
    this->refresh();

    return true;
}

void AddAnnouncementPopup::buildToolbar(CCNode* parent, float y) {
    float const ddW = 94.f;

    auto kindDropdown = cue::DropdownNode::create({ 0, 0, 0, 120 }, ddW, kRowH, kRowH * 5.f);
    for (auto const& def : kKinds) {
        kindDropdown->addCell(dropdownCell(ddW, def.name, std::nullopt));
    }
    kindDropdown->setAnchorPoint({ 0.5f, 1.f });
    kindDropdown->setPosition({ kLeftX + ddW / 2.f, y + kRowH / 2.f });
    kindDropdown->setZOrder(100);
    kindDropdown->setID("style-dropdown");
    kindDropdown->setCallback([this](size_t idx, CCNode*) {
        if (idx < std::size(kKinds)) this->setKind(kKinds[idx].kind);
    });
    parent->addChild(kindDropdown);

    auto colorDropdown = cue::DropdownNode::create({ 0, 0, 0, 120 }, ddW, kRowH, kRowH * 5.f);
    for (auto const& c : kPalette) {
        colorDropdown->addCell(dropdownCell(ddW, c.name, c.color));
    }
    colorDropdown->setAnchorPoint({ 0.5f, 1.f });
    colorDropdown->setPosition({ kLeftX + ddW * 1.5f + 6.f, y + kRowH / 2.f });
    colorDropdown->setZOrder(100);
    colorDropdown->setID("color-dropdown");
    colorDropdown->setCallback([this](size_t idx, CCNode*) {
        m_color = std::clamp(static_cast<int>(idx), 0, kPaletteSize - 1);
    });
    parent->addChild(colorDropdown);

    auto menu = Build<CCMenu>::create()
        .contentSize({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(parent)
        .collect();

    float const toggleX = kLeftX + ddW * 2.f + 12.f;

    m_boldSpr = ButtonSprite::create("B", 14, true, "bigFont.fnt", "GJ_button_04.png", 26.f, 0.6f);
    m_boldSpr->setScale(0.75f);
    Build(m_boldSpr)
        .intoMenuItem([this] { this->setBold(!m_bold); })
        .pos({ toggleX + 12.f, y })
        .parent(menu);

    m_italicSpr = ButtonSprite::create("I", 14, true, "bigFont.fnt", "GJ_button_04.png", 26.f, 0.6f);
    m_italicSpr->setScale(0.75f);
    Build(m_italicSpr)
        .intoMenuItem([this] { this->setItalic(!m_italic); })
        .pos({ toggleX + 38.f, y })
        .parent(menu);

    this->setBold(false);
    this->setItalic(false);
}

void AddAnnouncementPopup::buildComposer(CCNode* parent, float y) {
    m_pieceInput = Build(TextInput::create(232.f, "Type text, then press Add", "chatFont.fnt"))
        .scale(0.8f)
        .pos({ kLeftX + 93.f, y })
        .parent(parent)
        .id("piece-input")
        .collect();
    m_pieceInput->setCommonFilter(CommonFilter::Any);
    m_pieceInput->setMaxCharCount(500);

    auto menu = Build<CCMenu>::create()
        .contentSize({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(parent)
        .collect();

    auto addBtn = smallButton("Add", "GJ_button_01.png", 0.6f, [this] { this->onAddText(); });
    addBtn->setPosition({ kLeftX + 220.f, y });
    menu->addChild(addBtn);

    float const toolsY = y - 28.f;

    auto newLineBtn = smallButton("New Line", "GJ_button_04.png", 0.5f, [this] { this->onNewLine(); });
    newLineBtn->setPosition({ kLeftX + 38.f, toolsY });
    menu->addChild(newLineBtn);

    auto undoBtn = smallButton("Undo", "GJ_button_04.png", 0.5f, [this] { this->onUndoPiece(); });
    undoBtn->setPosition({ kLeftX + 108.f, toolsY });
    menu->addChild(undoBtn);

    auto clearBtn = smallButton("Clear", "GJ_button_06.png", 0.5f, [this] { this->onClearAll(); });
    clearBtn->setPosition({ kLeftX + 168.f, toolsY });
    menu->addChild(clearBtn);
}

void AddAnnouncementPopup::onAddText() {
    std::string text = m_pieceInput->getString();
    if (text.empty()) {
        Notification::create("Type some text first", NotificationIcon::Info)->show();
        return;
    }

    if (m_selected < 0 || m_selected >= static_cast<int>(m_lines.size()) || m_lines[m_selected].kind == LineKind::Divider) {
        m_lines.push_back(Line { m_kind == LineKind::Divider ? LineKind::Text : m_kind, {} });
        m_selected = static_cast<int>(m_lines.size()) - 1;
    }

    m_lines[m_selected].spans.push_back(Span { text, m_color, m_bold, m_italic });
    m_pieceInput->setString("");
    this->refresh();
}

void AddAnnouncementPopup::onNewLine() {
    Line line { m_kind, {} };
    int at = (m_selected >= 0 && m_selected < static_cast<int>(m_lines.size())) ? m_selected + 1 : static_cast<int>(m_lines.size());
    m_lines.insert(m_lines.begin() + at, line);
    m_selected = at;
    this->refresh();
}

void AddAnnouncementPopup::onUndoPiece() {
    if (m_selected < 0 || m_selected >= static_cast<int>(m_lines.size())) {
        if (m_lines.empty()) return;
        m_selected = static_cast<int>(m_lines.size()) - 1;
    }

    auto& line = m_lines[m_selected];
    if (!line.spans.empty()) {
        line.spans.pop_back();
    }
    if (line.spans.empty()) {
        this->deleteLine(m_selected);
        return;
    }
    this->refresh();
}

void AddAnnouncementPopup::onClearAll() {
    if (m_lines.empty() && m_titleTextInput->getString().empty()) return;

    Ref<AddAnnouncementPopup> self = this;
    createQuickPopup(
        "Clear draft",
        "Remove the title and <cr>all lines</c>?",
        "Cancel", "Clear",
        [self](auto, bool confirmed) {
            if (!confirmed || !self) return;
            self->m_lines.clear();
            self->m_selected = -1;
            self->m_titleTextInput->setString("");
            self->refresh();
        }
    );
}

void AddAnnouncementPopup::onPreview() {
    MDPopup::create(
        "Preview Announcement",
        fmt::format(
            "# {}\n\n"
            "{}\n"
            "{}",
            m_titleTextInput->getString(),
            this->buildMarkdown(),
            GJAccountManager::sharedState()->m_username
        ),
        "OK"
    )->show();
}

void AddAnnouncementPopup::onPost() {
    if (m_posting) return;

    std::string title = m_titleTextInput->getString();
    std::string content = this->buildMarkdown();

    if (title.empty()) {
        Notification::create("Add a title first", NotificationIcon::Error)->show();
        return;
    }
    if (content.empty()) {
        Notification::create("The announcement is empty", NotificationIcon::Error)->show();
        return;
    }

    Ref<AddAnnouncementPopup> self = this;
    createQuickPopup(
        "Post announcement?",
        fmt::format("<cy>{}</c> will be shown to <cg>everyone</c>.", title),
        "Cancel", "Post",
        [self, title, content](auto, bool confirmed) {
            if (!confirmed || !self) return;

            self->m_posting = true;
            auto uPopup = UploadActionPopup::create(nullptr, "Adding announcement...");
            uPopup->show();
            Ref<UploadActionPopup> uPopupRef = uPopup;

            self->m_listener.spawn(
                APIClient::getInstance().addAnnouncement(title, content),
                [self, uPopupRef](web::WebResponse res) {
                    if (!self) return;
                    self->m_posting = false;

                    auto parsed = APIClient::getInstance().addAnnouncementParse(res);
                    if (!parsed.ok) {
                        log::error("bad web req. add announcement endpoint");
                        if (uPopupRef) uPopupRef->showFailMessage("Failed! Try again later.");
                        return;
                    }

                    if (uPopupRef) uPopupRef->showSuccessMessage("Success! Announcement added.");

                    self->m_lines.clear();
                    self->m_selected = -1;
                    self->m_titleTextInput->setString("");
                    self->refresh();
                }
            );
        }
    );
}

void AddAnnouncementPopup::setKind(LineKind kind) {
    m_kind = kind;
    if (m_selected >= 0 && m_selected < static_cast<int>(m_lines.size())) {
        m_lines[m_selected].kind = kind;
        this->refresh(false);
    }
}

void AddAnnouncementPopup::setBold(bool bold) {
    m_bold = bold;
    if (m_boldSpr) m_boldSpr->setColor(bold ? ccColor3B{ 120, 255, 120 } : ccColor3B{ 255, 255, 255 });
}

void AddAnnouncementPopup::setItalic(bool italic) {
    m_italic = italic;
    if (m_italicSpr) m_italicSpr->setColor(italic ? ccColor3B{ 120, 255, 120 } : ccColor3B{ 255, 255, 255 });
}

void AddAnnouncementPopup::selectLine(int index) {
    m_selected = (m_selected == index) ? -1 : index;
    this->refresh(false);
}

void AddAnnouncementPopup::deleteLine(int index) {
    if (index < 0 || index >= static_cast<int>(m_lines.size())) return;
    m_lines.erase(m_lines.begin() + index);
    if (m_selected == index) m_selected = -1;
    else if (m_selected > index) m_selected--;
    this->refresh(false);
}

void AddAnnouncementPopup::moveLine(int index, int delta) {
    int to = index + delta;
    if (index < 0 || to < 0 || index >= static_cast<int>(m_lines.size()) || to >= static_cast<int>(m_lines.size())) return;
    std::swap(m_lines[index], m_lines[to]);
    if (m_selected == index) m_selected = to;
    else if (m_selected == to) m_selected = index;
    this->refresh(false);
}

void AddAnnouncementPopup::refresh(bool scrollToSelected) {
    this->refreshLineList(scrollToSelected);
    this->refreshPreview();
}

void AddAnnouncementPopup::refreshPreview() {
    std::string md = this->buildMarkdown();
    if (m_preview) {
        std::string title = m_titleTextInput ? m_titleTextInput->getString() : "";
        std::string full = title.empty() ? md : fmt::format("## {}\n\n{}", title, md);
        m_preview->setString(full.empty() ? "<ca>Nothing to show yet</c>" : full.c_str());
    }

    this->saveDraft();
}

void AddAnnouncementPopup::refreshLineList(bool scrollToSelected) {
    if (!m_lineList) return;

    m_lineList->clear();
    if (m_emptyLabel) m_emptyLabel->setVisible(m_lines.empty());

    for (int i = 0; i < static_cast<int>(m_lines.size()); i++) {
        auto const& line = m_lines[i];
        bool selected = i == m_selected;

        auto row = CCNode::create();
        row->setContentSize({ kLeftW, kRowH });

        if (selected) {
            auto hl = CCLayerColor::create({ 120, 255, 120, 45 }, kLeftW, kRowH);
            row->addChild(hl, -1);
        }

        auto menu = CCMenu::create();
        menu->setContentSize(row->getContentSize());
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint({ 0.f, 0.f });
        menu->setPosition({ 0.f, 0.f });
        row->addChild(menu);

        auto tag = CCLabelBMFont::create(kindDef(line.kind).tag, "goldFont.fnt");
        tag->setScale(0.4f);
        tag->setPosition({ 13.f, kRowH / 2.f });
        row->addChild(tag);

        std::string text = plainText(line);
        auto label = CCLabelBMFont::create(text.empty() ? "(empty line)" : text.c_str(), "chatFont.fnt");
        label->setAnchorPoint({ 0.f, 0.5f });
        label->limitLabelWidth(kLeftW - 100.f, 0.55f, 0.2f);
        if (text.empty()) label->setOpacity(120);
        if (!line.spans.empty()) label->setColor(kPalette[std::clamp(line.spans.front().color, 0, kPaletteSize - 1)].color);

        auto hitbox = CCNode::create();
        hitbox->setContentSize({ kLeftW - 90.f, kRowH });
        hitbox->addChild(label);
        label->setPosition({ 0.f, kRowH / 2.f });

        auto selectBtn = CCMenuItemExt::createSpriteExtra(hitbox, [this, i](CCObject*) { this->selectLine(i); });
        selectBtn->m_scaleMultiplier = 1.f;
        selectBtn->setPosition({ 26.f + (kLeftW - 90.f) / 2.f, kRowH / 2.f });
        menu->addChild(selectBtn);

        auto addIcon = [&](const char* frame, float x, float scale, geode::Function<void()> cb) {
            auto spr = CCSprite::createWithSpriteFrameName(frame);
            spr->setScale(scale);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [cb = std::move(cb)](CCObject*) mutable { cb(); });
            btn->setPosition({ x, kRowH / 2.f });
            menu->addChild(btn);
        };

        addIcon("edit_upBtn_001.png", kLeftW - 56.f, 0.55f, [this, i] { this->moveLine(i, -1); });
        addIcon("edit_downBtn_001.png", kLeftW - 36.f, 0.55f, [this, i] { this->moveLine(i, 1); });
        addIcon("GJ_trashBtn_001.png", kLeftW - 13.f, 0.4f, [this, i] { this->deleteLine(i); });

        m_lineList->addCell(row);
    }

    m_lineList->updateLayout(false);

    if (scrollToSelected && m_selected >= 0 && m_selected == static_cast<int>(m_lines.size()) - 1) {
        m_lineList->scrollToBottom();
    } else {
        m_lineList->scrollToTop();
    }
}

std::string AddAnnouncementPopup::buildMarkdown() const {
    std::string out;
    LineKind prev = LineKind::Divider;
    bool first = true;

    for (auto const& line : m_lines) {
        std::string body;
        if (line.kind == LineKind::Divider) {
            body = "---";
        } else {
            for (auto const& span : line.spans) {
                if (!body.empty()) body += " ";
                body += spanMarkdown(span);
            }
            if (body.empty()) continue;
        }

        switch (line.kind) {
            case LineKind::Heading: body = "## " + body; break;
            case LineKind::Subheading: body = "### " + body; break;
            case LineKind::Bullet: body = "- " + body; break;
            default: break;
        }

        if (!first) {
            out += (line.kind == LineKind::Bullet && prev == LineKind::Bullet) ? "\n" : "\n\n";
        }
        out += body;
        prev = line.kind;
        first = false;
    }

    return out;
}

void AddAnnouncementPopup::saveDraft() {
    matjson::Value lines = matjson::Value::array();
    for (auto const& line : m_lines) {
        matjson::Value spans = matjson::Value::array();
        for (auto const& span : line.spans) {
            matjson::Value s;
            s["text"] = span.text;
            s["color"] = span.color;
            s["bold"] = span.bold;
            s["italic"] = span.italic;
            spans.push(s);
        }
        matjson::Value l;
        l["kind"] = static_cast<int>(line.kind);
        l["spans"] = spans;
        lines.push(l);
    }

    matjson::Value draft;
    draft["title"] = m_titleTextInput ? m_titleTextInput->getString() : "";
    draft["lines"] = lines;
    Mod::get()->setSavedValue<matjson::Value>(kDraftKey, draft);
}

void AddAnnouncementPopup::loadDraft() {
    if (!Mod::get()->hasSavedValue(kDraftKey)) return;
    auto draft = Mod::get()->getSavedValue<matjson::Value>(kDraftKey);

    if (m_titleTextInput) m_titleTextInput->setString(draft["title"].asString().unwrapOrDefault());

    m_lines.clear();
    auto lines = draft["lines"].asArray();
    if (!lines) return;

    for (auto const& l : lines.unwrap()) {
        Line line;
        line.kind = static_cast<LineKind>(std::clamp(static_cast<int>(l["kind"].asInt().unwrapOr(0)), 0, static_cast<int>(LineKind::Divider)));
        if (auto spans = l["spans"].asArray()) {
            for (auto const& s : spans.unwrap()) {
                line.spans.push_back(Span {
                    s["text"].asString().unwrapOrDefault(),
                    std::clamp(static_cast<int>(s["color"].asInt().unwrapOr(0)), 0, kPaletteSize - 1),
                    s["bold"].asBool().unwrapOr(false),
                    s["italic"].asBool().unwrapOr(false),
                });
            }
        }
        m_lines.push_back(std::move(line));
    }
}

}