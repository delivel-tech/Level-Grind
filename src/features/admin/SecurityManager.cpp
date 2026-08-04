#include "SecurityManager.hpp"
#include "Geode/utils/general.hpp"

namespace levelgrind {

std::vector<uint8_t> SecurityManager::deriveKeystream(size_t length, int accountId) {
    std::string seed = fmt::format("{}:{}", BASE_SECRET, accountId);

    uint32_t state = 0;
    for (char c : seed) state = state * 31u + (uint8_t)c;

    std::vector<uint8_t> keystream;
    keystream.reserve(length);

    for (size_t i = 0; i < length; i++) {
        state = state * 1103515245u + 12345u;
        keystream.push_back(
            (uint8_t)((state >> 16) & 0xFF) ^ (uint8_t)seed[i % seed.size()]
        );
    }

    return keystream;
}

uint32_t SecurityManager::crc32(std::vector<uint8_t> const& data) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint8_t b : data) {
        crc ^= b;
        for (int i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (~(crc & 1u) + 1u));
        }
    }
    return ~crc;
}

std::string SecurityManager::encryptString(std::string const& plain, int accountId) {
    std::vector<uint8_t> raw(plain.begin(), plain.end());

    uint32_t checksum = crc32(raw);
    for (int i = 0; i < 4; i++) {
        raw.push_back((uint8_t)((checksum >> (i * 8)) & 0xFF));
    }

    auto keystream = deriveKeystream(raw.size(), accountId);
    for (size_t i = 0; i < raw.size(); i++) {
        raw[i] ^= keystream[i];
    }

    return geode::utils::base64::encode(raw);
}

std::string SecurityManager::decryptString(std::string const& cipherB64, int accountId) {
    auto decodedRes = geode::utils::base64::decode(cipherB64);
    if (!decodedRes) {
        log::warn("base64 decode failed");
        return "";
    }
    auto raw = decodedRes.unwrap();

    if (raw.size() < 4) return "";

    auto keystream = deriveKeystream(raw.size(), accountId);
    for (size_t i = 0; i < raw.size(); i++) {
        raw[i] ^= keystream[i];
    }

    std::vector<uint8_t> payload(raw.begin(), raw.end() - 4);
    uint32_t storedChecksum = 0;
    for (int i = 0; i < 4; i++) {
        storedChecksum |= ((uint32_t)raw[raw.size() - 4 + i]) << (i * 8);
    }

    if (crc32(payload) != storedChecksum) {
        log::warn("checksum mismatch, data likely tampered");
        return "";
    }

    return std::string(payload.begin(), payload.end());
}

void SecurityManager::setEncryptedInt(std::string const& key, int value, int accountId) {
    Mod::get()->setSavedValue(key, encryptString(fmt::format("{}", value), accountId));
}

int SecurityManager::getEncryptedInt(std::string const& key, int accountId, int defaultValue) {
    auto stored = Mod::get()->getSavedValue<std::string>(key);
    if (stored.empty()) return defaultValue;

    auto decrypted = decryptString(stored, accountId);
    if (decrypted.empty()) return defaultValue;

    try {
        return utils::numFromString<int>(decrypted).unwrapOrDefault();
    } catch (...) {
        return defaultValue;
    }
}

std::string SecurityManager::buildEncryptedPayload(matjson::Value const& body, int accountId) {
    return encryptString(body.dump(), accountId);
}

}