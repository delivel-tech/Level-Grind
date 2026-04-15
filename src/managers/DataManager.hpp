#pragma once
#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

class DataManager : public Singleton<DataManager> {
    friend class Singleton<DataManager>;

private:
    DataManager() = default;
    BootupGetResponse sharedData;
    GrindPosition userPosition;
    std::string userToken;

public:
    void setSharedData(BootupGetResponse sharedDataArg);
    void setUserPosition(GrindPosition pos);
    void setUserToken(const std::string& token);

    BootupGetResponse getSharedData();
    GrindPosition getUserPosition();
    GrindPosition getUserPosFromSaved();
    std::string getUserToken();
    std::string getUserTokenFromSaved();

    void initPermsOnBootup();
    void initTokenOnBootup();

};

}