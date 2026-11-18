#include "GDUsers.hpp"

#include <fmt/format.h>
#include <unordered_map>

namespace levelgrind::gdusers {

namespace {

constexpr const char* kServer = "https://www.boomlings.com/database/";
constexpr const char* kSecret = "Wmfd2893gb7";

std::string encode(std::string const& in) {
    std::string out;
    for (unsigned char ch : in) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') out += static_cast<char>(ch);
        else out += fmt::format("%{:02X}", ch);
    }
    return out;
}

web::WebFuture send(std::string const& endpoint, std::string body) {
    auto req = web::WebRequest();
    req.userAgent("");
    req.header("Content-Type", "application/x-www-form-urlencoded");
    req.timeout(std::chrono::seconds{ 15 });
    req.bodyString(body);
    return req.post(fmt::format("{}{}", kServer, endpoint));
}

std::unordered_map<int, std::string> fields(std::string_view entry) {
    std::unordered_map<int, std::string> out;
    size_t pos = 0;
    while (pos < entry.size()) {
        size_t keyEnd = entry.find(':', pos);
        if (keyEnd == std::string_view::npos) break;
        size_t valEnd = entry.find(':', keyEnd + 1);
        if (valEnd == std::string_view::npos) valEnd = entry.size();
        int key = numFromString<int>(entry.substr(pos, keyEnd - pos)).unwrapOr(-1);
        out[key] = std::string(entry.substr(keyEnd + 1, valEnd - keyEnd - 1));
        pos = valEnd + 1;
    }
    return out;
}

int num(std::unordered_map<int, std::string> const& f, int key, int fallback) {
    auto it = f.find(key);
    if (it == f.end()) return fallback;
    return numFromString<int>(it->second).unwrapOr(fallback);
}

std::optional<User> build(std::unordered_map<int, std::string> const& f, int iconKey) {
    User u;
    u.accountId = num(f, 16, 0);
    if (u.accountId <= 0) return std::nullopt;
    if (auto it = f.find(1); it != f.end()) u.name = it->second;
    u.icon = std::max(1, num(f, iconKey, 1));
    u.color1 = num(f, 10, 0);
    u.color2 = num(f, 11, 3);
    bool glow = num(f, 28, 0) != 0 || num(f, 15, 0) == 2;
    u.glowColor = glow ? num(f, 51, -1) : -1;
    return u;
}

std::optional<std::string> text(web::WebResponse const& res) {
    if (!res.ok()) return std::nullopt;
    auto str = res.string().unwrapOrDefault();
    if (str.empty() || str == "-1" || str.starts_with("error")) return std::nullopt;
    return str;
}

}

web::WebFuture search(std::string const& query) {
    return send("getGJUsers20.php", fmt::format("secret={}&str={}", kSecret, encode(query)));
}

web::WebFuture info(int accountId) {
    return send("getGJUserInfo20.php", fmt::format("secret={}&targetAccountID={}", kSecret, accountId));
}

std::optional<User> parseSearch(web::WebResponse const& res) {
    auto str = text(res);
    if (!str) return std::nullopt;
    std::string_view list = *str;
    list = list.substr(0, list.find('#'));
    auto first = list.substr(0, list.find('|'));
    auto f = fields(first);
    if (num(f, 14, 0) != 0) f[9] = "1";
    return build(f, 9);
}

std::optional<User> parseInfo(web::WebResponse const& res) {
    auto str = text(res);
    if (!str) return std::nullopt;
    return build(fields(*str), 21);
}

}
