#include "DataManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

void DataManager::setSharedData(BootupGetResponse sharedDataArg) {
    sharedData = std::move(sharedDataArg);
    return;
}

BootupGetResponse const& DataManager::getSharedData() const {
    return sharedData;
}

void DataManager::clearSharedData() {
    sharedData = {};
}

void DataManager::setUserPosition(GrindPosition pos) {
    userPosition = pos;
    Mod::get()->setSavedValue("user-position", static_cast<int>(pos));
    return;
}

GrindPosition DataManager::getUserPosition() {
    return userPosition;
}

GrindPosition DataManager::getUserPosFromSaved() {
    return static_cast<GrindPosition>(Mod::get()->getSavedValue<int>("user-position"));
}

std::vector<DiscordPopup::DiscordPageInfo> DataManager::getDiscordPages() {
    return discordPages;
}

void DataManager::initPermsOnBootup() {
    GrindPosition savedPos = getUserPosFromSaved();
    setUserPosition(savedPos);
}

void DataManager::initTokenOnBootup() {
    Mod::get()->setSavedValue<std::string>("user-token", "");
    Mod::get()->setSavedValue<std::string>("argon_token", "");
}

}