#include <Geode/Geode.hpp>
#include <Geode/binding/LevelListLayer.hpp>
#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <UIBuilder.hpp>

#include "../managers/DataManager.hpp"
#include "../ui/popups/ManageListPopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class $modify(LevelGrindListLayer, LevelListLayer) {
    bool init(GJLevelList* list) {
        if (!LevelListLayer::init(list)) return false;
        if (!list || list->m_listID <= 0) return true;

        auto pos = DataManager::getInstance().getUserPosition();
        if (pos != GrindPosition::Admin && pos != GrindPosition::Owner) return true;

        auto menu = this->getChildByIDRecursive("right-side-menu");
        if (!menu) return true;

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
