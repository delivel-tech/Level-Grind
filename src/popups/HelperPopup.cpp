#include "../popups/HelperPopup.hpp"

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <Geode/binding/UploadMessageDelegate.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

HelperPopup* HelperPopup::create(GJGameLevel *level) {
    auto ret = new HelperPopup;
    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool HelperPopup::init(GJGameLevel* level) {
    if (!Popup::init(240.f, 180.f)) return false;

    star = false;
    moon = false;
    demon = false;
    coin = false;

    levelID = level->m_levelID;
    levelName = level->m_levelName;
    levelDifficulty = level->m_stars.value();
    if (levelDifficulty == 10) {
        demonDifficulty = level->m_demonDifficulty;
    }
    
    levelLength = level->m_levelLength;

    if (level->isPlatformer()) {
        moon = true;
    } else {
        star = true;
    }

    if (level->m_stars.value() == 10) {
        demon = true;
    }

    if (Mod::get()->getSavedValue<bool>("isAdmin")) {
        this->setTitle("Admin: Add Level", "bigFont.fnt");
    } else {
        this->setTitle("Helper: Add Level", "bigFont.fnt");
    }

    auto deleteBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Delete", "goldFont.fnt", "GJ_button_06.png"),
        this,
        menu_selector(HelperPopup::onDeleteBtn)
    );
    deleteBtn->setID("delete-btn");

    auto addBtnSpr = ButtonSprite::create("Add", "goldFont.fnt", "GJ_button_01.png");
    
    auto addBtn = CCMenuItemSpriteExtra::create(
        addBtnSpr,
        this,
        menu_selector(HelperPopup::onAddButton)
    );
    addBtn->setID("add-btn");

    auto addBtnMenu = CCMenu::create();
    addBtnMenu->setLayout(RowLayout::create()->setGap(15.f));
    addBtnMenu->addChild(deleteBtn);
    addBtnMenu->addChild(addBtn);
    addBtnMenu->setID("add-btn-menu");

    m_mainLayer->addChildAtPosition(addBtnMenu, Anchor::Bottom, {0.f, 30.f});

    auto optionsMenu = CCMenu::create();
    optionsMenu->setLayout(RowLayout::create()->setGap(15.f));
    optionsMenu->setID("options-menu");

    m_mainLayer->addChildAtPosition(optionsMenu, Anchor::Center);

	auto coinIcon = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");

	coinIcon->setPosition({ 20.f, 20.f });

	auto coinIcon2 = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");

	coinIcon2->setPosition({ 20.f, 20.f });

	auto coinBtnSprOff = CCSprite::create("GJ_button_04.png");

	auto coinBtnSprOn = CCSprite::create("GJ_button_02.png");

	coinBtnSprOn->addChild(coinIcon2);
	coinBtnSprOff->addChild(coinIcon);

	auto coinSwitcher = CCMenuItemToggler::create(
		coinBtnSprOff,
		coinBtnSprOn,
		this,
		menu_selector(HelperPopup::onCoinSwitcher)
	);
	coinSwitcher->setID("coin-switcher");
	optionsMenu->addChild(coinSwitcher);

	optionsMenu->updateLayout();

    addBtnMenu->updateLayout();

    auto checkLoading = LoadingSpinner::create(20.f);
    m_mainLayer->addChildAtPosition(checkLoading, Anchor::Center, {0.f, 40.f});

    matjson::Value body;
    body["id"] = levelID;

    web::WebRequest req;
    req.bodyJSON(body);

    Ref<LoadingSpinner> spinnerRef = checkLoading;
    Ref<HelperPopup> layerRef = this;
    Ref<CCMenuItemToggler> coinSwitcherRef = coinSwitcher;
    Ref<CCMenuItemSpriteExtra> addBtnRef = addBtn;

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/check_level"),
        [spinnerRef, layerRef, coinSwitcherRef, addBtnRef](web::WebResponse res) {
            if (!res.ok()) return;
            if (!spinnerRef) return;
            auto json = res.json().unwrap();

            auto isOK = json["ok"].asBool().unwrapOrDefault();

            if (!isOK) {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
                layerRef->m_mainLayer->addChildAtPosition(spr, Anchor::Center, {0.f, 40.f});
                spinnerRef->removeFromParent();
                return;
            }

            auto exists = json["exists"].asBool().unwrapOrDefault();

            if (!exists) {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
                layerRef->m_mainLayer->addChildAtPosition(spr, Anchor::Center, {0.f, 40.f});
                spinnerRef->removeFromParent();
                return;
            } else {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                layerRef->m_mainLayer->addChildAtPosition(spr, Anchor::Center, {0.f, 40.f});
                spinnerRef->removeFromParent();

                auto parent = addBtnRef->getParent();
                addBtnRef->removeFromParent();

                auto readdBtn = CCMenuItemSpriteExtra::create(
                    ButtonSprite::create("Re-Add", "goldFont.fnt", "GJ_button_01.png"),
                    layerRef,
                    menu_selector(HelperPopup::onAddButton)
                );
                readdBtn->setID("readd-btn");
                parent->addChild(readdBtn);
                parent->updateLayout();
            }

            auto coinValue = json["coin"].asInt().unwrapOrDefault();

            if (coinValue == 1) {
                coinSwitcherRef->toggle(true);
                layerRef->coin = true;
            } else {
                coinSwitcherRef->toggle(false);
                layerRef->coin = false;
            }
        }
    );

    return true;
}

void HelperPopup::onAddButton(CCObject* sender) {
    matjson::Value body;
    auto finalLevelName = levelName.empty() ? std::string("blank name") : levelName;

    body["id"] = levelID;
    body["name"] = finalLevelName;
    body["difficulty"] = levelDifficulty;
    body["length"] = levelLength;

    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");
    body["account_id"] = GJAccountManager::get()->m_accountID;

    if (levelDifficulty == 10) {
        body["demon_difficulty"] = demonDifficulty;
    }

    if (star) body["star"] = star;
    if (moon) body["moon"] = moon;
    if (coin) body["coin"] = coin;
    if (demon) body["demon"] = demon;

    body["added_by"] = GJAccountManager::sharedState()->m_username.c_str();

    web::WebRequest req;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Adding level...");
    upopup->show();

    Ref<UploadActionPopup> popupRef = upopup;

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/new_level"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                popupRef->showFailMessage("Adding level failed. Try again later.");
                log::error("failed adding level");
                return;
            }
            popupRef->showSuccessMessage("Success! Level added.");
        }
    );
}

void HelperPopup::onDeleteBtn(CCObject* sender) {
    matjson::Value body;
    body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");
    body["account_id"] = GJAccountManager::get()->m_accountID;
    body["id"] = levelID;

    auto upopup = UploadActionPopup::create(nullptr, "Deleting level...");
    upopup->show();

    web::WebRequest req;
    req.bodyJSON(body);

    auto popupRef = Ref(upopup);

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/delete_level"),
        [popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (!res.ok()) {
                log::error("req failed");
                popupRef->showFailMessage("Deleting level failed. Try again later.");
                return;
            }
            popupRef->showSuccessMessage("Success! Level deleted.");
        }
    );
}

void HelperPopup::onCoinSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		coin = true;
	} else {
		coin = false;
	}
}
