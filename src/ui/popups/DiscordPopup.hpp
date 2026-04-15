#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class DiscordPopup : public BasePopup {
public:
    static DiscordPopup* create();

private:
    bool init() override;
};

}

