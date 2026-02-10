#include "LevelGrindLayer.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/c++stl/string.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/menu_nodes/CCMenuItem.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/cocos/sprite_nodes/CCSpriteFrameCache.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/async.hpp"
#include "Geode/utils/general.hpp"
#include <Geode/Enums.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/LevelFeatureLayer.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <matjson.hpp>
#include <string>
#include <Geode/utils/web.hpp>
#include "GrindBrowserLayer.hpp"

using namespace geode::prelude;

LevelGrindLayer* LevelGrindLayer::create() {
    auto ret = new LevelGrindLayer();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool LevelGrindLayer::init() {
    if (!CCLayer::init()) return false;

	difficulties.clear();
	lengths.clear();
	grindTypes.clear();
	demonDifficulties.clear();

    this->setKeypadEnabled(true);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    const char* bgFile = "game_bg_01_001.png";

    m_bg1 = CCSprite::create(bgFile);
	m_bg2 = CCSprite::create(bgFile);

    m_bg1->setID("bg-1");
    m_bg2->setID("bg-2");

	m_bg1->setColor({ 34, 60, 110 });
	m_bg2->setColor({34, 60, 110});

    if (!m_bg1 || !m_bg2) {
		return false;
	}

    m_bg1->setAnchorPoint({ 0.f, 0.f });
	m_bg2->setAnchorPoint({ 0.f, 0.f });

	float scaleY = winSize.height / m_bg1->getContentSize().height;
	float scaleX = winSize.width / m_bg1->getContentSize().width;
	float scale = std::max(scaleY, scaleX);
	m_bg1->setScale(scale);
	m_bg2->setScale(scale);

    m_bgWidth = m_bg1->boundingBox().size.width;

	m_bg1->setPosition({ 0.f, 0.f });
	m_bg2->setPosition({ m_bgWidth - 1.f, 0.f });

	this->addChild(m_bg1, -100);
	this->addChild(m_bg2, -100);
    this->scheduleUpdate();

    auto backBtn = CCMenuItemSpriteExtra::create(
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(LevelGrindLayer::onBack)
	);

    backBtn->setPosition(25, winSize.height - 25);
    backBtn->setID("back-btn");

	auto menu = CCMenu::create();
    menu->setID("back-btn-menu");
	menu->addChild(backBtn);
	menu->setPosition(0, 0);
	this->addChild(menu);

	auto logoSpr = CCSprite::create("lg-logo.png"_spr);
	logoSpr->setID("level-grind-logo");

	logoSpr->setScale(1.2f);

	this->addChild(logoSpr);
	logoSpr->setPosition({ winSize.width / 2, 260.f });

	auto panel = CCScale9Sprite::create("GJ_square01.png");
	panel->setID("buttons-panel");

	panel->setInsetLeft(12);
	panel->setInsetRight(12);
	panel->setInsetTop(12);
	panel->setInsetBottom(12);

	panel->setContentSize({210.f, 140.f});
	panel->setPosition({winSize.width / 2, winSize.height / 2});

	this->addChild(panel);

	auto optionsPanel1 = CCScale9Sprite::create("square02_small.png");
	optionsPanel1->setContentSize({ winSize.width / 2, 30.f });
	optionsPanel1->setOpacity(100);
	optionsPanel1->setID("options-panel-first");
	
	this->addChild(optionsPanel1);

	optionsPanel1->setPosition({ winSize.width / 2, winSize.height / 4.6f });

	auto optionsPanel2 = CCScale9Sprite::create("square02_small.png");
	optionsPanel2->setContentSize({ winSize.width / 2, 30.f });
	optionsPanel2->setOpacity(100);
	optionsPanel2->setID("options-panel-second");
	
	this->addChild(optionsPanel2);

	optionsPanel2->setPosition({ winSize.width / 2, winSize.height / 10.f });

	auto lengthsMenu = CCMenu::create();
	lengthsMenu->setLayout(RowLayout::create()->setGap(15.f));
	lengthsMenu->setID("lengths-menu");

	auto lengthShortOff = CCLabelBMFont::create("Short", "bigFont.fnt");
	lengthShortOff->setColor({ 128, 128, 128 });
	auto lengthShortOn = CCLabelBMFont::create("Short", "bigFont.fnt");

	auto lengthShort = CCMenuItemToggler::create(
		lengthShortOff,
		lengthShortOn,
		this,
		menu_selector(LevelGrindLayer::onShort)
	);
	lengthShort->setID("length-short-switcher");
	lengthsMenu->addChild(lengthShort);

	auto lengthMediumOff = CCLabelBMFont::create("Medium", "bigFont.fnt");
	lengthMediumOff->setColor({ 128, 128, 128 });
	auto lengthMediumOn = CCLabelBMFont::create("Medium", "bigFont.fnt");

	auto lengthMedium = CCMenuItemToggler::create(
		lengthMediumOff,
		lengthMediumOn,
		this,
		menu_selector(LevelGrindLayer::onMedium)
	);
	lengthMedium->setID("length-medium-switcher");
	lengthsMenu->addChild(lengthMedium);

	auto lengthLongOff = CCLabelBMFont::create("Long", "bigFont.fnt");
	lengthLongOff->setColor({ 128, 128, 128 });
	auto lengthLongOn = CCLabelBMFont::create("Long", "bigFont.fnt");

	auto lengthLong = CCMenuItemToggler::create(
		lengthLongOff,
		lengthLongOn,
		this,
		menu_selector(LevelGrindLayer::onLong)
	);
	lengthLong->setID("length-long-switcher");
	lengthsMenu->addChild(lengthLong);

	auto lengthXLOff = CCLabelBMFont::create("XL", "bigFont.fnt");
	lengthXLOff->setColor({ 128, 128, 128 });
	auto lengthXLOn = CCLabelBMFont::create("XL", "bigFont.fnt");

	auto lengthXL = CCMenuItemToggler::create(
		lengthXLOff,
		lengthXLOn,
		this,
		menu_selector(LevelGrindLayer::onXL)
	);
	lengthXL->setID("length-xl-switcher");
	lengthsMenu->addChild(lengthXL);

	this->addChild(lengthsMenu);
	lengthsMenu->setPosition({ winSize.width / 2, winSize.height / 10.f });
	lengthsMenu->updateLayout();
	lengthsMenu->setScale(0.6f);

	auto ratingsMenu = CCMenu::create();
	ratingsMenu->setLayout(RowLayout::create()->setGap(40.f));
	ratingsMenu->setPosition({ winSize.width / 2, winSize.height / 4.6f });
	ratingsMenu->setScale(0.6f);
	ratingsMenu->setID("ratings-menu");
	this->addChild(ratingsMenu);

	auto easyOff = CCSprite::createWithSpriteFrameName("diffIcon_01_btn_001.png");
	easyOff->setColor({ 128, 128, 128 });
	auto easyOn = CCSprite::createWithSpriteFrameName("diffIcon_01_btn_001.png");

	auto easyFace = CCMenuItemToggler::create(
		easyOff,
		easyOn,
		this,
		menu_selector(LevelGrindLayer::onEasy)
	);
	easyFace->setID("easy-face-switcher");
	ratingsMenu->addChild(easyFace);

	auto normalOff = CCSprite::createWithSpriteFrameName("diffIcon_02_btn_001.png");
	normalOff->setColor({ 128, 128, 128 });
	auto normalOn = CCSprite::createWithSpriteFrameName("diffIcon_02_btn_001.png");

	auto normalFace = CCMenuItemToggler::create(
		normalOff,
		normalOn,
		this,
		menu_selector(LevelGrindLayer::onNormal)
	);
	normalFace->setID("normal-face-switcher");
	ratingsMenu->addChild(normalFace);

	auto hardOff = CCSprite::createWithSpriteFrameName("diffIcon_03_btn_001.png");
	hardOff->setColor({ 128, 128, 128 });
	auto hardOn = CCSprite::createWithSpriteFrameName("diffIcon_03_btn_001.png");

	auto hardFace = CCMenuItemToggler::create(
		hardOff,
		hardOn,
		this,
		menu_selector(LevelGrindLayer::onHard)
	);
	hardFace->setID("hard-face-switcher");
	ratingsMenu->addChild(hardFace);

	auto harderOff = CCSprite::createWithSpriteFrameName("diffIcon_04_btn_001.png");
	harderOff->setColor({ 128, 128, 128 });
	auto harderOn = CCSprite::createWithSpriteFrameName("diffIcon_04_btn_001.png");

	auto harderFace = CCMenuItemToggler::create(
		harderOff,
		harderOn,
		this,
		menu_selector(LevelGrindLayer::onHarder)
	);
	harderFace->setID("harder-face-switcher");
	ratingsMenu->addChild(harderFace);

	auto insaneOff = CCSprite::createWithSpriteFrameName("diffIcon_05_btn_001.png");
	insaneOff->setColor({ 128, 128, 128 });
	auto insaneOn = CCSprite::createWithSpriteFrameName("diffIcon_05_btn_001.png");

	auto insaneFace = CCMenuItemToggler::create(
		insaneOff,
		insaneOn,
		this,
		menu_selector(LevelGrindLayer::onInsane)
	);
	insaneFace->setID("insane-face-switcher");
	ratingsMenu->addChild(insaneFace);

	ratingsMenu->updateLayout();

	auto chooseLabel = CCLabelBMFont::create(
		"Choose", "bigFont.fnt"
	);
	chooseLabel->setID("choose-label");
	this->addChild(chooseLabel);
	chooseLabel->setPosition({ winSize.width / 2, (winSize.height / 4) * 2.55f });

	auto searchBtn = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Search"),
		this,
		menu_selector(LevelGrindLayer::onSearchBtn)
	);
	searchBtn->setID("search-btn");

	auto searchBtnMenu = CCMenu::create();
	searchBtnMenu->setLayout(RowLayout::create());
	searchBtnMenu->setPosition({ winSize.width / 2, (winSize.height / 4) * 1.5f });

	this->addChild(searchBtnMenu);
	searchBtnMenu->addChild(searchBtn);

	searchBtnMenu->updateLayout();
	searchBtnMenu->setID("search-btn-menu");

	auto typeBtnMenu = CCMenu::create();
	typeBtnMenu->setLayout(RowLayout::create()->setGap(15.f));
	this->addChild(typeBtnMenu);
	typeBtnMenu->setPosition({ winSize.width / 2, (winSize.height / 4) * 2.05f });
	typeBtnMenu->setScale(0.8f);

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
		menu_selector(LevelGrindLayer::onStarSwitcher)
	);
	starSwitcher->setID("star-switcher");
	typeBtnMenu->addChild(starSwitcher);

	auto moonSwitcher = CCMenuItemToggler::create(
		moonBtnSprOff,
		moonBtnSprOn,
		this,
		menu_selector(LevelGrindLayer::onMoonSwitcher)
	);
	moonSwitcher->setID("moon-switcher");
	typeBtnMenu->addChild(moonSwitcher);

	auto coinSwitcher = CCMenuItemToggler::create(
		coinBtnSprOff,
		coinBtnSprOn,
		this,
		menu_selector(LevelGrindLayer::onCoinSwitcher)
	);
	coinSwitcher->setID("coin-switcher");
	typeBtnMenu->addChild(coinSwitcher);

	auto demonSwitcher = CCMenuItemToggler::create(
		demonBtnSprOff,
		demonBtnSprOn,
		this,
		menu_selector(LevelGrindLayer::onDemonSwitcher)
	);
	demonSwitcher->setID("demon-switcher");
	typeBtnMenu->addChild(demonSwitcher);

	typeBtnMenu->updateLayout();

	auto optionsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
	optionsSpr->setScale(0.8f);

	auto settingsBtn = CCMenuItemSpriteExtra::create(
		optionsSpr,
		this,
		menu_selector(LevelGrindLayer::onSettingsBtn)
	);
	settingsBtn->setID("settings-btn");
	searchBtnMenu->addChild(settingsBtn);
	searchBtnMenu->updateLayout();

	auto versionsPanel = CCScale9Sprite::create("square02_small.png");
	versionsPanel->setContentSize({ 30.f, winSize.height / 2.3f });
	versionsPanel->setID("versions-panel");
	this->addChild(versionsPanel);
	versionsPanel->setPosition({ 157.f, winSize.height / 2 });

	auto versionsMenu = CCMenu::create();
	versionsMenu->setLayout(ColumnLayout::create()->setGap(25.f)->setAxisReverse(true));
	versionsMenu->setID("versions-menu");
	versionsMenu->setScale(0.4f);
	versionsMenu->setPosition({ 157.f, winSize.height / 2 });
	this->addChild(versionsMenu);

	auto ver22Off = CCLabelBMFont::create("2.2", "bigFont.fnt");
	ver22Off->setColor({ 128, 128, 128 });
	auto ver22On = CCLabelBMFont::create("2.2", "bigFont.fnt");

	auto ver22 = CCMenuItemToggler::create(
		ver22Off,
		ver22On,
		this,
		menu_selector(LevelGrindLayer::on22)
	);
	ver22->setID("ver-22-btn");
	versionsMenu->addChild(ver22);

	auto ver21Off = CCLabelBMFont::create("2.1", "bigFont.fnt");
	ver21Off->setColor({ 128, 128, 128 });
	auto ver21On = CCLabelBMFont::create("2.1", "bigFont.fnt");

	auto ver21 = CCMenuItemToggler::create(
		ver21Off,
		ver21On,
		this,
		menu_selector(LevelGrindLayer::on21)
	);
	ver21->setID("ver-21-btn");
	versionsMenu->addChild(ver21);

	auto ver20Off = CCLabelBMFont::create("2.0", "bigFont.fnt");
	ver20Off->setColor({ 128, 128, 128 });
	auto ver20On = CCLabelBMFont::create("2.0", "bigFont.fnt");

	auto ver20 = CCMenuItemToggler::create(
		ver20Off,
		ver20On,
		this,
		menu_selector(LevelGrindLayer::on20)
	);
	ver20->setID("ver-20-btn");
	versionsMenu->addChild(ver20);

	auto ver19Off = CCLabelBMFont::create("1.9", "bigFont.fnt");
	ver19Off->setColor({ 128, 128, 128 });
	auto ver19On = CCLabelBMFont::create("1.9", "bigFont.fnt");

	auto ver19 = CCMenuItemToggler::create(
		ver19Off,
		ver19On,
		this,
		menu_selector(LevelGrindLayer::on19)
	);
	ver19->setID("ver-19-btn");
	versionsMenu->addChild(ver19);

	auto verLowerThan19Off = CCLabelBMFont::create("<1.9", "bigFont.fnt");
	verLowerThan19Off->setColor({ 128, 128, 128 });
	auto verLowerThan19On = CCLabelBMFont::create("<1.9", "bigFont.fnt");

	auto verLowerThan19 = CCMenuItemToggler::create(
		verLowerThan19Off,
		verLowerThan19On,
		this,
		menu_selector(LevelGrindLayer::onLowerThan19)
	);
	verLowerThan19->setID("ver-lower-than-19-btn");
	versionsMenu->addChild(verLowerThan19);

	versionsMenu->updateLayout();
	versionsPanel->setOpacity(100);

	auto demonsPanel = CCScale9Sprite::create("square02_small.png");
	demonsPanel->setContentSize({ 30.f, winSize.height / 2.3f });
	demonsPanel->setPosition({ 412.f, winSize.height / 2 });
	demonsPanel->setID("demons-panel");
	this->addChild(demonsPanel);

	auto demonsMenu = CCMenu::create();
	demonsMenu->setLayout(ColumnLayout::create()->setGap(25.f)->setAxisReverse(true));
	demonsMenu->setScale(0.4f);
	demonsMenu->setPosition({ 412.f, winSize.height / 2 });
	demonsMenu->setID("demons-menu");
	this->addChild(demonsMenu);

	auto easyDOff = CCSprite::createWithSpriteFrameName("diffIcon_07_btn_001.png");
	easyDOff->setColor({ 128, 128, 128 });
	auto easyDOn = CCSprite::createWithSpriteFrameName("diffIcon_07_btn_001.png");

	auto easyD = CCMenuItemToggler::create(
		easyDOff,
		easyDOn,
		this,
		menu_selector(LevelGrindLayer::onEasyD)
	);
	easyD->setID("easy-demon-face");
	demonsMenu->addChild(easyD);

	auto mediumDOff = CCSprite::createWithSpriteFrameName("diffIcon_08_btn_001.png");
	mediumDOff->setColor({ 128, 128, 128 });
	auto mediumDOn = CCSprite::createWithSpriteFrameName("diffIcon_08_btn_001.png");

	auto mediumD = CCMenuItemToggler::create(
		mediumDOff,
		mediumDOn,
		this,
		menu_selector(LevelGrindLayer::onMediumD)
	);
	mediumD->setID("medium-demon-face");
	demonsMenu->addChild(mediumD);

	auto hardDOff = CCSprite::createWithSpriteFrameName("diffIcon_06_btn_001.png");
	hardDOff->setColor({ 128, 128, 128 });
	auto hardDOn = CCSprite::createWithSpriteFrameName("diffIcon_06_btn_001.png");

	auto hardD = CCMenuItemToggler::create(
		hardDOff,
		hardDOn,
		this,
		menu_selector(LevelGrindLayer::onHardD)
	);
	hardD->setID("hard-demon-face");
	demonsMenu->addChild(hardD);

	auto insaneDOff = CCSprite::createWithSpriteFrameName("diffIcon_09_btn_001.png");
	insaneDOff->setColor({ 128, 128, 128 });
	auto insaneDOn = CCSprite::createWithSpriteFrameName("diffIcon_09_btn_001.png");

	auto insaneD = CCMenuItemToggler::create(
		insaneDOff,
		insaneDOn,
		this,
		menu_selector(LevelGrindLayer::onInsaneD)
	);
	insaneD->setID("insane-demon-face");
	demonsMenu->addChild(insaneD);

	auto extremeDOff = CCSprite::createWithSpriteFrameName("diffIcon_10_btn_001.png");
	extremeDOff->setColor({ 128, 128, 128 });
	auto extremeDOn = CCSprite::createWithSpriteFrameName("diffIcon_10_btn_001.png");

	auto extremeD = CCMenuItemToggler::create(
		extremeDOff,
		extremeDOn,
		this,
		menu_selector(LevelGrindLayer::onExtremeD)
	);
	extremeD->setID("extreme-demon-face");
	demonsMenu->addChild(extremeD);

	demonsMenu->updateLayout();
	demonsPanel->setOpacity(100);

	auto reqBtn = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Req", "goldFont.fnt", "GJ_button_04.png"),
		this,
		menu_selector(LevelGrindLayer::onReq)
	);

	auto searchBtnFound = getChildByIDRecursive("search-btn");
	searchBtnMenu->insertBefore(reqBtn, searchBtnFound);
	searchBtnMenu->updateLayout();
	searchBtnMenu->setScale(0.8f);

    return true;
}

