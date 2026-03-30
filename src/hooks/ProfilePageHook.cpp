#include <Geode/Geode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include "../other/LGManager.hpp"
#include "../popups/UserManagePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/web.hpp"

using namespace geode::prelude;

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
		if (auto badgeBtn = getChildByIDRecursive("grind-contributor-badge")) badgeBtn->removeFromParent();
		if (auto badgeBtn = getChildByIDRecursive("grind-artist-badge")) badgeBtn->removeFromParent();
		if (auto badgeBtn = getChildByIDRecursive("grind-booster-badge")) badgeBtn->removeFromParent();

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

		auto usernameMenu = getChildByIDRecursive("username-menu");
		bool hasModBadge = getChildByIDRecursive("mod-badge") != nullptr;
		bool noBadgeForMods = Mod::get()->getSavedValue<bool>("no-badge-for-mods");
		bool shouldSkipStaffBadge = noBadgeForMods && hasModBadge;
		bool dontShowBadges = Mod::get()->getSavedValue<bool>("disable-badges");

		if (!dontShowBadges && usernameMenu && !shouldSkipStaffBadge) {
			auto admins = LGManager::get()->getStaff().admins;
			auto helpers = LGManager::get()->getStaff().helpers;
			auto conts = LGManager::get()->getStaff().contributors;
			auto artists = LGManager::get()->getStaff().artists;
			auto boosters = LGManager::get()->getStaff().boosters;

			bool adminFound = std::find(admins.begin(), admins.end(), score->m_accountID) != admins.end();
			bool helperFound = std::find(helpers.begin(), helpers.end(), score->m_accountID) != helpers.end();
			bool contFound = std::find(conts.begin(), conts.end(), score->m_accountID) != conts.end();
			bool artistFound = std::find(artists.begin(), artists.end(), score->m_accountID) != artists.end();
			bool boosterFound = std::find(boosters.begin(), boosters.end(), score->m_accountID) != boosters.end();

			if (adminFound) {
				auto badgeSpr = CCSprite::create("badge_admin.png"_spr);
				auto badgeBtn = CCMenuItemSpriteExtra::create(
					badgeSpr,
					this,
					menu_selector(UserManage::onAdminBadge)
				);

				badgeBtn->setID("grind-admin-badge");
				usernameMenu->addChild(badgeBtn);
				usernameMenu->updateLayout();
			}
			if (helperFound) {
				auto badgeSpr = CCSprite::create("badge_helper.png"_spr);

				auto badgeBtn = CCMenuItemSpriteExtra::create(
					badgeSpr,
					this,
					menu_selector(UserManage::onHelperBadge)
				);

				badgeBtn->setID("grind-helper-badge");
				usernameMenu->addChild(badgeBtn);
				usernameMenu->updateLayout();
			}
			if (artistFound) {
				auto badgeSpr = CCSprite::create("badge_artist.png"_spr);

				auto badgeBtn = CCMenuItemSpriteExtra::create(
					badgeSpr,
					this,
					menu_selector(UserManage::onArtistBadge)
				);

				badgeBtn->setID("grind-artist-badge");
				usernameMenu->addChild(badgeBtn);
				usernameMenu->updateLayout();
			}
			if (contFound) {
				auto badgeSpr = CCSprite::create("badge_contributor.png"_spr);

				auto badgeBtn = CCMenuItemSpriteExtra::create(
					badgeSpr,
					this,
					menu_selector(UserManage::onContributorBadge)
				);

				badgeBtn->setID("grind-contributor-badge");
				usernameMenu->addChild(badgeBtn);
				usernameMenu->updateLayout();
			}
			if (boosterFound) {
				auto badgeSpr = CCSprite::create("badge_booster.png"_spr);

				auto badgeBtn = CCMenuItemSpriteExtra::create(
					badgeSpr,
					this,
					menu_selector(UserManage::onBoosterBadge)
				);

				badgeBtn->setID("grind-booster-badge");
				usernameMenu->addChild(badgeBtn);
				usernameMenu->updateLayout();
			}
		}

		if (!LGManager::get()->isAdmin()) return;

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
			"They <cj>help</c> with adding and <cr>deleting</c> levels on the <cp>Level Grind</c> database.",
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

    void onContributorBadge(CCObject* sender) {
        FLAlertLayer::create(
            "Grind Contributor",
            "This user is a <cg>Contributor</c> on the <cp>Level Grind</c> mod. " \
            "They help the project through various contributions, such as art, testing and so on.",
            "OK"
        )->show();
    }

	void onArtistBadge(CCObject* sender) {
		FLAlertLayer::create(
			"Grind Artist",
			"This user is an <cp>Artist</c> on the <cy>Level Grind</c> mod. " \
			"They are <cr>responsible for the visual part</c> of the mod",
			"OK"
		)->show();
	}

	void onBoosterBadge(CCObject* sender) {
		createQuickPopup(
			"Grind Booster",
			"This user is a <cp>Booster</c> of <cy>Level Grind Discord server</c>. " \
			"Their <cg>support is greatly appreciated</c>! <cl>Join the server?</c>",
			"Cancel", "Join",
			[&](auto, bool btn2) {
				if (btn2) {
					web::openLinkInBrowser("https://discord.gg/tmf5xtCX5y");
				}
			}
		);
	}
};