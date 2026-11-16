#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <cue/RadioLogic.hpp>
#include "../layers/SuggestionsLayer.hpp"
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class SuggestionsFilterPopup : public BasePopup {
public:
    static SuggestionsFilterPopup* create(SuggestionsLayer* owner);

private:
    bool init(SuggestionsLayer* owner);

    void step(int delta);
    void refreshDifficulty();
    void updatePreview();
    void onApply();

    SuggestionsLayer* m_owner = nullptr;
    cue::RadioLogic<SuggestionsLayer::PointsFilterOp> m_radio;
    cue::RadioLogic<int> m_modeRadio;
    TextInput* m_input = nullptr;
    int m_difficulty = 0;
    CCNode* m_diffHolder = nullptr;
    CCLabelBMFont* m_diffLabel = nullptr;
    CCLabelBMFont* m_preview = nullptr;
};

}