void LevelGrindLayer::onReq(CCObject* sender) {
	matjson::Value body;
	body["account_id"] = GJAccountManager::get()->m_accountID;
	body["token"] = Mod::get()->getSavedValue<std::string>("argon_token");

	web::WebRequest req;
	req.bodyJSON(body);

	auto upopup = UploadActionPopup::create(nullptr, "Loading...");
	upopup->show();

	m_listener.spawn(
		req.post("https://delivel.tech/grindapi/check_helper"),
		[upopup](web::WebResponse res) {
			if (!res.ok()) {
				log::error("req failed");
				upopup->showFailMessage("Request failed! Try again later.");
				Mod::get()->setSavedValue("isHelper", false);
				return;
			}
			auto json = res.json().unwrap();
			auto isHelper = json["ok"].asBool().unwrap();
			if (isHelper) {
				upopup->showSuccessMessage("Success! Helper granted.");
				Mod::get()->setSavedValue("isHelper", true);
			} else {
				upopup->showFailMessage("Failed! User is not a helper.");
				Mod::get()->setSavedValue("isHelper", false);
			}
		}
	);
}

void LevelGrindLayer::update(float dt) {
	float dx = m_speed * dt;

	m_bg1->setPositionX(m_bg1->getPositionX() - dx);
	m_bg2->setPositionX(m_bg2->getPositionX() - dx);
	if (m_bg1->getPositionX() <= -m_bgWidth) {
		m_bg1->setPositionX(m_bg2->getPositionX() + m_bgWidth - 1.f);
	}
	if (m_bg2->getPositionX() <= -m_bgWidth) {
		m_bg2->setPositionX(m_bg1->getPositionX() + m_bgWidth - 1.f);
	}
}

