#include "managers/APIClient.hpp"
#include "managers/DataManager.hpp"
#include "utils/globals.hpp"
#include "ui/layers/SettingsLayer.hpp"

using namespace geode::prelude;
using namespace levelgrind;

$on_mod(Loaded) {
    SettingsLayer::initSettings();
    APIClient::getInstance().performBootupGet();
    APIClient::getInstance().performGetToken();
    DataManager::getInstance().setUserPosition(GrindPosition::Owner);
    DataManager::getInstance().initPermsOnBootup();
    DataManager::getInstance().initTokenOnBootup();
}