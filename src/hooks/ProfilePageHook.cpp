#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include "../other/LGManager.hpp"
#include "../popups/UserManagePopup.hpp"
#include "Geode/cocos/cocoa/CCObject.h"

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

		if (!Mod::get()->getSavedValue<bool>("disable-badges")) {
			m_fields->m_staffFound = false;
		    m_fields->m_staffRole = 0;

		    auto it = LGManager::get()->getStaff().find(score->m_accountID);
		    if (it != LGManager::get()->getStaff().end()) {
			    m_fields->m_staffFound = true;
			    m_fields->m_staffRole = it->second;

				auto usernameMenu = getChildByIDRecursive("username-menu");
				bool hasModBadge = getChildByIDRecursive("mod-badge") != nullptr;
				bool noBadgeForMods = Mod::get()->getSavedValue<bool>("no-badge-for-mods");
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
			    } else if (m_fields->m_staffRole == 3) {
                    auto badgeSpr = CCSprite::create("badge_artist.png"_spr);
                    auto badgeBtn = CCMenuItemSpriteExtra::create(
                        badgeSpr,
						this,
                        menu_selector(UserManage::onContributorBadge)
                    );
                    badgeBtn->setID("grind-contributor-badge");

                    if (usernameMenu && !shouldSkipStaffBadge) {
                        usernameMenu->addChild(badgeBtn);
                        usernameMenu->updateLayout();
                    }
                }
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
};