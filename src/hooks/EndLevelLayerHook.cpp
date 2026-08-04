#include <Geode/Geode.hpp>

#include "../features/pets/PetManager.hpp"

#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(PetELL, EndLevelLayer) {
    void customSetup() {
        EndLevelLayer::customSetup();

        PetManager::getInstance().updatePetDeltasOnCompletion(this->m_stars, this->m_moons);
    }
};

}