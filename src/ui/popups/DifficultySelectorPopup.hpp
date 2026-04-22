#include <Geode/Geode.hpp>
#include "../layers/MainLayer.hpp"
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class DifficultySelectorPopup : public BasePopup {
public:
    static DifficultySelectorPopup* create(MainLayer* owner);

private:
    MainLayer* m_owner = nullptr;

    bool init(MainLayer* owner);
    
    void onFourBtn(CCObject* sender);
    void onFiveBtn(CCObject* sender);

    void onSixBtn(CCObject* sender);
    void onSevenBtn(CCObject* sender);

    void onEightBtn(CCObject* sender);
    void onNineBtn(CCObject* sender);
};

}