#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class NotesManagerPopup : public BasePopup {
public:
    enum class Target { Level, List };
    struct Entry {
        long long id = 0;
        std::string note;
        std::string by;
    };

    static NotesManagerPopup* create(Target target, int id, std::string const& name);
    static std::vector<Entry> notesFor(Target target, int id);

private:
    bool init(Target target, int id, std::string const& name);
    void build();
    void openEditor(std::optional<size_t> index);
    void send(std::string const& endpoint, matjson::Value body, std::string const& loading, geode::Function<void(matjson::Value const&)> done);

    Target m_target = Target::Level;
    int m_id = 0;
    std::string m_name;
    std::vector<Entry> m_notes;
    cue::ListNode* m_list = nullptr;
    CCLabelBMFont* m_empty = nullptr;
    TaskHolder<web::WebResponse> m_task;

    ~NotesManagerPopup() { m_task.cancel(); }
};

class NoteEditPopup : public BasePopup {
public:
    static NoteEditPopup* create(std::string const& title, std::string const& initial, geode::Function<void(std::string const&)> onSave);

private:
    bool init(std::string const& title, std::string const& initial, geode::Function<void(std::string const&)> onSave);

    TextInput* m_input = nullptr;
    geode::Function<void(std::string const&)> m_onSave;
};

}
