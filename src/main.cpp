#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCTextInputNode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <argon/argon.hpp>
#include <fmt/format.h>

#include "other/LGManager.hpp"

#include <alphalaneous.badgify/include/Badgify.hpp>

using namespace geode::prelude;

namespace {
void initializeDefaultSettingsIfNeeded() {
	auto mod = Mod::get();
	auto bgColor = mod->getSavedValue<cocos2d::ccColor3B>("rgbBackground");
	auto bgSpeed = mod->getSavedValue<float>("background-speed");

	bool looksUninitialized =
		bgColor.r == 0 && bgColor.g == 0 && bgColor.b == 0 &&
		bgSpeed == 0.f &&
		!mod->getSavedValue<bool>("only-uncompleted") &&
		!mod->getSavedValue<bool>("only-completed") &&
		!mod->getSavedValue<bool>("newer-first") &&
		!mod->getSavedValue<bool>("recently-added") &&
		!mod->getSavedValue<bool>("hide-bar") &&
		!mod->getSavedValue<bool>("hide-completion-info") &&
		!mod->getSavedValue<bool>("disable-star-particles") &&
		!mod->getSavedValue<bool>("disable-custom-background") &&
		!mod->getSavedValue<bool>("no-badge-for-mods") &&
		!mod->getSavedValue<bool>("disable-badges");

	if (!looksUninitialized) {
		return;
	}

	mod->setSavedValue("only-uncompleted", false);
	mod->setSavedValue("only-completed", false);
	mod->setSavedValue("newer-first", true);
	mod->setSavedValue("recently-added", false);
	mod->setSavedValue("hide-bar", false);
	mod->setSavedValue("hide-completion-info", false);

	mod->setSavedValue("disable-star-particles", false);
	mod->setSavedValue("rgbBackground", cocos2d::ccColor3B { 32, 69, 125 });
	mod->setSavedValue("background-speed", 0.5f);
	mod->setSavedValue("disable-custom-background", false);
	mod->setSavedValue("no-badge-for-mods", true);
	mod->setSavedValue("disable-badges", false);
}
}

$on_mod(Loaded) {
	initializeDefaultSettingsIfNeeded();

	LGManager::get()->authArgon();

	LGManager::get()->fetchBootup();
}

$on_mod(Loaded) {
    alpha::badgify::registerBadge(
        "grind-owner-badge"_spr,
        "Grind Owner",
        "This user is <cy>an Owner</c> of the <cp>Level Grind</c> mod. They are <cg>responsible for leading the project</c>, <cj>development</c>, <cy>making final decisions</c> etc.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;
			if (badge.user.data()->m_accountID != 13678537) return;

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
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;
			if (badge.user.data()->m_accountID == 13678537) return;
			
			const auto& owners = LGManager::get()->getStaff().owners;
			if (std::find(owners.begin(), owners.end(), badge.user.data()->m_accountID) == owners.end()) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_owner.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-admin-badge"_spr,
        "Grind Owner",
        "This user is an <cg>Admin</c> on the <cp>Level Grind</c> mod. They can do everything a <cj>Helper</c> can do, but they can also manage the staff team on the <cp>Level Grind</c> mod.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& admins = LGManager::get()->getStaff().admins;
			if (std::find(admins.begin(), admins.end(), badge.user.data()->m_accountID) == admins.end()) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_admin.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-admin-badge"_spr,
        "Grind Owner",
        "This user is an <cg>Admin</c> on the <cp>Level Grind</c> mod. They can do everything a <cj>Helper</c> can do, but they can also manage the staff team on the <cp>Level Grind</c> mod.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& admins = LGManager::get()->getStaff().admins;
			if (std::find(admins.begin(), admins.end(), badge.user.data()->m_accountID) == admins.end()) return;

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
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& helpers = LGManager::get()->getStaff().helpers;
			if (std::find(helpers.begin(), helpers.end(), badge.user.data()->m_accountID) == helpers.end()) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_admin.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-contributor-badge"_spr,
        "Grind Contributor",
        "This user is a <cg>Contributor</c> on the <cp>Level Grind</c> mod. They help the project through various contributions, such as art, testing and so on.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& conts = LGManager::get()->getStaff().contributors;
			if (std::find(conts.begin(), conts.end(), badge.user.data()->m_accountID) == conts.end()) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_contributor.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
    alpha::badgify::registerBadge(
        "grind-contributor-badge"_spr,
        "Grind Artist",
        "This user is an <cp>Artist</c> on the <cy>Level Grind</c> mod. They are <cr>responsible for the visual part</c> of the mod.",
        [] (const alpha::badgify::Badge& badge) {
			if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& artists = LGManager::get()->getStaff().artists;
			if (std::find(artists.begin(), artists.end(), badge.user.data()->m_accountID) == artists.end()) return;

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
			if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && static_cast<int>(badge.modStatus) != 0) return;

			const auto& boosters = LGManager::get()->getStaff().boosters;
			if (std::find(boosters.begin(), boosters.end(), badge.user.data()->m_accountID) == boosters.end()) return;

            CCSprite* theBadgeSprite = CCSprite::create("badge_booster.png"_spr);
			alpha::badgify::showBadge(badge, theBadgeSprite);
        }
    );
}
