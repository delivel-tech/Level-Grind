#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCTextInputNode.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <argon/argon.hpp>
#include <fmt/format.h>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"
#include "other/ReqButtonSetting.hpp"
#include "popups/UserManagePopup.hpp"

using namespace geode::prelude;

std::unordered_map<int, int> staffs; // accountId, role (1 owner, 2 admin)

$on_mod(Loaded) {
	registerReqButtonSettingType();

	async::spawn(
        argon::startAuth(),
        [](Result<std::string> res) {
            if (!res.ok()) return;
			auto token = std::move(res).unwrap();
			Mod::get()->setSavedValue("argon_token", token);
        }
    );

	web::WebRequest req;

	async::spawn(
		req.get("https://delivel.tech/grindapi/get_staff"),
		[](web::WebResponse res) {
			if (!res.ok()) {
				log::warn("get_staff returned non-ok status: {}", res.code());
				return;
			}

			auto jsonRes = res.json();
			if (!jsonRes) {
				log::warn("Failed to parse get_staff JSON");
				return;
			}

			auto json = jsonRes.unwrap();

			bool isOK = json["ok"].asBool().unwrapOrDefault();
			if (!isOK) {
				log::warn("Server returned ok=false for get_staff");
				return;
			}

			auto staff = json["staff"].asArray();

			if (!staff) {
				log::warn("get_staff JSON does not contain staff array");
				return;
			}

			for (auto const& val : staff.unwrap()) {
				auto accountID = val["accountID"].asInt();
				auto role = val["role"].asInt();

				if (!accountID || !role) {
					continue;
				}

				staffs[accountID.unwrap()] = role.unwrap();
			}
		}
	);
}

class $modify(UserManage, ProfilePage) {
	struct Fields {
		int m_targetAccountID;
		const char* m_username;
		int m_targetIcon;
		int m_targetColor1;
		int m_targetColor2;
		int m_targetColor3;
		bool m_staffFound = false;
		int m_staffRole = 0;
	};
	void loadPageFromUserInfo(GJUserScore* score) {
		ProfilePage::loadPageFromUserInfo(score);

		if (auto mngBtn = getChildByIDRecursive("lg-manage-btn")) mngBtn->removeFromParent();
		if (auto badgeBtn = getChildByIDRecursive("grind-helper-badge")) badgeBtn->removeFromParent();
		if (auto badgeBtn = getChildByIDRecursive("grind-admin-badge")) badgeBtn->removeFromParent();

		m_fields->m_targetAccountID = score->m_accountID;
		m_fields->m_username = score->m_userName.c_str();
		m_fields->m_targetIcon = score->m_playerCube;
		m_fields->m_targetColor1 = score->m_color1;
		m_fields->m_targetColor2 = score->m_color2;
		m_fields->m_targetColor3 = score->m_color3;

		auto leftMenu = getChildByIDRecursive("left-menu");
		if (!leftMenu) return;

		auto spr = CCSprite::create("button_add_2.png"_spr);
		spr->setScale(0.55f);

		auto manageBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(UserManage::onManageBtn)
		);
		manageBtn->setID("lg-manage-btn");

		if (!Mod::get()->getSettingValue<bool>("disable-badges")) {
			m_fields->m_staffFound = false;
		    m_fields->m_staffRole = 0;

		    auto it = staffs.find(score->m_accountID);
		    if (it != staffs.end()) {
			    m_fields->m_staffFound = true;
			    m_fields->m_staffRole = it->second;

				auto usernameMenu = getChildByIDRecursive("username-menu");
				bool hasModBadge = getChildByIDRecursive("mod-badge") != nullptr;
				bool noBadgeForMods = Mod::get()->getSettingValue<bool>("no-badge-for-mods");
				bool shouldSkipStaffBadge = noBadgeForMods && hasModBadge;

			    if (m_fields->m_staffRole == 1) {
				    auto badgeSpr = CCSprite::create("badge_helper.png"_spr);

				    auto badgeBtn = CCMenuItemSpriteExtra::create(
					    badgeSpr,
					    this,
					    menu_selector(UserManage::onHelperBadge)
				    );

				    badgeBtn->setID("grind-helper-badge");

				    if (usernameMenu && !shouldSkipStaffBadge) {
					    usernameMenu->addChild(badgeBtn);
					    usernameMenu->updateLayout();
				    }
			    } else if (m_fields->m_staffRole == 2) {
				    auto badgeSpr = CCSprite::create("badge_admin.png"_spr);
				    auto badgeBtn = CCMenuItemSpriteExtra::create(
					    badgeSpr,
					    this,
					    menu_selector(UserManage::onAdminBadge)
				    );

				    badgeBtn->setID("grind-admin-badge");

				    if (usernameMenu && !shouldSkipStaffBadge) {
					    usernameMenu->addChild(badgeBtn);
					    usernameMenu->updateLayout();
				    }
			    }
		    }
		}

		if (!Mod::get()->getSavedValue<bool>("isAdmin")) return;

		leftMenu->addChild(manageBtn);
		leftMenu->updateLayout();
	}

	void onManageBtn(CCObject* sender) {
		UserManagePopup::create(
			m_fields->m_targetAccountID, 
			m_fields->m_username, 
			m_fields->m_targetIcon, 
			m_fields->m_targetColor1, 
			m_fields->m_targetColor2, 
			m_fields->m_targetColor3
		)->show();
	}

	void onHelperBadge(CCObject* sender) {
		FLAlertLayer::create(
			"Grind Helper",
			"This user is a <cg>Helper</c> on the <cp>Level Grind</c> mod. " \
			"They <cj>help</c> add <cr>delete</c> levels on the <cp>Level Grind</c> database.",
			"OK"
		)->show();
	}

	void onAdminBadge(CCObject* sender) {
		FLAlertLayer::create(
			"Grind Admin",
			"This user is an <cg>Admin</c> on the <cp>Level Grind</c> mod. " \
			"They can do everything a <cj>Helper</c> can do, but they can also manage the staff team on the <cp>Level Grind</c> mod.",
			"OK"
		)->show();
	}
};
