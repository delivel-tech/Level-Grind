#include <Geode/Geode.hpp>
#include "../BaseLayer.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

namespace levelgrind {

class CreatorLayer : public BaseLayer {
public:
    static CreatorLayer* create();

private:
    bool init() override;
    bool initFarMenus();
    bool initMd();

    bool initCategoryScroll(std::initializer_list<CCMenuItemSpriteExtra*> topRow,
        std::initializer_list<CCMenuItemSpriteExtra*> bottomRow);

    Ref<CCGLProgram> m_categoryFade;
    CCMenu* m_catMenu = nullptr;
    std::vector<Ref<CCMenuItemSpriteExtra>> m_cats;

    TaskHolder<web::WebResponse> m_listener;

    void updateBadges();
    void applyConsent();
    void onEnterTransitionDidFinish() override;
    bool m_consentAsked = false;
    void tickBadges(float);
    void onEnter() override;
    bool m_badgesAsked = false;

    ~CreatorLayer() {
        m_listener.cancel();
    }
};

}