void LevelGrindLayer::onBack(CCObject*) {
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void LevelGrindLayer::keyBackClicked() {
	this->onBack(nullptr);
}

void LevelGrindLayer::onShort(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();

	auto it = std::find(lengths.begin(), lengths.end(), 1);

	if (isToggled) {
		if (it == lengths.end()) {
			lengths.push_back(1);
		}
	} else {
		if (it != lengths.end()) {
			lengths.erase(it);
		}
	}
}

void LevelGrindLayer::onMedium(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();

	auto it = std::find(lengths.begin(), lengths.end(), 2);

	if (isToggled) {
		if (it == lengths.end()) {
			lengths.push_back(2);
		}
	} else {
		if (it != lengths.end()) {
			lengths.erase(it);
		}
	}
}

void LevelGrindLayer::onLong(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();

	auto it = std::find(lengths.begin(), lengths.end(), 3);

	if (isToggled) {
		if (it == lengths.end()) {
			lengths.push_back(3);
		}
	} else {
		if (it != lengths.end()) {
			lengths.erase(it);
		}
	}
}

void LevelGrindLayer::onXL(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();

	auto it = std::find(lengths.begin(), lengths.end(), 4);

	if (isToggled) {
		if (it == lengths.end()) {
			lengths.push_back(4);
		}
	} else {
		if (it != lengths.end()) {
			lengths.erase(it);
		}
	}
}

void LevelGrindLayer::onEasy(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();

	if (isToggled) {
		difficulties.push_back(2);
	} else {
		difficulties.erase(std::find(difficulties.begin(), difficulties.end(), 2));
	}
}

void LevelGrindLayer::onNormal(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		difficulties.push_back(3);
	} else {
		difficulties.erase(std::find(difficulties.begin(), difficulties.end(), 3));
	}
}

