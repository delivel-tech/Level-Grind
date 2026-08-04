#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include "SuggestionsLayer.hpp"
#include "../../shared/BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class SuggestionsFilterPopup : public BasePopup {
public:
    static SuggestionsFilterPopup* create(SuggestionsLayer* owner);

private:
    bool init(SuggestionsLayer* owner);

    SuggestionsLayer* m_owner = nullptr;
};

}