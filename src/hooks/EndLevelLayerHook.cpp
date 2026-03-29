#include <Geode/Geode.hpp>

#include "../other/PetManager.hpp"

#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(PetELL, EndLevelLayer) {
    void customSetup() {
        EndLevelLayer::customSetup();

        PetManager::get()->updatePetDeltasOnCompletion(this->m_stars, this->m_moons);
    }
};