void LevelGrindLayer::onHard(CCObject* sender) {
  auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
  bool isToggled = !toggler->isToggled();
  if (isToggled) {
    if (Mod::get()->getSettingValue<bool>("hard-separate")) {
      int choice = Mod::get()->getSettingValue<int>("hard-choice");
      difficulties.push_back(choice);
    } else {
      difficulties.push_back(4);
      difficulties.push_back(5);
    }
  } else {
    if (Mod::get()->getSettingValue<bool>("hard-separate")) {
      int choice = Mod::get()->getSettingValue<int>("hard-choice");
      auto it = std::find(difficulties.begin(), difficulties.end(), choice);
      if (it != difficulties.end()) {
        difficulties.erase(it);
      }
    } else {
      auto it4 = std::find(difficulties.begin(), difficulties.end(), 4);
      if (it4 != difficulties.end()) {
        difficulties.erase(it4);
      }
      auto it5 = std::find(difficulties.begin(), difficulties.end(), 5);
      if (it5 != difficulties.end()) {
        difficulties.erase(it5);
      }
    }
  }
}

void LevelGrindLayer::onHarder(CCObject* sender) {
  auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
  bool isToggled = !toggler->isToggled();
  if (isToggled) {
    if (Mod::get()->getSettingValue<bool>("harder-separate")) {
      int choice = Mod::get()->getSettingValue<int>("harder-choice");
      difficulties.push_back(choice);
    } else {
      difficulties.push_back(6);
      difficulties.push_back(7);
    }
  } else {
    if (Mod::get()->getSettingValue<bool>("harder-separate")) {
      int choice = Mod::get()->getSettingValue<int>("harder-choice");
      auto it = std::find(difficulties.begin(), difficulties.end(), choice);
      if (it != difficulties.end()) {
        difficulties.erase(it);
      }
    } else {
      auto it6 = std::find(difficulties.begin(), difficulties.end(), 6);
      if (it6 != difficulties.end()) {
        difficulties.erase(it6);
      }
      auto it7 = std::find(difficulties.begin(), difficulties.end(), 7);
      if (it7 != difficulties.end()) {
        difficulties.erase(it7);
      }
    }
  }
}

