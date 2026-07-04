#include "managers/APIClient.hpp"
#include "managers/DataManager.hpp"
#include "ui/layers/SettingsLayer.hpp"

#include <alphalaneous.badgify/include/Badgify.hpp>

using namespace geode::prelude;
using namespace levelgrind;

$execute {
    SettingsLayer::initSettings();
    APIClient::getInstance().performBootupGet();
    APIClient::getInstance().performGetToken();
    DataManager::getInstance().initPermsOnBootup();
    DataManager::getInstance().initTokenOnBootup();

    alpha::badgify::registerBadge(
        "grind-owner-badge"_spr,
        "Grind Owner",
        "This user is <cy>an Owner</c> of the <cp>Level Grind</c> mod. They are <cg>responsible for leading the project</c>, <cj>development</c>, <cy>making final decisions</c> etc.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (badge.user.data()->m_accountID != 13678537) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool ownerFound =
                std::find(shared.owners.begin(), shared.owners.end(), badge.user.data()->m_accountID)
                != shared.owners.end();

            if (!ownerFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_owner.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-co-owner-badge"_spr,
        "Grind Co-Owner",
        "This user is <cy>a Co-Owner</c> of the <cp>Level Grind</c> mod. They are <cg>responsible for leading the project</c>, <cj>development</c>, <cy>making final decisions</c> etc.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (badge.user.data()->m_accountID == 13678537) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();
			
			bool ownerFound =
                std::find(shared.owners.begin(), shared.owners.end(), badge.user.data()->m_accountID)
                != shared.owners.end();

            if (!ownerFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_owner.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-admin-badge"_spr,
        "Grind Admin",
        "This user is an <cg>Admin</c> on the <cp>Level Grind</c> mod. They can do everything a <cj>Helper</c> can do, but they can also manage the staff team on the <cp>Level Grind</c> mod.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

			bool adminFound =
                std::find(shared.admins.begin(), shared.admins.end(), badge.user.data()->m_accountID)
                != shared.admins.end();

            if (!adminFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_admin.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-helper-badge"_spr,
        "Grind Helper",
        "This user is a <cg>Helper</c> on the <cp>Level Grind</c> mod. They <cj>help</c> with adding and <cr>deleting</c> levels on the <cp>Level Grind</c> database.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool helperFound =
                std::find(shared.helpers.begin(), shared.helpers.end(), badge.user.data()->m_accountID)
                != shared.helpers.end();

            if (!helperFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_helper.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-contributor-badge"_spr,
        "Grind Contributor",
        "This user is a <cg>Contributor</c> on the <cp>Level Grind</c> mod. They help the project through various contributions, such as art, testing and so on.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool contFound =
                std::find(shared.contributors.begin(), shared.contributors.end(), badge.user.data()->m_accountID)
                != shared.contributors.end();

            if (!contFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_contributor.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-artist-badge"_spr,
        "Grind Artist",
        "This user is an <cp>Artist</c> on the <cy>Level Grind</c> mod. They are <cr>responsible for the visual part</c> of the mod.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool artistFound =
                std::find(shared.artists.begin(), shared.artists.end(), badge.user.data()->m_accountID)
                != shared.artists.end();

            if (!artistFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_artist.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-booster-badge"_spr,
        "Grind Booster",
		"This user is a <cp>Booster</c> of <cy>Level Grind Discord server</c>. Their <cg>support is greatly appreciated</c>! <cl>[Join the server?](https://discord.gg/tmf5xtCX5y)</c>",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool boosterFound =
                std::find(shared.boosters.begin(), shared.boosters.end(), badge.user.data()->m_accountID)
                != shared.boosters.end();

            if (!boosterFound) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_booster.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
}