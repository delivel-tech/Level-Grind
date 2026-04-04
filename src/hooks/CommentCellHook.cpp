#include <Geode/Geode.hpp>
#include <Geode/modify/CommentCell.hpp>

#include "../other/LGManager.hpp"

using namespace geode::prelude;

class $modify(GrindCommentCell, CommentCell) {
    struct Fields {
        bool isOwner = false;
    };
    void loadFromComment(GJComment* p0) {
        CommentCell::loadFromComment(p0);

        m_fields->isOwner = p0->m_accountID == 13678537;

        CCMenu* username_menu = static_cast<CCMenu*>(m_mainLayer->getChildByIDRecursive("username-menu"));
        if (!username_menu) return;

        if (auto b = username_menu->getChildByID("grind-owner-badge"_spr)) b->removeFromParent();
        if (auto b = username_menu->getChildByID("grind-admin-badge"_spr)) b->removeFromParent();
        if (auto b = username_menu->getChildByID("grind-helper-badge"_spr)) b->removeFromParent();
        if (auto b = username_menu->getChildByID("grind-artist-badge"_spr)) b->removeFromParent();
        if (auto b = username_menu->getChildByID("grind-contributor-badge"_spr)) b->removeFromParent();
        if (auto b = username_menu->getChildByID("grind-booster-badge"_spr)) b->removeFromParent();

        if (Mod::get()->getSavedValue<bool>("disable-badges")) return;
        if (Mod::get()->getSavedValue<bool>("no-badge-for-mods") && username_menu->getChildByID("mod-badge") != nullptr) return;

        int accountID = p0->m_accountID;
        auto& staff = LGManager::get()->getStaff();

        auto tryAddBadge = [&](std::vector<int>& list, const char* sprName, const char* id, SEL_MenuHandler selector) {
            if (std::find(list.begin(), list.end(), accountID) == list.end()) return;
            auto spr = CCSprite::create(sprName);
            spr->setScale(0.65f);
            auto btn = CCMenuItemSpriteExtra::create(spr, this, selector);
            btn->setID(id);
            username_menu->addChild(btn);
        };

        tryAddBadge(staff.owners, "badge_owner.png"_spr, "grind-owner-badge"_spr, menu_selector(GrindCommentCell::onOwnerBadge));
        tryAddBadge(staff.admins, "badge_admin.png"_spr, "grind-admin-badge"_spr, menu_selector(GrindCommentCell::onAdminBadge));
        tryAddBadge(staff.helpers, "badge_helper.png"_spr, "grind-helper-badge"_spr, menu_selector(GrindCommentCell::onHelperBadge));
        tryAddBadge(staff.artists, "badge_artist.png"_spr, "grind-artist-badge"_spr, menu_selector(GrindCommentCell::onArtistBadge));
        tryAddBadge(staff.contributors, "badge_contributor.png"_spr, "grind-contributor-badge"_spr, menu_selector(GrindCommentCell::onContributorBadge));
        tryAddBadge(staff.boosters, "badge_booster.png"_spr, "grind-booster-badge"_spr, menu_selector(GrindCommentCell::onBoosterBadge));

        username_menu->updateLayout();
    }

    void onOwnerBadge(CCObject*) {
        std::string role = m_fields->isOwner ? "Owner" : "Co-Owner";
		std::string role2 = m_fields->isOwner ? "an Owner" : "a Co-Owner";
		FLAlertLayer::create(
			fmt::format("Grind {}", role).c_str(),
			fmt::format(
				"This user is <cy>{}</c> on the <cp>Level Grind</c> mod. " \
				"They are <cg>responsible for leading the project</c>, <cj>development</c>, <cy>making final decisions</c> etc.",
				role2
			).c_str(),
			"OK"
		)->show();
    }

    void onAdminBadge(CCObject*) {
        FLAlertLayer::create(
            "Grind Admin",
            "This user is an <cg>Admin</c> on the <cp>Level Grind</c> mod. "
            "They can do everything a <cj>Helper</c> can do, but they can also manage the staff team on the <cp>Level Grind</c> mod.",
            "OK"
        )->show();
    }

    void onHelperBadge(CCObject*) {
        FLAlertLayer::create(
            "Grind Helper",
            "This user is a <cg>Helper</c> on the <cp>Level Grind</c> mod. "
            "They <cj>help</c> with adding and <cr>deleting</c> levels on the <cp>Level Grind</c> database.",
            "OK"
        )->show();
    }

    void onArtistBadge(CCObject*) {
        FLAlertLayer::create(
            "Grind Artist",
            "This user is an <cp>Artist</c> on the <cy>Level Grind</c> mod. "
            "They are <cr>responsible for the visual part</c> of the mod.",
            "OK"
        )->show();
    }

    void onContributorBadge(CCObject*) {
        FLAlertLayer::create(
            "Grind Contributor",
            "This user is a <cg>Contributor</c> on the <cp>Level Grind</c> mod. "
            "They help the project through various contributions, such as art, testing and so on.",
            "OK"
        )->show();
    }

    void onBoosterBadge(CCObject*) {
        createQuickPopup(
            "Grind Booster",
            "This user is a <cp>Booster</c> of <cy>Level Grind Discord server</c>. "
            "Their <cg>support is greatly appreciated</c>! <cl>Join the server?</c>",
            "Cancel", "Join",
            [](auto, bool btn2) {
                if (btn2) web::openLinkInBrowser("https://discord.gg/tmf5xtCX5y");
            }
        );
    }
};