void LevelGrindLayer::onInsane(CCObject* sender) {
  auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
  bool isToggled = !toggler->isToggled();
  if (isToggled) {
    if (Mod::get()->getSettingValue<bool>("insane-separate")) {
      int choice = Mod::get()->getSettingValue<int>("insane-choice");
      difficulties.push_back(choice);
    } else {
      difficulties.push_back(8);
      difficulties.push_back(9);
    }
  } else {
    if (Mod::get()->getSettingValue<bool>("insane-separate")) {
      int choice = Mod::get()->getSettingValue<int>("insane-choice");
      auto it = std::find(difficulties.begin(), difficulties.end(), choice);
      if (it != difficulties.end()) {
        difficulties.erase(it);
      }
    } else {
      auto it8 = std::find(difficulties.begin(), difficulties.end(), 8);
      if (it8 != difficulties.end()) {
        difficulties.erase(it8);
      }
      auto it9 = std::find(difficulties.begin(), difficulties.end(), 9);
      if (it9 != difficulties.end()) {
        difficulties.erase(it9);
      }
    }
  }
}


void LevelGrindLayer::onSearchBtn(CCObject* sender) {
	matjson::Value body;
	if (!difficulties.empty()) body["difficulties"] = difficulties;
	if (!lengths.empty()) body["lengths"] = lengths;
	if (!demonDifficulties.empty()) body["demonDifficulties"] = demonDifficulties;
	if (!grindTypes.empty()) body["grindTypes"] = grindTypes;
	if (!versions.empty()) body["versions"] = versions;
	auto loading = LoadingCircle::create();
	loading->show();

	web::WebRequest req;
	req.bodyJSON(body);
	
	m_listener.spawn(
		req.post("https://delivel.tech/grindapi/get_levels"),
		[loading](web::WebResponse res) {
			if (!res.ok()) {
				log::error("bad web request");
				Notification::create("Failed to fetch levels", NotificationIcon::Error)->show();
				loading->fadeAndRemove();
				return;
			}
			std::vector<int> ids;
			auto response = res.json().unwrap();

			if (response["count"].asInt().unwrap() == 0) {
				Notification::create("No levels found", NotificationIcon::Info)->show();
				loading->fadeAndRemove();
				return;
			}

			auto arrRes = response["ids"].asArray().unwrap();
			std::vector<int> uncompletedIDs;

			if (Mod::get()->getSettingValue<bool>("only-uncompleted")) {
				for (auto id : arrRes) {
				    auto isCompleted = GameStatsManager::sharedState()->hasCompletedOnlineLevel(id.asInt().unwrapOrDefault());
				    if (!isCompleted) {
					    auto uncID = id.asInt().unwrapOrDefault();
					    uncompletedIDs.push_back(uncID);
				    }
			    }

				if (uncompletedIDs.empty()) {
					Notification::create("No uncompleted levels found", NotificationIcon::Info)->show();
					loading->fadeAndRemove();
					return;
				}

				std::string uncLevelIDs = numToString(uncompletedIDs[0]);

				for (size_t i = 1; i < uncompletedIDs.size(); ++i) {
				    uncLevelIDs += "," + numToString(uncompletedIDs[i]);
                }

				auto search = GJSearchObject::create(SearchType::Type19, uncLevelIDs);

				auto browser = GrindBrowserLayer::create("Grinding Levels", search);

				auto scene = CCScene::create();
			    scene->addChild(browser);

			    auto transition = CCTransitionFade::create(0.5f, scene);
			    loading->fadeAndRemove();
			    CCDirector::sharedDirector()->pushScene(transition);
				return;
			}

			for (auto lvl : arrRes) {
				int id = lvl.asInt().unwrapOrDefault();
				ids.push_back(id);
			}

			if (ids.empty()) {
				Notification::create("No levels found", NotificationIcon::Info)->show();
				loading->fadeAndRemove();
				return;
			}

			std::string levelIDs = numToString(ids[0]);

            for (size_t i = 1; i < ids.size(); ++i) {
				levelIDs += "," + numToString(ids[i]);
            }

			auto search = GJSearchObject::create(SearchType::Type19, levelIDs);

			auto browser = GrindBrowserLayer::create("Grinding Levels", search);

			auto scene = CCScene::create();
			scene->addChild(browser);

			auto transition = CCTransitionFade::create(0.5f, scene);
			loading->fadeAndRemove();
			CCDirector::sharedDirector()->pushScene(transition);
		}
	);
}

