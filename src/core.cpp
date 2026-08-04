#include "core/BackendManager.hpp"
#include "core/DataManager.hpp"
#include "core/SettingsLayer.hpp"
#include <Geode/utils/async.hpp>

using namespace geode::prelude;
using namespace levelgrind;

namespace {
arc::Future<> performBootupGet() {
    auto data = co_await BackendManager::getInstance().bootupGet();
    DataManager::getInstance().setSharedData(data);
    co_return;
}
}

$execute {
    SettingsLayer::initSettings();
    async::spawn(performBootupGet());
    Loader::get()->queueInMainThread([] {
        BackendManager::getInstance().performGetToken();
    });
    DataManager::getInstance().initPermsOnBootup();
    DataManager::getInstance().initTokenOnBootup();

    // waiting for badgify to settle, using badges api temporarily

    /*
    alpha::badgify::registerBadge(
        "grind-owner-badge"_spr,
        "Grind Owner",
        "# <cp>Owner Role</c>\n\n"
        "<cp>Owners</c> are <cy>responsible for</c> leading the <cg>Level Grind</c> project.\n"
        "They manage <cr>development</c>, make <cj>final decisions</c>, and oversee all roles.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (badge.user.data()->m_accountID != 13678537) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool ownerFound =
                std::find(shared.owners.begin(), shared.owners.end(), badge.user.data()->m_accountID)
                != shared.owners.end();

            if (!ownerFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_owner.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_owner_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-co-owner-badge"_spr,
        "Grind Co-Owner",
        "# <cp>Owner Role</c>\n\n"
        "<cp>Owners</c> are <cy>responsible for</c> leading the <cg>Level Grind</c> project.\n"
        "They manage <cr>development</c>, make <cj>final decisions</c>, and oversee all roles.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (badge.user.data()->m_accountID == 13678537) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();
			
			bool ownerFound =
                std::find(shared.owners.begin(), shared.owners.end(), badge.user.data()->m_accountID)
                != shared.owners.end();

            if (!ownerFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_owner.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_owner_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-admin-badge"_spr,
        "Grind Admin",
        "# <cc>Admin Role</c>\n\n"
        "<cc>Admins</c> manage the <cp>Level Grind</c> <cl>database</c>.\n\n"
        "They are responsible for <cg>adding</c> and <co>deleting</c> levels, selecting <cr>Event</c> levels, choosing <cy>Weekly Achievements</c>, and more.\n\n"
        "They also manage the <cg>Helper</c> team to ensure the project stays organized.\n",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

            auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

			bool adminFound =
                std::find(shared.admins.begin(), shared.admins.end(), badge.user.data()->m_accountID)
                != shared.admins.end();

            if (!adminFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_admin.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_admin_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-helper-badge"_spr,
        "Grind Helper",
        "# <cg>Helper Role</c>\n\n"
        "<cg>Helpers</c> are an integral part of the <cp>Level Grind</c> mod.\n\n"
        "They are responsible for suggesting levels, providing helpful <ca>Level Notes</c>, and more.\n\n"
        "You can join our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server to learn more about the role and how you can become one too! :)\n",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool helperFound =
                std::find(shared.helpers.begin(), shared.helpers.end(), badge.user.data()->m_accountID)
                != shared.helpers.end();

            if (!helperFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_helper.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_helper_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-contributor-badge"_spr,
        "Grind Contributor",
        "# <cy>Contributor Role</c>\n\n"
        "<cy>Contributors</c> are the people who have supported the project through <cy>Boosty</c> donations or by providing other meaningful help to the <cp>Level Grind</c> mod.\n",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool contFound =
                std::find(shared.contributors.begin(), shared.contributors.end(), badge.user.data()->m_accountID)
                != shared.contributors.end();

            if (!contFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_contributor.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_contributor_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-artist-badge"_spr,
        "Grind Artist",
        "# <cp>Artist Role</c>\n\n"
        "<cp>Artists</c> are responsible for <cy>visual part of the mod</c>, including sprites, textures, and more.\n"
        "Their support is <cg>greatly appreciated <3</c>",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool artistFound =
                std::find(shared.artists.begin(), shared.artists.end(), badge.user.data()->m_accountID)
                != shared.artists.end();

            if (!artistFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_artist.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_artist_big.png"_spr));
            }
        }
    );
    alpha::badgify::registerBadge(
        "grind-booster-badge"_spr,
        "Grind Booster",
		"# <ca>Booster Role</c>\n\n"
        "<ca>Boosters</c> help our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server by providing server <ca>boosts</c>.\n\n"
        "Their support is greatly appreciated! :3\n",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;

			auto& dm = DataManager::getInstance();
            auto shared = dm.getSharedData();

            bool boosterFound =
                std::find(shared.boosters.begin(), shared.boosters.end(), badge.user.data()->m_accountID)
                != shared.boosters.end();

            if (!boosterFound) return;

            if (badge.location == alpha::badgify::Location::Profile || badge.location == alpha::badgify::Location::Comment) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_booster.png"_spr));
            }
            if (badge.location == alpha::badgify::Location::InfoPopup) {
                alpha::badgify::showBadge(badge, CCSprite::create("badge_booster_big.png"_spr));
            }
        }
    );
    */
}