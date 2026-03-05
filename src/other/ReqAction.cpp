#include "../other/ReqAction.hpp"
#include "../other/LGManager.hpp"
#include "LGManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

void lg::runReqCheck() {
    matjson::Value body;
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["token"] = LGManager::get()->getArgonToken();

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
                LGManager::get()->setHelper(false);
                LGManager::get()->setAdmin(false);
                return;
            }
            auto json = res.json().unwrapOrDefault();
            auto position = json["pos"].asInt().unwrapOrDefault();
            if (position == 1) {
                popupRef->showSuccessMessage("Success! Helper granted.");
                LGManager::get()->setHelper(true);
                LGManager::get()->setAdmin(false);
            } else if (position == 2) {
                popupRef->showSuccessMessage("Success! Admin granted.");
                LGManager::get()->setAdmin(true);
                LGManager::get()->setHelper(false);
            } else {
                popupRef->showFailMessage("Failed! User is not a helper.");
                LGManager::get()->setHelper(false);
                LGManager::get()->setAdmin(false);
            }
        }
    );
}