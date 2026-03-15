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

	LGManager::get()->fetchStaff();
}
