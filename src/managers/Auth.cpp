#include "Auth.hpp"

#include <dankmeme.argon/argon.hpp>

namespace levelgrind::auth {

arc::Future<std::string> token() {
    auto account = argon::getGameAccountData();
    return [](argon::AccountData account) -> arc::Future<std::string> {
        argon::_overrides overrides = argon::g_overrides;
        overrides.account = std::move(account);
        auto res = co_await argon::startAuth({ ._overrides = overrides });
        if (!res) {
            log::warn("Argon: {}", res.unwrapErr());
            co_return std::string{};
        }
        co_return std::move(res).unwrap();
    }(std::move(account));
}

Future post(std::string url, matjson::Value body, std::optional<std::chrono::seconds> timeout) {
    return [](std::string url, matjson::Value body, std::optional<std::chrono::seconds> timeout,
              arc::Future<std::string> tokenFuture) -> Future {
        body["token"] = co_await std::move(tokenFuture);
        auto req = web::WebRequest();
        if (timeout) req.timeout(*timeout);
        req.bodyJSON(body);
        co_return co_await req.post(std::move(url));
    }(std::move(url), std::move(body), timeout, token());
}

}
