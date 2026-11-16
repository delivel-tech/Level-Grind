#include <Geode/Geode.hpp>

#include "../managers/PetManager.hpp"
#include "../managers/GrindClansManager.hpp"

#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

namespace levelgrind {

class $modify(PetELL, EndLevelLayer) {
    void customSetup() {
        EndLevelLayer::customSetup();

        PetManager::getInstance().updatePetDeltasOnCompletion(this->m_stars, this->m_moons);

        int reward = this->m_stars > 0 ? this->m_stars : this->m_moons;
        if (reward > 0 && this->m_playLayer && this->m_playLayer->m_level) {
            auto level = this->m_playLayer->m_level;
            GrindClansManager::getInstance().queueLevelClaim(
                level->m_levelID.value(), reward, level->isPlatformer()
            );
        }

        GrindClansManager::getInstance().checkAllPacks();
    }
};

}