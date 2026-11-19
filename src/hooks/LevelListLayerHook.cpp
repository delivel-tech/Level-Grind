#include <Geode/Geode.hpp>
#include <Geode/binding/LevelListLayer.hpp>
#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <UIBuilder.hpp>

#include "../managers/DataManager.hpp"
#include "../ui/popups/ManageListPopup.hpp"
#include "../ui/popups/NotesManagerPopup.hpp"
#include "../ui/popups/NoteViewerPopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class $modify(LevelGrindListLayer, LevelListLayer) {
    bool init(GJLevelList* list) {
        if (!LevelListLayer::init(list)) return false;
        if (!list || list->m_listID <= 0) return true;

        auto menu = this->getChildByIDRecursive("right-side-menu");
        if (!menu) return true;
        auto pos = DataManager::getInstance().getUserPosition();

        auto notes = NotesManagerPopup::notesFor(NotesManagerPopup::Target::List, list->m_listID);
        if (!notes.empty()) {
            std::vector<NoteInfo> view;
            for (auto const& n : notes) view.push_back({ list->m_listID, n.note, n.by });
            Build(CircleButtonSprite::createWithSprite("button_note.png"_spr))
                .scale(0.8f)
                .intoMenuItem([view] { NoteViewerPopup::create(view)->show(); })
                .id("list-notes-btn"_spr)
                .parent(menu);
        }
        if (pos == GrindPosition::Helper) {
            Ref<GJLevelList> ref = list;
            Build(CircleButtonSprite::createWithSprite("button_edit_note.png"_spr, 1.f, CircleBaseColor::Cyan))
                .scale(0.8f)
                .intoMenuItem([ref] {
                    NotesManagerPopup::create(NotesManagerPopup::Target::List, ref->m_listID, ref->m_listName)->show();
                })
                .id("list-notes-manage-btn"_spr)
                .parent(menu);
        }
        menu->updateLayout();
        if (pos != GrindPosition::Admin && pos != GrindPosition::Owner) return true;


        CCSprite* topSpr = CCSprite::create("database_config_spr.png"_spr);
        Build(CircleButtonSprite::create(topSpr, CircleBaseColor::Cyan))
            .with([](CircleButtonSprite* spr) {
                spr->getTopNode()->setPosition({ 24.375f, 23.5f });
                spr->getTopNode()->setScale(1.05f);
            })
            .scale(0.8f)
            .intoMenuItem([this] {
                ManageListPopup::create(m_levelList, m_levels)->show();
            })
            .id("manage-list-btn"_spr)
            .parent(menu)
            .intoParent()
            .updateLayout();
        return true;
    }
};

}