void LevelGrindLayer::onStarSwitcher(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		grindTypes.push_back("star");
	} else {
		grindTypes.erase(std::find(grindTypes.begin(), grindTypes.end(), "star"));
	}
}

void LevelGrindLayer::onMoonSwitcher(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		grindTypes.push_back("moon");
	} else {
		grindTypes.erase(std::find(grindTypes.begin(), grindTypes.end(), "moon"));
	}
}

void LevelGrindLayer::onCoinSwitcher(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		grindTypes.push_back("coin");
	} else {
		grindTypes.erase(std::find(grindTypes.begin(), grindTypes.end(), "coin"));
	}
}

void LevelGrindLayer::onDemonSwitcher(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		grindTypes.push_back("demon");
	} else {
		grindTypes.erase(std::find(grindTypes.begin(), grindTypes.end(), "demon"));
	}
}

void LevelGrindLayer::onSettingsBtn(CCObject* sender) {
	openSettingsPopup(getMod());
}

void LevelGrindLayer::on22(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		versions.push_back(22);
	} else {
		versions.erase(std::find(versions.begin(), versions.end(), 22));
	}
}

void LevelGrindLayer::on21(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		versions.push_back(21);
	} else {
		versions.erase(std::find(versions.begin(), versions.end(), 21));
	}
}

