#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <arc/future/Future.hpp>
#include <asp/time/Duration.hpp>
#include <chrono>
#include <optional>

using namespace geode::prelude;

namespace levelgrind::auth {

using Future = arc::Future<web::WebResponse>;

arc::Future<std::string> token();

// its chrono seconds because it was like that before and now i just do compat stuff and convert to asp::Duration in the cpp file
Future post(std::string url, matjson::Value body, std::optional<std::chrono::seconds> timeout = std::nullopt);

}