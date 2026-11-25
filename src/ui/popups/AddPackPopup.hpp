#include <Geode/Geode.hpp>
#include <Geode/ui/Label.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class AddPackPopup : public BasePopup {
public:
    static AddPackPopup* create();

private:
    bool init() override;

    void buildPreview();
    void buildInfoPanel(CCNode* panel);
    void buildLevelsPanel(CCNode* panel);

    void stepDifficulty(int delta);
    void setStar(bool star);
    void updatePreview();
    void onAdd();

    bool m_star = true;
    int m_difficulty = 2;
    cocos2d::ccColor3B m_color { 255, 255, 255 };
    bool m_adding = false;

    TextInput* m_titleInput = nullptr;
    TextInput* m_levelInputs[3] = {};

    CCSprite* m_diffIcon = nullptr;
    geode::Label* m_diffLabel = nullptr;
    CCSprite* m_starSpr = nullptr;
    CCSprite* m_moonSpr = nullptr;

    CCSprite* m_previewDiff = nullptr;
    geode::Label* m_previewTitle = nullptr;
    CCSprite* m_previewType = nullptr;

    TaskHolder<web::WebResponse> m_listener;

    ~AddPackPopup() {
        m_listener.cancel();
    }
};

}