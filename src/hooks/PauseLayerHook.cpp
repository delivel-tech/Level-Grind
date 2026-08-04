#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include "../core/DataManager.hpp"
#include "Geode/ui/BasedButtonSprite.hpp"
#include "../features/levels/NoteViewerPopup.hpp"

#include <Geode/modify/PauseLayer.hpp>

#include <UIBuilder.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(GrindPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto p1 = PlayLayer::get();
        if (!p1 || !p1->m_level) return;

        int levelID = p1->m_level->m_levelID;

        auto& dm = DataManager::getInstance();
        auto shared = dm.getSharedData();

        auto range = shared.notes.equal_range(levelID);

        std::vector<NoteInfo> notes_vec;

        for (auto it = range.first; it != range.second; ++it) {
            notes_vec.push_back(it->second);
        }

        if (notes_vec.empty()) return;

        auto noteBtn = Build(CircleButtonSprite::createWithSprite("button_note.png"_spr))
            .scale(0.625f)
            .intoMenuItem([notes_vec] {
                NoteViewerPopup::create(notes_vec)->show();
            })
            .collect();

        auto rightButtonMenu = getChildByIDRecursive("right-button-menu");
        if (!rightButtonMenu)
            return;

        rightButtonMenu->addChild(noteBtn);
        rightButtonMenu->updateLayout();
    }
};

}