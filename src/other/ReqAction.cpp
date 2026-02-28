#include "../other/ReqAction.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

void lg::runReqCheck() {
    matjson::Value body;
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");

    web::WebRequest req;
    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Loading...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    async::spawn(
        req.post("https://delivel.tech/grindapi/check_helper_new"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                log::error("req failed");
                popupRef->showFailMessage("Request failed! Try again later.");
                Mod::get()->setSavedValue("isHelper", false);
                Mod::get()->setSavedValue("isAdmin", false);
                return;
            }
            auto json = res.json().unwrapOrDefault();
            auto position = json["pos"].asInt().unwrapOrDefault();
            if (position == 1) {
                popupRef->showSuccessMessage("Success! Helper granted.");
                Mod::get()->setSavedValue("isHelper", true);
                Mod::get()->setSavedValue("isAdmin", false);
            } else if (position == 2) {
                popupRef->showSuccessMessage("Success! Admin granted.");
                Mod::get()->setSavedValue("isAdmin", true);
                Mod::get()->setSavedValue("isHelper", false);
            } else {
                popupRef->showFailMessage("Failed! User is not a helper.");
                Mod::get()->setSavedValue("isHelper", false);
                Mod::get()->setSavedValue("isAdmin", false);
            }
        }
    );
}