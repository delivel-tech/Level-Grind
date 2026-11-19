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

    static constexpr size_t kCatsPerPage = 8;
    void showCatPage(int page);
    int catPageCount() const;

    CCMenu* m_catMenu = nullptr;
    std::vector<Ref<CCMenuItemSpriteExtra>> m_cats;
    int m_catPage = 0;
    CCMenuItemSpriteExtra* m_prevPageBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextPageBtn = nullptr;

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