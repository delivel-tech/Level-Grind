#include "../BaseLayer.hpp"

using namespace geode::prelude;

namespace levelgrind {

class SettingsLayer : public BaseLayer {
public:
    static SettingsLayer* create();

private:
    bool init() override;
};

}