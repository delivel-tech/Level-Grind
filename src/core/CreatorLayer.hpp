#include <Geode/Geode.hpp>
#include "../shared/BaseLayer.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/utils/async.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <arc/future/Future.hpp>
#include <functional>
#include <vector>
#include <string>

using namespace geode::prelude;

namespace levelgrind {

class CreatorLayer : public BaseLayer {
public:
    static CreatorLayer* create();

private:
    bool init() override;
    bool initFarMenus();
    bool initMd();

    struct CategoryEntry {
        std::function<CCSprite*()> spriteBuilder;
        std::function<void()> callback;
        std::string id;
    };

    static constexpr int CATS_PER_PAGE = 8;

    std::vector<CategoryEntry> m_categories;
    CCMenu* m_catMenu = nullptr;
    CCMenu* m_catArrowMenu = nullptr;
    CCMenuItemSpriteExtra* m_catArrowLeft = nullptr;
    CCMenuItemSpriteExtra* m_catArrowRight = nullptr;
    CCLabelBMFont* m_catPageLabel = nullptr;
    int m_catPage = 0;

    void buildCategoryList();
    CCSprite* buildPetCategorySprite();
    void setupCategoryMenu();
    void updateCategoryPage();
    void turnCategoryPage(int dir);
    int getCategoryPageCount();

    arc::Future<> onLoadServerHealth(Ref<CCLabelBMFont> serverLabelRef);
};

}