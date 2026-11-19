#include "NotesManagerPopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/Auth.hpp"
#include "../../managers/DataManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

constexpr CCSize kPopupSize { 380.f, 250.f };
constexpr CCSize kListSize { 300.f, 150.f };
constexpr float kCellH = 44.f;

CCSprite* fitSprite(const char* frame, float size) {
    auto spr = CCSprite::createWithSpriteFrameName(frame);
    spr->setScale(size / std::max(spr->getContentWidth(), spr->getContentHeight()));
    return spr;
}

std::string preview(std::string text) {
    for (auto& ch : text) {
        if (ch == '\n' || ch == '\r' || ch == '\t') ch = ' ';
    }
    return text;
}

}

std::vector<NotesManagerPopup::Entry> NotesManagerPopup::notesFor(Target target, int id) {
    std::vector<Entry> out;
    auto const& shared = DataManager::getInstance().getSharedData();
    if (target == Target::Level) {
        auto range = shared.notes.equal_range(id);
        for (auto it = range.first; it != range.second; ++it) out.push_back({ 0, it->second.note, it->second.senderUsername });
    } else {
        auto range = shared.listNotes.equal_range(id);
        for (auto it = range.first; it != range.second; ++it) out.push_back({ it->second.id, it->second.note, it->second.senderUsername });
        std::sort(out.begin(), out.end(), [](Entry const& a, Entry const& b) { return a.id < b.id; });
    }
    return out;
}

