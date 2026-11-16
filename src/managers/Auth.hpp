#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <arc/future/Future.hpp>
#include <chrono>
#include <optional>

using namespace geode::prelude;

namespace levelgrind::auth {

using Future = arc::Future<web::WebResponse>;

arc::Future<std::string> token();

Future post(std::string url, matjson::Value body, std::optional<std::chrono::seconds> timeout = std::nullopt);

}