void LevelGrindLayer::on20(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		versions.push_back(20);
	} else {
		versions.erase(std::find(versions.begin(), versions.end(), 20));
	}
}

void LevelGrindLayer::on19(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		versions.push_back(19);
	} else {
		versions.erase(std::find(versions.begin(), versions.end(), 19));
	}
}

void LevelGrindLayer::onLowerThan19(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		versions.push_back(18);
	} else {
		versions.erase(std::find(versions.begin(), versions.end(), 18));
	}
}

void LevelGrindLayer::onEasyD(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demonDifficulties.push_back(3);
	} else {
		demonDifficulties.erase(std::find(demonDifficulties.begin(), demonDifficulties.end(), 3));
	}

}

void LevelGrindLayer::onMediumD(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demonDifficulties.push_back(4);
	} else {
		demonDifficulties.erase(std::find(demonDifficulties.begin(), demonDifficulties.end(), 4));
	}
}

void LevelGrindLayer::onHardD(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demonDifficulties.push_back(0);
	} else {
		demonDifficulties.erase(std::find(demonDifficulties.begin(), demonDifficulties.end(), 0));
	}
}

void LevelGrindLayer::onInsaneD(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demonDifficulties.push_back(5);
	} else {
		demonDifficulties.erase(std::find(demonDifficulties.begin(), demonDifficulties.end(), 5));
	}
}

void LevelGrindLayer::onExtremeD(CCObject* sender) {
	auto toggler = typeinfo_cast<CCMenuItemToggler*>(sender);
	bool isToggled = !toggler->isToggled();
	if (isToggled) {
		demonDifficulties.push_back(6);
	} else {
		demonDifficulties.erase(std::find(demonDifficulties.begin(), demonDifficulties.end(), 6));
	}
}
