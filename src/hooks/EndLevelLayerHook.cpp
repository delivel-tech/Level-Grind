#include <Geode/Geode.hpp>

#include "../managers/PetManager.hpp"
#include "../managers/GrindClansManager.hpp"

#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>

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

        if (this->m_playLayer && this->m_playLayer->m_level) {
            auto level = this->m_playLayer->m_level;
            auto gsm = GameStatsManager::sharedState();
            if (gsm && gsm->m_verifiedUserCoins && level->m_coinsVerified.value() > 0 && level->m_levelID.value() > 0) {
                std::vector<int> coins;
                for (int i = 1; i <= std::min(level->m_coins, 3); i++) {
                    if (gsm->m_verifiedUserCoins->objectForKey(level->getCoinKey(i))) coins.push_back(i);
                }
                if (!coins.empty()) GrindClansManager::getInstance().queueCoinClaim(level->m_levelID.value(), coins);
            }
        }
        GrindClansManager::getInstance().checkAllPacks();
    }
};

}