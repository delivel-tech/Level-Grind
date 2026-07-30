#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "../utils/singleton.hpp"

using namespace geode::prelude;

namespace levelgrind {

class SecurityManager : public Singleton<SecurityManager> {
    friend class Singleton<SecurityManager>;

private:
    SecurityManager() = default;

    static constexpr const char* BASE_SECRET = "lg_pet_9F3q!kX02_secureSalt";

    std::vector<uint8_t> deriveKeystream(size_t length, int accountId);
    uint32_t crc32(std::vector<uint8_t> const& data);

public:
    std::string encryptString(std::string const& plain, int accountId);
    std::string decryptString(std::string const& cipherB64, int accountId);

    void setEncryptedInt(std::string const& key, int value, int accountId);
    int getEncryptedInt(std::string const& key, int accountId, int defaultValue = 0);

    std::string buildEncryptedPayload(matjson::Value const& body, int accountId);
};

}