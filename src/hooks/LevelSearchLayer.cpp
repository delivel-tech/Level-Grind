#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

#include "../core/CreatorLayer.hpp"

using namespace geode::prelude;

#include <UIBuilder.hpp>

namespace levelgrind {

class $modify(LevelGrind, LevelSearchLayer) {
    bool init(int type) {
        if (!LevelSearchLayer::init(type)) return false;

        auto menu = getChildByIDRecursive("other-filter-menu");

        Build(CCSprite::create("search_btn_1.png"_spr))
            .scale(0.67f)
            .intoMenuItem([this] {
                auto searchBar = typeinfo_cast<CCTextInputNode*>(getChildByIDRecursive("search-bar"));
		        if (searchBar) {
			        searchBar->onClickTrackNode(false);
		        }
                levelgrind::CreatorLayer::create()->open();
            })
            .id("level-grind-btn")
            .parent(menu)
            .intoParent()
            .updateLayout();

        return true;
    }
};

}