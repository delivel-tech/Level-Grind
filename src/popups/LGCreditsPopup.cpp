#include "../popups/LGCreditsPopup.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

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

    auto borders = ListBorders::create();
    borders->setContentSize({340.f, 195.f});
    borders->setPosition({
        m_mainLayer->getContentSize().width / 2.f,
        m_mainLayer->getContentSize().height / 2.f - 5.f
    });
    m_mainLayer->addChild(borders);

    m_scrollLayer = scrollLayer;

    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    auto infoBtn = CCMenuItemSpriteExtra::create(
        infoSpr, this, menu_selector(LGCreditsPopup::onInfo)
    );
    infoBtn->setPosition({
        m_mainLayer->getContentSize().width,
        m_mainLayer->getContentSize().height - 3
    });
    m_buttonMenu->addChild(infoBtn);

    auto contentLayer = m_scrollLayer->m_contentLayer;
    if (contentLayer) {
        auto layout = ColumnLayout::create();
        layout->setGap(0.f);
        layout->setAutoGrowAxis(0.f);
        layout->setAxisReverse(true);
        contentLayer->setLayout(layout);

        auto spinner = LoadingSpinner::create(48.f);
        spinner->setPosition(contentLayer->getContentSize() / 2);
        contentLayer->addChild(spinner);
        m_spinner = spinner;
    }

    auto scrollbar = Scrollbar::create(m_scrollLayer);
    scrollbar->setPosition({
        m_mainLayer->getContentSize().width - 12.f,
        m_mainLayer->getContentSize().height / 2.f - 5.f
    });
    scrollbar->setScale(0.9f);
    m_mainLayer->addChild(scrollbar);

    auto joinBtn = CCMenuItemExt::createSpriteExtra(
        CCSprite::createWithSpriteFrameName("GJ_longBtn05_001.png"),
        [](CCObject*) {
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
        }
    );
    joinBtn->setPositionX(m_mainLayer->getContentSize().width / 2);
    m_buttonMenu->addChild(joinBtn);

    Ref<LGCreditsPopup> self = this;
    web::WebRequest req;

    async::spawn(
        req.get("https://delivel.tech/grindapi/get_credits"),
        [self](web::WebResponse res) {
            if (!self) return;

            auto removeSpinner = [&]() {
                if (self->m_spinner) {
                    self->m_spinner->removeFromParent();
                    self->m_spinner = nullptr;
                }
            };

            if (!res.ok()) {
                log::warn("get_credits returned non-ok status: {}", res.code());
                removeSpinner();
                Notification::create("Failed to fetch credits", NotificationIcon::Error)->show();
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                log::warn("Failed to parse get_credits JSON");
                removeSpinner();
                Notification::create("Invalid server response", NotificationIcon::Error)->show();
                return;
            }

            auto json = jsonRes.unwrap();
            if (!json["success"].asBool().unwrapOrDefault()) {
                log::warn("Server returned success=false for get_credits");
                removeSpinner();
                return;
            }

            auto content = self->m_scrollLayer ? self->m_scrollLayer->m_contentLayer : nullptr;
            if (!content) return;

            removeSpinner();
            content->removeAllChildrenWithCleanup(true);

            auto addHeader = [&](std::string_view text) {
                auto cell = TableViewCell::create();
                cell->setContentSize({340.f, 30.f});

                const float cellH = cell->getContentSize().height;
                const float dividerH = 1.f;

                if (content->getChildren()->count() > 0) {
                    auto topDivider = CCSprite::create();
                    topDivider->setTextureRect(CCRectMake(0, 0, 340.f, dividerH));
                    topDivider->setPosition({170.f, cellH - dividerH / 2.f});
                    topDivider->setColor({0, 0, 0});
                    topDivider->setOpacity(80);
                    cell->addChild(topDivider, 2);
                }

                auto bottomDivider = CCSprite::create();
                bottomDivider->setTextureRect(CCRectMake(0, 0, 340.f, dividerH));
                bottomDivider->setPosition({170.f, dividerH / 2.f});
                bottomDivider->setColor({0, 0, 0});
                bottomDivider->setOpacity(80);
                cell->addChild(bottomDivider, 2);

                auto row = CCMenu::create();
                row->setPosition({170.f, 15.f});
                row->setAnchorPoint({0.5f, 0.5f});
                row->setContentSize({340.f, 30.f});

                auto rowLayout = RowLayout::create();
                rowLayout->setGap(7.f);
                rowLayout->setAxisAlignment(AxisAlignment::Center);
                rowLayout->setCrossAxisAlignment(AxisAlignment::Center);
                row->setLayout(rowLayout);

                const char* badgeName = "badge_helper.png"_spr;
                if (text == "Owners") badgeName = "badge_owner.png"_spr;
                else if (text == "Admins") badgeName = "badge_admin.png"_spr;
                else if (text == "Contributors") badgeName = "badge_contributor.png"_spr;
                else if (text == "Artists") badgeName = "badge_artist.png"_spr;
                else if (text == "Boosters") badgeName = "badge_booster.png"_spr;

                auto badgeSpr = CCSprite::create(badgeName);
                if (badgeSpr) {
                    auto badgeWrapper = CCNode::create();
                    float bw = badgeSpr->getContentSize().width * 0.45f;
                    float bh = badgeSpr->getContentSize().height * 0.45f;
                    badgeWrapper->setContentSize({bw, bh});
                    badgeSpr->setScale(0.65f);
                    badgeSpr->setPosition({bw / 2.f, bh / 2.f});
                    badgeWrapper->addChild(badgeSpr);
                    row->addChild(badgeWrapper);
                }

                auto label = CCLabelBMFont::create(std::string{text}.c_str(), "bigFont.fnt");
                auto labelWrapper = CCNode::create();
                float lw = label->getContentSize().width * 0.45f;
                float lh = label->getContentSize().height * 0.45f;
                labelWrapper->setContentSize({lw, lh});
                label->setScale(0.45f);
                label->setPosition({lw / 2.f, lh / 2.f});
                labelWrapper->addChild(label);
                row->addChild(labelWrapper);

                auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
                auto infoBtn = CCMenuItemSpriteExtra::create(
                    infoSpr, self, menu_selector(LGCreditsPopup::onHeaderInfo)
                );
                float iw = infoSpr->getContentSize().width * 0.45f;
                float ih = infoSpr->getContentSize().height * 0.45f;
                infoBtn->setContentSize({iw, ih});
                infoSpr->setScale(0.45f);
                infoSpr->setPosition({iw / 2.f, ih / 2.f});

                int infoTag = 0;
                if (text == "Owners") infoTag = 1;
                else if (text == "Admins") infoTag = 2;
                else if (text == "Helpers") infoTag = 3;
                else if (text == "Contributors") infoTag = 4;
                else if (text == "Artists") infoTag = 5;
                else if (text == "Boosters") infoTag = 6;
                infoBtn->setTag(infoTag);
                row->addChild(infoBtn);

                row->updateLayout();
                cell->addChild(row, 1);
                content->addChild(cell);
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

                auto bg = CCSprite::create();
                bg->setTextureRect(CCRectMake(0, 0, 340.f, 50.f));
                bg->setPosition({170.f, 25.f});
                bg->setOpacity(95);
                if (isOwner) bg->setColor({150, 255, 255});
                else if (isAdmin) bg->setColor({245, 107, 107});
                else bg->setColor({81, 147, 248});
                cell->addChild(bg);

                auto gm = GameManager::sharedState();
                auto player = SimplePlayer::create(iconId);
                player->updatePlayerFrame(iconId, IconType::Cube);
                player->setColors(gm->colorForIdx(color1), gm->colorForIdx(color2));
                if (color3 != 0) player->setGlowOutline(gm->colorForIdx(color3));
                player->setPosition({40.f, 25.f});
                cell->addChild(player);

                auto nameLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
                nameLabel->setAnchorPoint({0.f, 0.5f});
                nameLabel->setScale(0.8f);

                auto menu = CCMenu::create();
                menu->setPosition({0.f, 0.f});

                auto nameBtn = CCMenuItemSpriteExtra::create(
                    nameLabel, self, menu_selector(LGCreditsPopup::onAccountClicked)
                );
                nameBtn->setTag(accountId);
                nameBtn->setPosition({80.f, 25.f});
                nameBtn->setAnchorPoint({0.f, 0.5f});
                menu->addChild(nameBtn);
                cell->addChild(menu);

                content->addChild(cell);
            };

            struct Section {
                std::string key;
                std::string header;
                bool isAdmin;
                bool isOwner;
            };

            std::vector<Section> sections = {
                {"owners", "Owners",true, true},
                {"admins", "Admins", true, false},
                {"helpers", "Helpers", false, false},
                {"artists", "Artists", false, false},
                {"contributors", "Contributors", false, false},
                {"boosters", "Boosters", false, false},
            };

            bool hasAny = false;
            for (auto const& section : sections) {
                if (!json.contains(section.key) || !json[section.key].isArray()) continue;
                auto arr = json[section.key].asArray().unwrap();
                if (arr.empty()) continue;

                hasAny = true;
                addHeader(section.header);
                for (auto const& val : arr) {
                    addPlayer(val, section.isAdmin, section.isOwner);
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
            "Owners",
            "<co>Owners</c> are main people in <cp>Level Grind</c>. " \
            "They are responsible for <cg>development</c>, <cy>leading the project</c>, <cf>making final decisions</c>, and <cl>managing all roles</c>.",
            "OK"
        )->show();
        return;
    }

    if (tag == 2) {
        FLAlertLayer::create(
            "Admins",
            "<cr>Admins</c> have all helper permissions and can also " \
            "<cl>manage helpers</c> to keep the project organized.",
            "OK"
        )->show();
        return;
    }

    if (tag == 3) {
        FLAlertLayer::create(
            "Helpers",
            "<cg>Helpers</c> are responsible for <cf>adding and deleting levels</c> " \
            "in the Level Grind project.",
            "OK"
        )->show();
        return;
    }

    if (tag == 4) {
        FLAlertLayer::create(
            "Contributors",
            "<cy>Contributors</c> help the <cp>Level Grind</c> project through various contributions." \
            " They may be artists of the mod, people who actively help with testing and so on.",
            "OK"
        )->show();
        return;
    }

    if (tag == 5) {
        FLAlertLayer::create(
            "Artists",
            "<cy>Artists</c> are responsible for <cg>visual part of the mod</c>. " \
            "Their <cp>work is greatly appreciated</c>!",
            "OK"
        )->show();
    }

    if (tag == 6) {
        FLAlertLayer::create(
            "Boosters",
            "<cy>Boosters</c> boost the <cp>Level Grind Discord server</c>. " \
            "Their <cy>support is greatly appreciated</c>!",
            "OK"
        )->show();
    }
}

void LGCreditsPopup::onInfo(CCObject* sender) {
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
}