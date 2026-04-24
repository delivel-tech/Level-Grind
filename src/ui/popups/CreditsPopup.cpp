#include "CreditsPopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Layout.hpp"
#include <Geode/Enums.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LoadingCircleSprite.hpp>
#include <Geode/binding/ProfilePage.hpp>

#include <Geode/binding/SimplePlayer.hpp>
#include <UIBuilder.hpp>
#include <cstddef>
#include <cue/ListNode.hpp>
#include <cue/PlayerIcon.hpp>
#include <fmt/format.h>
#include <unordered_map>

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

private:
    bool init(const CreditsUser& user) {
        CCNode::init();

        auto gm = GameManager::sharedState();

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
            {"Owners", "badge_owner.png"_spr},
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
            auto badge = Build<CCSprite>::create(currentCategoryBadgeName)
                .parent(titleMenu)
                .collect();

            m_badge = badge;
        }

        auto title = Build<CCLabelBMFont>::create(fmt::format("{}", cat.categoryName).c_str(), "bigFont.fnt")
            .parent(titleMenu)
            .collect();

        std::unordered_map<std::string, GuidePage> infoPageToOpen {
            {"Owners", GuidePage::OwnerRoleGuide},
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

        auto membersAmountLabel = Build(CCLabelBMFont::create(fmt::format("{}", cat.users.size()).c_str(), "chatFont.fnt"))
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
                        web::openLinkInBrowser("https://docs.google.com/forms/d/e/1FAIpQLScOUeCa13hvgnaWoJzkK1DXdOOhoNWwDmepYV4Tg2zj1prmMQ/viewform?usp=publish-editor");
                    }
                }
            );
        })
        .parent(m_buttonMenu)
        .centerX()
        .collect();

    auto* infoBtn = Build<CCSprite>::createSpriteName("GJ_infoIcon_001.png")
        .intoMenuItem([]() {
            MDPopup::create(
                "Credits Info",
                "These <cy>users</c> help with <cp>Level Grind</c>!\n\n" \
                "<cg>Helpers</c> are people who are responsible for <cf>adding / deleting levels</c> in the project.\n" \
                "<cy>Contributors</c> are people who contribute to the project in other helpful ways.\n" \
                "<cr>Admins</c> are people who have the same permissions as helpers, but also have the <cl>ability to manage helpers</c>!\n" \
                "Want to become a helper? Join our [<cg>Discord server</c>](https://discord.gg/tmf5xtCX5y) and fill out an [<cy>application form</c>](https://docs.google.com/forms/d/e/1FAIpQLScOUeCa13hvgnaWoJzkK1DXdOOhoNWwDmepYV4Tg2zj1prmMQ/viewform?usp=publish-editor)!\n\n" \
                "Have any <cy>suggestions</c> or want to <cy>report a bug</c>? Join our [<cg>Discord server</c>](https://discord.gg/tmf5xtCX5y) and let us know!",
                "OK"
            )->show();
        })
        .parent(m_buttonMenu)
        .pos(
            m_mainLayer->getContentSize().width,
            m_mainLayer->getContentSize().height - 3
        )
        .collect();

    auto* scrollBar = Build<Scrollbar>::create(m_list->getScrollLayer())
        .scale(0.9f)
        .pos(
            m_mainLayer->getScaledContentWidth() - 12.f,
            m_mainLayer->getContentHeight() / 2.f - 5.f
        )
        .parent(m_mainLayer)
        .collect();

    auto* loading = Build<LoadingSpinner>::create(50.f)
        .parent(m_list)
        .center()
        .collect();

    Ref<LoadingSpinner> loadingRef = loading;
    Ref<CreditsPopup> self = this;

    web::WebRequest req;

    m_listener.spawn(
        req.get("https://api.delivel.tech/get_credits"),
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

            CategoryCell::CreditsCategory ownersCat;
            ownersCat.categoryName = "Owners";
            CategoryCell::CreditsCategory adminsCat;
            adminsCat.categoryName = "Admins";
            CategoryCell::CreditsCategory helpersCat;
            helpersCat.categoryName = "Helpers";
            CategoryCell::CreditsCategory artistsCat;
            artistsCat.categoryName = "Artists";
            CategoryCell::CreditsCategory contributorsCat;
            contributorsCat.categoryName = "Contributors";
            CategoryCell::CreditsCategory boostersCat;
            boostersCat.categoryName = "Boosters";

            auto owners = json["owners"].asArray();
            auto admins = json["admins"].asArray();
            auto helpers = json["helpers"].asArray();
            auto artists = json["artists"].asArray();
            auto contributors = json["contributors"].asArray();
            auto boosters = json["boosters"].asArray();

            if (!owners || !admins || !helpers || !artists || !contributors || !boosters) {
                log::warn("failed to get credits");
                loadingRef->removeFromParent();
                Notification::create("Invalid server response", NotificationIcon::Error)->show();
                return;
            }

            for (auto& userRes : owners.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                ownersCat.users.push_back(user);
            }

            for (auto& userRes : admins.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                adminsCat.users.push_back(user);
            }

            for (auto& userRes : helpers.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                helpersCat.users.push_back(user);
            }

            for (auto& userRes : artists.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                artistsCat.users.push_back(user);
            }

            for (auto& userRes : contributors.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                contributorsCat.users.push_back(user);
            }

            for (auto& userRes : boosters.unwrap()) {
                CreditsPlayerNode::CreditsUser user;
                user.username = userRes["username"].asString().unwrapOrDefault();
                user.accountId = userRes["accountId"].asInt().unwrapOrDefault();
                user.color1 = userRes["color1"].asInt().unwrapOrDefault();
                user.color2 = userRes["color2"].asInt().unwrapOrDefault();
                user.glowColor = userRes["color3"].asInt().unwrapOrDefault();
                user.cube = userRes["iconid"].asInt().unwrapOrDefault();

                boostersCat.users.push_back(user);
            }

            self->m_list->addCell(CategoryCell::create(ownersCat));
            self->m_list->addCell(CategoryCell::create(adminsCat));
            self->m_list->addCell(CategoryCell::create(helpersCat));
            self->m_list->addCell(CategoryCell::create(artistsCat));
            self->m_list->addCell(CategoryCell::create(contributorsCat));
            self->m_list->addCell(CategoryCell::create(boostersCat));

            self->m_list->updateLayout();
            loadingRef->removeFromParent();
        }
    );

    return true;
}

}