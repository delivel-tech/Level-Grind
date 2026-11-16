#include "CreditsPopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include <Geode/Enums.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LoadingCircleSprite.hpp>
#include <Geode/binding/ProfilePage.hpp>

#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <cstddef>
#include <cue/ListNode.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>
#include <unordered_map>

#include "../../managers/APIClient.hpp"
#include "../../managers/DataManager.hpp"
#include "../../utils/globals.hpp"
#include "GuidePopup.hpp"

using namespace geode::prelude;

static constexpr CCSize LIST_SIZE { 340.f, 200.f};

namespace levelgrind {

class CreditsPlayerNode : public CCNode {
public:
    struct CreditsUser {
        int cube;
        int color1;
        int color2;
        int glowColor;
        int accountId;
        std::string username;
    };

    static CreditsPlayerNode* create(const CreditsUser& user) {
        auto ret = new CreditsPlayerNode;
        if (ret && ret->init(user)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    int getAccountId() const { return m_accountId; }

    void applyUser(const CreditsUser& user) {
        if (m_icon) {
            m_icon->updateIcons(cue::Icons {
                .type = IconType::Cube,
                .id = user.cube,
                .color1 = user.color1,
                .color2 = user.color2,
                .glowColor = user.glowColor
            });
            if (user.glowColor == 0) m_icon->disableGlowOutline();
        }
        if (m_nameLabel && m_nameItem) {
            m_nameLabel->setString(user.username.c_str());
            m_nameLabel->limitLabelWidth(52.f, 0.45f, 0.05f);
            auto pos = m_nameItem->getPosition();
            m_nameItem->updateSprite();
            m_nameItem->setPosition(pos);
        }
    }

private:
    int m_accountId = 0;
    cue::PlayerIcon* m_icon = nullptr;
    CCLabelBMFont* m_nameLabel = nullptr;
    CCMenuItemSpriteExtra* m_nameItem = nullptr;

    bool init(const CreditsUser& user) {
        CCNode::init();

        m_accountId = user.accountId;

        auto* playerIcon = Build(cue::PlayerIcon::create(
            cue::Icons {
                .type = IconType::Cube,
                .id = user.cube,
                .color1 = user.color1,
                .color2 = user.color2,
                .glowColor = user.glowColor
            }
        ))
                .anchorPoint({ 0.5f, 0.5f })
                .zOrder(0)
                .parent(this)
                .collect();

        if (user.glowColor == 0) playerIcon->disableGlowOutline();
        m_icon = playerIcon;

        auto* playerShadow = Build<CCSprite>::createSpriteName("chest_shadow_001.png")
            .scale(0.4f)
            .pos(0.f, -15.f)
            .opacity(128)
            .zOrder(-1)
            .parent(this)
            .collect();

        CCMenuItemSpriteExtra* nameLabel;
        auto menu = Build<CCLabelBMFont>::create(user.username.c_str(), "goldFont.fnt")
            .scale(0.45f)
            .limitLabelWidth(52.f, 0.45f, 0.05f)
            .store(m_nameLabel)
            .intoMenuItem([accountId = user.accountId, name = user.username] {
                auto pp = ProfilePage::create(accountId, false);
                pp->show();
            })
            .pos(0.f, 24.f)
            .store(nameLabel)
            .intoNewParent(CCMenu::create())
            .pos(0.f, 0.f)
            .parent(this)
            .collect();

        m_nameItem = nameLabel;

        float width = playerIcon->getScaledContentSize().width * 1.1f;
        float height = playerIcon->getScaledContentSize().height * 1.1f + nameLabel->getScaledContentSize().height;

        this->setContentSize({width, height});

        CCPoint delta = CCPoint{width, height} / 2;
        playerIcon->setPosition(playerIcon->getPosition() + delta);
        playerShadow->setPosition(playerShadow->getPosition() + delta);
        menu->setPosition(menu->getPosition() + delta);

        return true;
    }
};

class CategoryCell : public CCNode {
public:
    struct CreditsCategory {
        std::string categoryName;
        std::vector<CreditsPlayerNode::CreditsUser> users;
    };

    static CategoryCell* create(const CreditsCategory& cat) {
        auto ret = new CategoryCell;
        if (ret && ret->init(cat)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

private:
    CCSprite* m_badge = nullptr;
    bool init(const CreditsCategory& cat) {
        CCNode::init();

        constexpr size_t kPlayersInRow = 6;

        size_t rows = (cat.users.size() + kPlayersInRow - 1) / kPlayersInRow;

        auto* titleMenu = Build<CCMenu>::create()
            .layout(RowLayout::create()->setGap(8)->setAutoScale(false))
            .scale(0.68f)
            .pos(LIST_SIZE.width / 2.f, 3.f)
            .parent(this)
            .collect();

        std::unordered_map<std::string, const char*> badgeNames {
            {"Coordinators", "badge_owner.png"_spr},
            {"Developers", "badge_developer.png"_spr},
            {"Admins", "badge_admin.png"_spr},
            {"Helpers", "badge_helper.png"_spr},
            {"Artists", "badge_artist.png"_spr},
            {"Contributors", "badge_contributor.png"_spr},
            {"Boosters", "badge_booster.png"_spr}
        };

        const char* currentCategoryBadgeName = [badgeNames, cat] -> const char* {
            auto it = badgeNames.find(cat.categoryName);
            if (it != badgeNames.end()) return it->second;
            else return nullptr;
        }();

        if (currentCategoryBadgeName) {
            auto badge = Build<CCSprite>::createSpriteName(currentCategoryBadgeName)
                .parent(titleMenu)
                .collect();

            m_badge = badge;
        }

        auto title = Build<CCLabelBMFont>::create(fmt::format("{}", cat.categoryName).c_str(), "bigFont.fnt")
            .parent(titleMenu)
            .collect();

        std::unordered_map<std::string, GuidePage> infoPageToOpen {
            {"Coordinators", GuidePage::OwnerRoleGuide},
            {"Developers", GuidePage::DeveloperRoleGuide},
            {"Admins", GuidePage::AdminRoleGuide},
            {"Helpers", GuidePage::HelperRoleGuide},
            {"Artists", GuidePage::ArtistRoleGuide},
            {"Contributors", GuidePage::ContribRoleGuide},
            {"Boosters", GuidePage::BoosterRoleGuide}
        };

        GuidePage infoPage = [infoPageToOpen, cat] -> GuidePage {
            auto it = infoPageToOpen.find(cat.categoryName);
            if (it != infoPageToOpen.end()) return it->second;
            else return GuidePage::MainPage;
        }();

        auto infoBtn = Build<CCSprite>::create("info_btn.png"_spr)
            .scale(0.35f)
            .intoMenuItem([infoPage] {
                GuidePopup::create(infoPage, GuidePopupState::FromOutside)->show();
            })
            .parent(titleMenu)
            .collect();

        titleMenu->updateLayout();

        const float wrapperGap = 8.f;
        auto* playerWrapper = Build<CCNode>::create()
            .layout(ColumnLayout::create()->setAxisReverse(true)->setGap(wrapperGap))
            .pos(LIST_SIZE.width / 2.f, 7.f)
            .anchorPoint(0.5f, 0.0f)
            .contentSize(LIST_SIZE.width, 50.f)
            .id("player-wrapper"_spr)
            .parent(this)
            .collect();

        float wrapperHeight = 0.f;

        for (size_t i = 0; i < rows; i++) {
            size_t firstIdx = i * kPlayersInRow;
            size_t lastIdx = std::min((i + 1) * kPlayersInRow, cat.users.size());
            size_t count = lastIdx - firstIdx;

            float playerGap = 15.f;
            if (count < 4) {
                playerGap = 40.f;
            } else if (count == 4) {
                playerGap = 30.f;
            } else if (count == 5) {
                playerGap = 25.f;
            } else if (count == 6) {
                playerGap = 18.f;
            }

            auto* row = Build<CCNode>::create()
                .layout(RowLayout::create()->setGap(playerGap))
                .id("wrapper-row"_spr)
                .parent(playerWrapper)
                .contentSize(LIST_SIZE.width, 0.f)
                .collect();

            for (size_t i = firstIdx; i < lastIdx; i++) {
                row->addChild(CreditsPlayerNode::create(cat.users[i]));
            }

            row->updateLayout();

            wrapperHeight += row->getContentHeight();
            if (i != 0) {
                wrapperHeight += wrapperGap;
            }
        }

        playerWrapper->setContentSize({0.f, wrapperHeight});
        playerWrapper->updateLayout();

        this->setContentSize(CCSize{LIST_SIZE.width, playerWrapper->getScaledContentSize().height + 8.f + title->getScaledContentSize().height});

        auto membersAmountLabel = Build(CCLabelBMFont::create(numToString(cat.users.size()).c_str(), "chatFont.fnt"))
            .scale(0.65f)
            .color({ 0, 0, 0  })
            .opacity(100)
            .anchorPoint({ 1.f, 0.5f })
            .pos({ LIST_SIZE.width - 10, this->getContentHeight() - 10})
            .parent(this)
            .collect();

        titleMenu->setPosition({titleMenu->getPositionX(), this->getContentHeight() - 10.f});

        if (m_badge) m_badge->setPositionY(m_badge->getPositionY() - 2);
        if (infoBtn) infoBtn->setPositionY(infoBtn->getPositionY() - 2);

        return true;
    }
};

CreditsPopup* CreditsPopup::create() {
    auto ret = new CreditsPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CreditsPopup::init() {
    if (!Popup::init(380.f, 260.f)) return false;

    this->setTitle("Level Grind Credits");

    m_list = Build(cue::ListNode::create(LIST_SIZE))
        .pos(m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f - 10.f)
        .parent(m_mainLayer)
        .collect();

    m_list->setAutoUpdate(false);

    auto* joinBtn = Build<CCSprite>::createSpriteName("GJ_longBtn05_001.png")
        .intoMenuItem([]() {
            createQuickPopup(
                "Want to be a Helper?",
                "If you're interested in helping with the <cp>Level Grind</c> project, fill out an <cg>application form</c>.",
                "Cancel", "Apply",
                [](auto, bool btn2) {
                    if (btn2) {
                        web::openLinkInBrowser("https://forms.gle/RpLraEUuAZFuWw6E6");
                    }
                }
            );
        })
        .parent(m_buttonMenu)
        .centerX()
        .collect();

    auto* scrollBar = Build<Scrollbar>::create(m_list->getScrollLayer())
        .scale(0.9f)
        .pos(
            m_mainLayer->getScaledContentWidth() - 12.f,
            m_mainLayer->getContentHeight() / 2.f - 5.f
        )
        .parent(m_mainLayer)
        .collect();

    auto infoBtn = Build(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))
        .intoMenuItem([] {
            GuidePopup::create(GuidePage::CreditsInfo, GuidePopupState::FromOutside)->show();
        })
        .parent(m_buttonMenu)
        .pos(
            m_buttonMenu->getContentSize()
        )
        .collect();

    auto* updateSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    updateSpr->setScale(infoBtn->getContentHeight() / updateSpr->getContentHeight());
    m_updateBtn = CCMenuItemSpriteExtra::create(updateSpr, this, menu_selector(CreditsPopup::onUpdateProfile));
    m_updateBtn->setPosition({
        infoBtn->getPositionX() - infoBtn->getScaledContentWidth() / 2.f - 6.f - m_updateBtn->getScaledContentWidth() / 2.f,
        infoBtn->getPositionY()
    });
    m_updateBtn->setID("update-profile-btn"_spr);
    m_updateBtn->setVisible(false);
    m_buttonMenu->addChild(m_updateBtn);

    this->loadCredits();

    return true;
}

void CreditsPopup::onUpdateProfile(CCObject*) {
    if (m_updating) return;

    auto uPopup = UploadActionPopup::create(nullptr, "Updating...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;

    if (DataManager::getInstance().getUserToken().empty()) {
        uPopup->showFailMessage("Failed! Try again later\nStill logging you in.");
        return;
    }

    m_updating = true;

    Ref<CreditsPopup> self = this;
    m_updateListener.spawn(
        APIClient::getInstance().updateCreditsProfile(),
        [self, uPopupRef](web::WebResponse res) {
            if (!self) return;
            self->m_updating = false;

            auto parsed = APIClient::getInstance().updateCreditsProfileParse(res);

            if (!parsed.ok) {
                std::string msg = "Failed! Try again later";
                if (!parsed.error.empty()) msg += "\n" + parsed.error;
                if (uPopupRef) uPopupRef->showFailMessage(msg);
                return;
            }

            if (uPopupRef) uPopupRef->showSuccessMessage("Success! Updated user info.");

            self->applyProfileUpdate(
                GJAccountManager::get()->m_accountID,
                parsed.username, parsed.iconID, parsed.color1, parsed.color2, parsed.color3
            );
        }
    );
}

static void forEachPlayerNode(CCNode* node, geode::FunctionRef<void(CreditsPlayerNode*)> fn) {
    if (!node) return;
    if (auto pn = typeinfo_cast<CreditsPlayerNode*>(node)) {
        fn(pn);
        return;
    }
    for (auto child : CCArrayExt<CCNode>(node->getChildren())) {
        forEachPlayerNode(child, fn);
    }
}

void CreditsPopup::applyProfileUpdate(int accountId, const std::string& username, int icon, int color1, int color2, int color3) {
    CreditsPlayerNode::CreditsUser user;
    user.accountId = accountId;
    user.username = username;
    user.cube = icon;
    user.color1 = color1;
    user.color2 = color2;
    user.glowColor = color3;

    forEachPlayerNode(m_list, [&](CreditsPlayerNode* pn) {
        if (pn->getAccountId() == accountId) pn->applyUser(user);
    });
}

void CreditsPopup::loadCredits() {
    auto* loading = Build<LoadingSpinner>::create(50.f)
        .parent(m_list)
        .center()
        .collect();

    Ref<LoadingSpinner> loadingRef = loading;
    Ref<CreditsPopup> self = this;

    web::WebRequest req;

    m_listener.spawn(
        req.get(fmt::format("{}/get_credits", APIClient::getInstance().baseUrl)),
        [loadingRef, self](web::WebResponse res) {
            if (!loadingRef || !self) return;

            if (!res.ok()) {
                log::error("bad web req, err code: {}", res.code());
                loadingRef->removeFromParent();
                Notification::create("Failed to fetch credits", NotificationIcon::Error)->show();
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                log::warn("Failed to parse get_credits JSON");
                loadingRef->removeFromParent();
                Notification::create("Invalid server response", NotificationIcon::Error)->show();
                return;
            }

            auto json = jsonRes.unwrap();
            if (!json["success"].asBool().unwrapOrDefault()) {
                log::warn("Server returned success=false for get_credits");
                loadingRef->removeFromParent();
                return;
            }

            static constexpr std::pair<const char*, const char*> kCategories[] = {
                { "owners", "Coordinators" },
                { "developers", "Developers" },
                { "admins", "Admins" },
                { "helpers", "Helpers" },
                { "artists", "Artists" },
                { "contributors", "Contributors" },
                { "boosters", "Boosters" },
            };

            int myAccountId = GJAccountManager::get()->m_accountID;
            bool isListed = false;
            std::vector<CategoryCell::CreditsCategory> categories;

            for (auto& [key, title] : kCategories) {
                auto arr = json[key].asArray();
                if (!arr) {
                    log::warn("failed to get credits");
                    loadingRef->removeFromParent();
                    Notification::create("Invalid server response", NotificationIcon::Error)->show();
                    return;
                }

                CategoryCell::CreditsCategory cat;
                cat.categoryName = title;

                for (auto& userRes : arr.unwrap()) {
                    CreditsPlayerNode::CreditsUser user;
                    user.username = userRes["username"].asString().unwrapOrDefault();
                    user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                    user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                    user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                    user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                    user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                    if (myAccountId > 0 && user.accountId == myAccountId) isListed = true;

                    cat.users.push_back(user);
                }

                categories.push_back(std::move(cat));
            }

            for (auto& cat : categories) {
                self->m_list->addCell(CategoryCell::create(cat));
            }

            self->m_list->updateLayout();
            loadingRef->removeFromParent();

            if (self->m_updateBtn) self->m_updateBtn->setVisible(isListed);
        }
    );
}

}