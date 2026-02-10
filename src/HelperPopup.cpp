#include "HelperPopup.hpp"
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
    levelName = level->m_levelName.c_str();
    levelDifficulty = level->m_stars.value();
    if (levelDifficulty == 10) {
        demonDifficulty = level->m_demonDifficulty;
    }
    log::info("Level Difficulty: {}", levelDifficulty);
    levelLength = level->m_levelLength;

    this->setTitle("Helper: Add Level", "bigFont.fnt");

    auto addBtnSpr = ButtonSprite::create("Add");
    
    auto addBtn = CCMenuItemSpriteExtra::create(
        addBtnSpr,
        this,
        menu_selector(HelperPopup::onAddButton)
    );
    addBtn->setID("add-btn");

    auto addBtnMenu = CCMenu::create();
    addBtnMenu->setLayout(RowLayout::create()->setGap(15.f));
    addBtnMenu->addChild(addBtn);
    addBtnMenu->setID("add-btn-menu");

    m_mainLayer->addChildAtPosition(addBtnMenu, Anchor::Bottom, {0.f, 30.f});

    auto optionsMenu = CCMenu::create();
    optionsMenu->setLayout(RowLayout::create()->setGap(15.f));
    optionsMenu->setID("options-menu");

    m_mainLayer->addChildAtPosition(optionsMenu, Anchor::Center);

    auto starIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
	auto moonIcon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
	auto coinIcon = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
	auto demonIcon = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");
	
	starIcon->setPosition({ 20.f, 20.f });
	moonIcon->setPosition({ 20.f, 20.f });
	coinIcon->setPosition({ 20.f, 20.f });
	demonIcon->setPosition({ 20.f, 20.f });

	auto starIcon2 = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
	auto moonIcon2 = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
	auto coinIcon2 = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
	auto demonIcon2 = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");

	starIcon2->setPosition({ 20.f, 20.f });
	moonIcon2->setPosition({ 20.f, 20.f });
	coinIcon2->setPosition({ 20.f, 20.f });
	demonIcon2->setPosition({ 20.f, 20.f });

	auto starBtnSprOff = CCSprite::create("GJ_button_04.png");
	auto moonBtnSprOff = CCSprite::create("GJ_button_04.png");
	auto coinBtnSprOff = CCSprite::create("GJ_button_04.png");
	auto demonBtnSprOff = CCSprite::create("GJ_button_04.png");

	auto starBtnSprOn = CCSprite::create("GJ_button_02.png");
	auto moonBtnSprOn = CCSprite::create("GJ_button_02.png");
	auto coinBtnSprOn = CCSprite::create("GJ_button_02.png");
	auto demonBtnSprOn = CCSprite::create("GJ_button_02.png");

	starBtnSprOn->addChild(starIcon2);
	moonBtnSprOn->addChild(moonIcon2);
	coinBtnSprOn->addChild(coinIcon2);
	demonBtnSprOn->addChild(demonIcon2);
	starBtnSprOff->addChild(starIcon);
	moonBtnSprOff->addChild(moonIcon);
	coinBtnSprOff->addChild(coinIcon);
	demonBtnSprOff->addChild(demonIcon);

	auto starSwitcher = CCMenuItemToggler::create(
		starBtnSprOff,
		starBtnSprOn,
		this,
		menu_selector(HelperPopup::onStarSwitcher)
	);
	starSwitcher->setID("star-switcher");
	optionsMenu->addChild(starSwitcher);

	auto moonSwitcher = CCMenuItemToggler::create(
		moonBtnSprOff,
		moonBtnSprOn,
		this,
		menu_selector(HelperPopup::onMoonSwitcher)
	);
	moonSwitcher->setID("moon-switcher");
	optionsMenu->addChild(moonSwitcher);

	auto coinSwitcher = CCMenuItemToggler::create(
		coinBtnSprOff,
		coinBtnSprOn,
		this,
		menu_selector(HelperPopup::onCoinSwitcher)
	);
	coinSwitcher->setID("coin-switcher");
	optionsMenu->addChild(coinSwitcher);

	auto demonSwitcher = CCMenuItemToggler::create(
		demonBtnSprOff,
		demonBtnSprOn,
		this,
		menu_selector(HelperPopup::onDemonSwitcher)
	);
	demonSwitcher->setID("demon-switcher");
	optionsMenu->addChild(demonSwitcher);

	optionsMenu->updateLayout();

    auto deleteBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Delete"),
        this,
        menu_selector(HelperPopup::onDeleteBtn)
    );
    deleteBtn->setID("delete-btn");
    addBtnMenu->addChild(deleteBtn);

    addBtnMenu->updateLayout();

    auto checkLoading = LoadingSpinner::create(20.f);
    m_mainLayer->addChildAtPosition(checkLoading, Anchor::Center, {0.f, 40.f});

    matjson::Value body;
    body["id"] = levelID;

    web::WebRequest req;
    req.bodyJSON(body);

    m_listener2.spawn(
        req.post("https://delivel.tech/grindapi/check_level"),
        [checkLoading, this](web::WebResponse res) {
            if (!res.ok()) {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
                m_mainLayer->addChildAtPosition(spr, Anchor::Center, {0.f, 40.f});
                checkLoading->removeFromParent();
            } else {
                auto spr = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                m_mainLayer->addChildAtPosition(spr, Anchor::Center, {0.f, 40.f});
                checkLoading->removeFromParent();
            }
        }
    );

    return true;
}

void HelperPopup::onAddButton(CCObject* sender) {
    matjson::Value body;

    body["id"] = levelID;
    body["name"] = levelName;
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

    web::WebRequest req;

    req.bodyJSON(body);

    auto upopup = UploadActionPopup::create(nullptr, "Adding level...");
    upopup->show();

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/new_level"),
        [upopup](web::WebResponse res) {
            if (!res.ok()) {
                upopup->showFailMessage("Adding level failed. Try again later.");
                log::error("failed adding level");
                return;
            }
            upopup->showSuccessMessage("Success! Level added.");
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

    m_listener.spawn(
        req.post("https://delivel.tech/grindapi/delete_level"),
        [upopup](web::WebResponse res) {
            if (!res.ok()) {
                log::error("req failed");
                upopup->showFailMessage("Deleting level failed. Try again later.");
                return;
            }
            upopup->showSuccessMessage("Success! Level deleted.");
        }
    );
}

void HelperPopup::onStarSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		star = true;
	} else {
		star = false;
	}
}

void HelperPopup::onMoonSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		moon = true;
	} else {
		moon = false;
	}
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

void HelperPopup::onDemonSwitcher(CCObject* sender) {
    auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demon = true;
	} else {
		demon = false;
	}
}