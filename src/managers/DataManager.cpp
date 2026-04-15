#include "DataManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

void DataManager::setSharedData(BootupGetResponse sharedDataArg) {
    sharedData = sharedDataArg;
    return;
}

BootupGetResponse DataManager::getSharedData() {
    return sharedData;
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

void DataManager::setUserToken(const std::string& token) {
    userToken = token;
    Mod::get()->setSavedValue("user-token", token);
    return;
}

std::string DataManager::getUserToken() {
    return userToken;
}

std::string DataManager::getUserTokenFromSaved() {
    auto token = Mod::get()->getSavedValue<std::string>("user-token");
    return token;
}

void DataManager::initPermsOnBootup() {
    GrindPosition savedPos = getUserPosFromSaved();
    setUserPosition(savedPos);
}

void DataManager::initTokenOnBootup() {
    std::string savedToken = getUserTokenFromSaved();
    setUserToken(savedToken);
}

}