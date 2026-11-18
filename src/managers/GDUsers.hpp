#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <optional>
#include <string>

using namespace geode::prelude;

namespace levelgrind::gdusers {

struct User {
    int accountId = 0;
    std::string name;
    int icon = 1;
    int color1 = 0;
    int color2 = 3;
    int glowColor = -1;
};

web::WebFuture search(std::string const& query);
web::WebFuture info(int accountId);

std::optional<User> parseSearch(web::WebResponse const& res);
std::optional<User> parseInfo(web::WebResponse const& res);

}
