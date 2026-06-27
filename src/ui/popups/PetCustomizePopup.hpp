#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"

#include "../../managers/PetManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

class PetCustomisePopup : public BasePopup {
public:
    static PetCustomisePopup* create(PetManager::PetData petData);

private:
    bool init(PetManager::PetData petData);

    enum Rarity {
        Common = 1, Rare = 2, Legend = 3, Mythic = 4
    };

    Rarity m_rarity;
};

}