NotesManagerPopup* NotesManagerPopup::create(Target target, int id, std::string const& name) {
    auto ret = new NotesManagerPopup;
    if (ret->init(target, id, name)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NotesManagerPopup::init(Target target, int id, std::string const& name) {
    if (!BasePopup::init(kPopupSize.width, kPopupSize.height)) return false;
    m_target = target;
    m_id = id;
    m_name = name.empty() ? "Unknown" : name;
    m_notes = notesFor(target, id);
    this->setTitle(target == Target::Level ? "Level Notes" : "List Notes");

    m_list = cue::ListNode::create(kListSize);
    m_list->setPosition({ centerX(), centerY() + 2.f });
    m_list->setAutoUpdate(false);
    m_mainLayer->addChild(m_list);

    m_empty = CCLabelBMFont::create("No notes", "bigFont.fnt");
    m_empty->setScale(0.45f);
    m_empty->setOpacity(160);
    m_empty->setPosition(m_list->getPosition());
    m_mainLayer->addChild(m_empty, 5);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu, 5);
    Ref<NotesManagerPopup> self = this;
    auto addSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    addSpr->setScale(0.7f);
    auto add = CCMenuItemExt::createSpriteExtra(addSpr, [self](CCObject*) {
        if (self) self->openEditor(std::nullopt);
    });
    add->setPosition({ kPopupSize.width - 26.f, 24.f });
    menu->addChild(add);

    this->build();
    return true;
}

void NotesManagerPopup::build() {
    m_list->clear();
    m_empty->setVisible(m_notes.empty());
    float const w = kListSize.width;
    Ref<NotesManagerPopup> self = this;
    for (size_t i = 0; i < m_notes.size(); i++) {
        auto const& e = m_notes[i];
        auto cell = CCMenu::create();
        cell->ignoreAnchorPointForPosition(false);
        cell->setContentSize({ w, kCellH });

        auto text = CCLabelBMFont::create(preview(e.note).c_str(), "chatFont.fnt");
        text->setAnchorPoint({ 0.f, 0.5f });
        text->setPosition({ 12.f, kCellH / 2.f + 7.f });
        text->limitLabelWidth(w - 90.f, 0.7f, 0.3f);
        cell->addChild(text);

        auto by = CCLabelBMFont::create(fmt::format("by {}", e.by).c_str(), "bigFont.fnt");
        by->setAnchorPoint({ 0.f, 0.5f });
        by->setPosition({ 12.f, kCellH / 2.f - 9.f });
        by->limitLabelWidth(w - 90.f, 0.28f, 0.1f);
        by->setOpacity(170);
        cell->addChild(by);

        auto edit = CCMenuItemExt::createSpriteExtra(fitSprite("GJ_editBtn_001.png", 24.f), [self, i](CCObject*) {
            if (self) self->openEditor(i);
        });
        edit->setPosition({ w - 52.f, kCellH / 2.f });
        cell->addChild(edit);

        auto del = CCMenuItemExt::createSpriteExtra(fitSprite("GJ_deleteBtn_001.png", 24.f), [self, i](CCObject*) {
            if (!self || i >= self->m_notes.size()) return;
            createQuickPopup("Delete Note", "Delete this note?", "Cancel", "Delete", [self, i](auto, bool yes) {
                if (!yes || !self || i >= self->m_notes.size()) return;
                auto entry = self->m_notes[i];
                matjson::Value body;
                body["noteID"] = entry.id;
                body["oldNote"] = entry.note;
                self->send("note_delete", std::move(body), "Deleting note...", [self, i](matjson::Value const&) {
                    if (!self || i >= self->m_notes.size()) return;
                    self->m_notes.erase(self->m_notes.begin() + static_cast<long>(i));
                    self->build();
                });
            });
        });
        del->setPosition({ w - 22.f, kCellH / 2.f });
        cell->addChild(del);

        m_list->addCell(cell);
    }
    m_list->updateLayout(false);
    m_list->scrollToTop();
    handleTouchPriority(this);
}

void NotesManagerPopup::openEditor(std::optional<size_t> index) {
    Ref<NotesManagerPopup> self = this;
    std::string initial = index && *index < m_notes.size() ? m_notes[*index].note : "";
    NoteEditPopup::create(index ? "Edit Note" : "Add Note", initial, [self, index](std::string const& text) {
        if (!self) return;
        matjson::Value body;
        body["note"] = text;
        if (index) {
            if (*index >= self->m_notes.size()) return;
            body["noteID"] = self->m_notes[*index].id;
            body["oldNote"] = self->m_notes[*index].note;
            self->send("note_edit", std::move(body), "Saving note...", [self, index, text](matjson::Value const&) {
                if (!self || *index >= self->m_notes.size()) return;
                self->m_notes[*index].note = text;
                self->build();
            });
        } else {
            self->send("note_add", std::move(body), "Adding note...", [self, text](matjson::Value const& res) {
                if (!self) return;
                self->m_notes.push_back({ res["id"].asInt().unwrapOr(0), text, res["addedBy"].asString().unwrapOr(std::string(GJAccountManager::get()->m_username)) });
                self->build();
            });
        }
    })->show();
}

void NotesManagerPopup::send(std::string const& endpoint, matjson::Value body, std::string const& loading, geode::Function<void(matjson::Value const&)> done) {
    body["accountID"] = GJAccountManager::get()->m_accountID;
    body["targetType"] = m_target == Target::Level ? "level" : "list";
    body["targetID"] = m_id;
    body["targetName"] = m_name;

    auto popup = UploadActionPopup::create(nullptr, loading);
    popup->show();
    Ref<UploadActionPopup> popupRef = popup;
    m_task.spawn(auth::post(fmt::format("{}/{}", APIClient::getInstance().baseUrl, endpoint), std::move(body)),
        [popupRef, done = std::move(done)](web::WebResponse res) mutable {
            auto json = res.json();
            bool ok = res.ok() && json && json.unwrap()["ok"].asBool().unwrapOr(false);
            if (!ok) {
                if (popupRef) popupRef->showFailMessage(APIClient::errorMessage(res, "Failed! Try again later."));
                return;
            }
            if (popupRef) popupRef->showSuccessMessage("Done!");
            done(json.unwrap());
            APIClient::getInstance().performBootupGet();
        });
}

NoteEditPopup* NoteEditPopup::create(std::string const& title, std::string const& initial, geode::Function<void(std::string const&)> onSave) {
    auto ret = new NoteEditPopup;
    if (ret->init(title, initial, std::move(onSave))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NoteEditPopup::init(std::string const& title, std::string const& initial, geode::Function<void(std::string const&)> onSave) {
    if (!BasePopup::init(400.f, 140.f)) return false;
    this->setTitle(title.c_str());
    m_onSave = std::move(onSave);

    m_input = TextInput::create(360.f, "Note", "bigFont.fnt");
    m_input->setCommonFilter(CommonFilter::Any);
    m_input->setMaxCharCount(1000);
    m_input->setString(initial);
    m_input->setPosition({ centerX(), centerY() + 4.f });
    m_mainLayer->addChild(m_input);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu);
    auto spr = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    spr->setScale(0.75f);
    Ref<NoteEditPopup> self = this;
    auto btn = CCMenuItemExt::createSpriteExtra(spr, [self](CCObject*) {
        if (!self) return;
        auto text = string::trim(self->m_input->getString());
        if (text.empty()) return;
        if (self->m_onSave) self->m_onSave(text);
        self->onClose(nullptr);
    });
    btn->setPosition({ centerX(), 22.f });
    menu->addChild(btn);
    return true;
}

}
