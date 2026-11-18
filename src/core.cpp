#include "managers/APIClient.hpp"
#include "managers/DataManager.hpp"
#include "ui/layers/SettingsLayer.hpp"

using namespace geode::prelude;
using namespace levelgrind;

$execute {
    SettingsLayer::initSettings();
    APIClient::getInstance().performBootupGet();
    Loader::get()->queueInMainThread([] {
        APIClient::getInstance().performGetToken();
    });
    DataManager::getInstance().initPermsOnBootup();
    DataManager::getInstance().initTokenOnBootup();
}