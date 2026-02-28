#include "../popups/LGCreditsPopup.hpp"
#include "Geode/ui/General.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <vector>

using namespace geode::prelude;

LGCreditsPopup* LGCreditsPopup::create() {
    auto ret = new LGCreditsPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LGCreditsPopup::init() {
    if (!Popup::init(380.f, 250.f)) return false;

    setTitle("Level Grind Credits");

    auto scrollLayer = ScrollLayer::create({340.f, 195.f});
    scrollLayer->setPosition({20.f, 23.f});
    m_mainLayer->addChild(scrollLayer);
    addListBorders(m_mainLayer,
                 {m_mainLayer->getContentSize().width / 2,
                  m_mainLayer->getContentSize().height / 2 - 5.f},
                 {340.f, 195.f});

    m_scrollLayer = scrollLayer;

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    
    auto infoBtn = CCMenuItemSpriteExtra::create(
        infoSpr,
        this,
        menu_selector(LGCreditsPopup::onInfo)
    );
    infoBtn->setPosition({m_mainLayer->getContentSize().width,
                        m_mainLayer->getContentSize().height - 3});
    m_buttonMenu->addChild(infoBtn);

    auto contentLayer = m_scrollLayer->m_contentLayer;
    if (contentLayer) {
        auto layout = ColumnLayout::create();
        contentLayer->setLayout(layout);
        layout->setGap(0.f);
        layout->setAutoGrowAxis(0.f);
        layout->setAxisReverse(true);

        auto spinner = LoadingSpinner::create(48.f);
        spinner->setPosition(contentLayer->getContentSize() / 2);
        contentLayer->addChild(spinner);
        m_spinner = spinner;
    }

    Ref<LGCreditsPopup> self = this;

    web::WebRequest req;

    async::spawn(
        req.get("https://delivel.tech/grindapi/get_credits"),
        [self](web::WebResponse res) {
            if (!self) return;
            if (!res.ok()) {
                log::warn("get_credits returned non-ok status: {}", res.code());
                if (self->m_spinner) {
                    self->m_spinner->removeFromParent();
                    self->m_spinner = nullptr;
                }
                Notification::create("Failed to fetch credits",
                               NotificationIcon::Error)->show();
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                log::warn("Failed to parse get_credits JSON");
                if (self->m_spinner) {
                    self->m_spinner->removeFromParent();
                    self->m_spinner = nullptr;
                }
                Notification::create("Invalid server response",
                               NotificationIcon::Error)->show();
                return;
            }

            auto json = jsonRes.unwrap();
            bool success = json["success"].asBool().unwrapOrDefault();
            if (!success) {
                log::warn("Server returned success=false for get_credits");
                if (self->m_spinner) {
                    self->m_spinner->removeFromParent();
                    self->m_spinner = nullptr;
                }
                return;
            }

            auto content = self->m_scrollLayer ? self->m_scrollLayer->m_contentLayer : nullptr;
            if (!content) return;

            if (self->m_spinner) {
                self->m_spinner->removeFromParent();
                self->m_spinner = nullptr;
            }

            content->removeAllChildrenWithCleanup(true);

            auto addHeader = [&](std::string_view text) {
                auto tableCell = TableViewCell::create();
                tableCell->setContentSize({340.f, 30.f});

                auto label = CCLabelBMFont::create(std::string{text}.c_str(), "bigFont.fnt");
                label->setScale(0.45f);
                label->setAnchorPoint({0.5f, 0.5f});
                auto labelPos = CCPoint { tableCell->getContentSize().width / 2.f, 15.f };
                label->setPosition(labelPos);
                tableCell->addChild(label);

                auto headerMenu = CCMenu::create();
                headerMenu->setPosition({0.f, 0.f});

                auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
                infoSpr->setScale(0.45f);
                auto infoBtn = CCMenuItemSpriteExtra::create(
                    infoSpr,
                    self,
                    menu_selector(LGCreditsPopup::onHeaderInfo)
                );

                int infoTag = 0;
                if (text == "Owner") infoTag = 1;
                else if (text == "Admins") infoTag = 2;
                else if (text == "Helpers") infoTag = 3;
                infoBtn->setTag(infoTag);

                auto labelWidth = label->getContentSize().width * label->getScale();
                auto infoWidth = infoSpr->getContentSize().width * infoSpr->getScale();
                float infoX = labelPos.x + (labelWidth / 2.f) + 8.f + (infoWidth / 2.f);
                infoBtn->setPosition({infoX, labelPos.y});

                headerMenu->addChild(infoBtn);
                tableCell->addChild(headerMenu);

                const float contentH = tableCell->getContentSize().height;
                const float dividerH = 1.f;
                const float halfDivider = dividerH / 2.f;

                if (content->getChildren()->count() > 0) {
                    auto topDivider = CCSprite::create();
                    topDivider->setTextureRect(CCRectMake(0, 0, tableCell->getContentSize().width, dividerH));
                    topDivider->setPosition({tableCell->getContentSize().width / 2.f, contentH - halfDivider});
                    topDivider->setColor({0, 0, 0});
                    topDivider->setOpacity(80);
                    tableCell->addChild(topDivider, 2);
                }

                auto bottomDivider = CCSprite::create();
                bottomDivider->setTextureRect(CCRectMake(0, 0, tableCell->getContentSize().width, dividerH));
                bottomDivider->setPosition({tableCell->getContentSize().width / 2.f, halfDivider});
                bottomDivider->setColor({0, 0, 0});
                bottomDivider->setOpacity(80);
                tableCell->addChild(bottomDivider, 2);

                content->addChild(tableCell);
            };

            auto addPlayer = [&](const matjson::Value& userVal, bool isAdmin, bool isOwner) {
                if (!userVal.isObject()) return;

                int accountId = userVal["accountId"].asInt().unwrapOrDefault();
                std::string username = userVal["username"].asString().unwrapOrDefault();
                int iconId = userVal["iconid"].asInt().unwrapOrDefault();
                int color1 = userVal["color1"].asInt().unwrapOrDefault();
                int color2 = userVal["color2"].asInt().unwrapOrDefault();
                int color3 = userVal["color3"].asInt().unwrapOrDefault();

                auto cell = TableViewCell::create();
                cell->setContentSize({340.f, 50.f});

                auto bgSprite = CCSprite::create();
                bgSprite->setTextureRect(CCRectMake(0, 0, 340.f, 50.f));
                bgSprite->setPosition({170.f, 25.f});
                bgSprite->setOpacity(95);
                if (isOwner) {
                    bgSprite->setColor({150, 255, 255});
                } else if (isAdmin) {
                    bgSprite->setColor({245, 107, 107});
                } else {
                    bgSprite->setColor({81, 147, 248});
                }
                cell->addChild(bgSprite);

                auto gm = GameManager::sharedState();
                auto player = SimplePlayer::create(iconId);
                player->updatePlayerFrame(iconId, IconType::Cube);
                player->setColors(gm->colorForIdx(color1), gm->colorForIdx(color2));
                if (color3 != 0) {
                    player->setGlowOutline(gm->colorForIdx(color3));
                }
                player->setPosition({40.f, 25.f});
                cell->addChild(player);

                auto nameLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
                nameLabel->setAnchorPoint({0.f, 0.5f});
                nameLabel->setScale(0.8f);

                auto menu = CCMenu::create();
                menu->setPosition({0.f, 0.f});

                auto nameBtn = CCMenuItemSpriteExtra::create(
                    nameLabel,
                    self,
                    menu_selector(LGCreditsPopup::onAccountClicked)
                );
                nameBtn->setTag(accountId);
                nameBtn->setPosition({80.f, 25.f});
                nameBtn->setAnchorPoint({0.f, 0.5f});
                menu->addChild(nameBtn);
                cell->addChild(menu);

                content->addChild(cell);
            };

            bool hasAny = false;

            std::vector<matjson::Value> ownerUsers;
            std::vector<matjson::Value> adminUsers;

            if (json.contains("admins") && json["admins"].isArray()) {
                auto admins = json["admins"].asArray().unwrap();
                for (auto const& val : admins) {
                    if (!val.isObject()) continue;
                    int accountId = val["accountId"].asInt().unwrapOrDefault();
                    std::string username = val["username"].asString().unwrapOrDefault();
                    bool isDelivel = accountId == 13678537 || username == "Delivel" || username == "delivel";
                    if (isDelivel) {
                        ownerUsers.push_back(val);
                    } else {
                        adminUsers.push_back(val);
                    }
                }
            }

            if (!ownerUsers.empty()) {
                hasAny = true;
                addHeader("Owner");
                for (auto const& val : ownerUsers) {
                    addPlayer(val, true, true);
                }
            }

            if (!adminUsers.empty()) {
                hasAny = true;
                addHeader("Admins");
                for (auto const& val : adminUsers) {
                    addPlayer(val, true, false);
                }
            }

            if (json.contains("helpers") && json["helpers"].isArray()) {
                auto helpers = json["helpers"].asArray().unwrap();
                if (!helpers.empty()) {
                    hasAny = true;
                    addHeader("Helpers");
                    for (auto const& val : helpers) {
                        addPlayer(val, false, false);
                    }
                }
            }

            if (!hasAny) {
                auto emptyCell = TableViewCell::create();
                emptyCell->setContentSize({340.f, 40.f});
                auto emptyLabel = CCLabelBMFont::create("No credits found", "bigFont.fnt");
                emptyLabel->setScale(0.45f);
                emptyLabel->setAnchorPoint({0.5f, 0.5f});
                emptyLabel->setPosition({170.f, 20.f});
                emptyCell->addChild(emptyLabel);
                content->addChild(emptyCell);
            }

            content->updateLayout();
            if (self->m_scrollLayer) {
                self->m_scrollLayer->scrollToTop();
            }
        }
    );

    return true;
}

void LGCreditsPopup::onAccountClicked(CCObject* sender) {
    auto button = static_cast<CCMenuItem*>(sender);
    int accountId = button->getTag();
    ProfilePage::create(accountId, false)->show();
}

void LGCreditsPopup::onHeaderInfo(CCObject* sender) {
    auto button = static_cast<CCMenuItem*>(sender);
    int tag = button->getTag();

    if (tag == 1) {
        FLAlertLayer::create(
            "Owner",
            "<co>Owner</c> is the main person in <cp>Level Grind</c>.\n"
            "They are responsible for <cg>development</c>, <cy>leading the project</c>, <cf>making final decisions</c>, and <cl>managing all roles</c>.",
            "OK"
        )->show();
        return;
    }

    if (tag == 2) {
        FLAlertLayer::create(
            "Admins",
            "<cr>Admins</c> have all helper permissions and can also\n"
            "<cl>manage helpers</c> to keep the project organized.",
            "OK"
        )->show();
        return;
    }

    if (tag == 3) {
        FLAlertLayer::create(
            "Helpers",
            "<cg>Helpers</c> are responsible for <cf>adding and deleting levels</c>\n"
            "in the Level Grind project.",
            "OK"
        )->show();
        return;
    }
}

void LGCreditsPopup::onInfo(CCObject* sender) {
    MDPopup::create(
        "Credits Info",
        "These <cy>users</c> help with <cp>Level Grind</c>!\n\n" \
        "<cg>Helpers</c> are people who are responsible for <cf>adding / deleting levels</c> in the project.\n" \
        "<cr>Admins</c> are people who have the same permissions as helpers, but also have the <cl>ability to manage helpers</c>!\n" \
        "Want to become a helper? DM <cr>admins</c> on our [<cg>Discord server</c>](https://discord.gg/tmf5xtCX5y)!\n\n" \
        "Have any <cy>suggestions</c> or want to <cy>report a bug</c>? Join our [<cg>Discord server</c>](https://discord.gg/tmf5xtCX5y) and let us know!",
        "OK"
    )->show();
}