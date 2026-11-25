#include "ManageListPopup.hpp"
#include <Geode/ui/Label.hpp>
#include "NotesManagerPopup.hpp"
#include "../../managers/APIClient.hpp"
#include "../../managers/GrindClansManager.hpp"
#include "../../managers/DataManager.hpp"
#include "../../managers/GrindListsData.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/Notification.hpp>
#include <fmt/format.h>

namespace levelgrind {

namespace {

geode::Label* lbl(CCNode* parent, std::string const& text, CCPoint pos, float scale, const char* font = "bigFont.fnt") {
    auto l = geode::Label::create(text, font);
    l->setScale(scale);
    l->setPosition(pos);
    parent->addChild(l);
    return l;
}

}

ManageListPopup* ManageListPopup::create(GJLevelList* list, CCArray* levels) {
    auto ret = new ManageListPopup;
    if (ret->init(list, levels)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ManageListPopup::collect(CCArray* levels) {
    std::unordered_map<int, GJGameLevel*> known;
    if (levels) {
        for (auto obj : CCArrayExt<CCObject*>(levels)) {
            if (auto level = typeinfo_cast<GJGameLevel*>(obj)) known[level->m_levelID.value()] = level;
        }
    }
    auto glm = GameLevelManager::get();
    m_levels.clear();
    m_loaded = 0;
    for (int id : m_list->m_levels) {
        GJGameLevel* level = nullptr;
        if (auto it = known.find(id); it != known.end()) level = it->second;
        if (!level && glm->m_onlineLevels) {
            level = typeinfo_cast<GJGameLevel*>(glm->m_onlineLevels->objectForKey(numToString(id)));
        }
        if (!level) level = glm->getSavedLevel(id);

        Level l;
        l.id = id;
        if (level) {
            l.stars = level->m_stars.value();
            l.moon = level->isPlatformer();
            l.demon = level->m_demon.value() != 0;
            l.coins = level->m_coins;
            l.loaded = true;
            m_loaded++;
        }
        m_levels.push_back(l);
    }
}

bool ManageListPopup::init(GJLevelList* list, CCArray* levels) {
    if (!BasePopup::init(280.f, 190.f)) return false;
    m_list = list;
    this->collect(levels);

    for (auto const& l : m_levels) {
        if (!l.loaded) continue;
        if (l.stars > 0) m_tags |= l.moon ? lists::TagMoon : lists::TagStar;
        if (l.coins > 0) m_tags |= lists::TagCoin;
        if (l.demon) m_tags |= lists::TagDemon;
    }

    auto pos = DataManager::getInstance().getUserPosition();
    this->setTitle(pos == GrindPosition::Owner ? "Grind Coordinator: Manage List" : "Grind Admin: Manage List");
    if (m_title->getScaledContentWidth() > 250.f) m_title->setScale(m_title->getScale() * 250.f / m_title->getScaledContentWidth());

    auto beta = lbl(m_mainLayer, "Beta", { centerX(), top() - 38.f }, 0.4f, "goldFont.fnt");
    beta->setOpacity(200);

    CCSize const boxSize { 240.f, 50.f };
    for (float y : { 108.f, 48.f }) {
        auto box = NineSlice::create("square02_small.png");
        box->setContentSize(boxSize);
        box->setOpacity(80);
        box->setPosition({ centerX(), y });
        m_mainLayer->addChild(box);
    }

    m_tagMenu = CCMenu::create();
    m_tagMenu->setLayout(RowLayout::create()->setGap(8.f)->setAutoScale(false));
    m_tagMenu->setContentSize({ boxSize.width - 12.f, 36.f });
    m_tagMenu->setPosition({ centerX(), 108.f });
    m_mainLayer->addChild(m_tagMenu);
    this->buildTags();

    m_actions = CCMenu::create();
    m_actions->setLayout(RowLayout::create()->setGap(8.f));
    m_actions->setContentSize({ boxSize.width - 12.f, 36.f });
    m_actions->setPosition({ centerX(), 48.f });
    m_mainLayer->addChild(m_actions);

    m_status = lbl(m_mainLayer, "Checking...", { centerX(), 48.f }, 0.4f);

    this->loadInfo();
    return true;
}

void ManageListPopup::buildTags() {
    m_tagMenu->removeAllChildren();
    struct { const char* icon; int tag; } const tags[] = {
        { "GJ_starsIcon_001.png", lists::TagStar },
        { "GJ_moonsIcon_001.png", lists::TagMoon },
        { "GJ_coinsIcon2_001.png", lists::TagCoin },
        { "GJ_demonIcon_001.png", lists::TagDemon },
    };
    for (auto const& t : tags) {
        auto make = [&](bool on) {
            auto bg = CCSprite::create(on ? "GJ_button_02.png" : "GJ_button_04.png");
            auto icon = CCSprite::createWithSpriteFrameName(t.icon);
            icon->setPosition(bg->getContentSize() / 2);
            if (!on) icon->setColor({ 150, 150, 150 });
            bg->addChild(icon);
            bg->setScale(0.85f);
            return bg;
        };
        int tag = t.tag;
        auto toggler = CCMenuItemExt::createToggler(make(true), make(false), [this, tag](CCMenuItemToggler*) {
            m_tags ^= tag;
        });
        toggler->toggle((m_tags & tag) != 0);
        m_tagMenu->addChild(toggler);
    }
    m_tagMenu->updateLayout();
}

void ManageListPopup::setAdded(bool added, std::string const&, long long) {
    m_added = added;
    m_status->setVisible(false);

    m_actions->removeAllChildren();
    m_actions->addChild(CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create(added ? "Update" : "Add", "goldFont.fnt", "GJ_button_01.png", 0.75f),
        [this](CCObject*) { this->onAdd(); }));
    if (added) {
        m_actions->addChild(CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Delete", "goldFont.fnt", "GJ_button_06.png", 0.75f),
            [this](CCObject*) { this->onRemove(); }));
    }
    Ref<GJLevelList> list = m_list;
    m_actions->addChild(CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Notes", "goldFont.fnt", "GJ_button_03.png", 0.75f),
        [list](CCObject*) {
            if (list) NotesManagerPopup::create(NotesManagerPopup::Target::List, list->m_listID, list->m_listName)->show();
        }));
    m_actions->updateLayout();
}

void ManageListPopup::loadInfo() {
    auto req = web::WebRequest();
    m_task.spawn(
        req.get(fmt::format("{}/list_info?id={}", APIClient::getInstance().baseUrl, m_list->m_listID)),
        [this](web::WebResponse res) {
            auto json = res.json();
            if (!res.ok() || !json) {
                this->setAdded(false, "", 0);
                Notification::create(res.code() == 404 ? "Grind Lists are not live on the server yet" : "Could not check the list",
                    NotificationIcon::Warning)->show();
                return;
            }
            auto v = json.unwrap();
            if (int tags = static_cast<int>(v["tags"].asInt().unwrapOr(0)); tags > 0) {
                m_tags = tags;
                this->buildTags();
            }
            this->setAdded(
                v["added"].asBool().unwrapOr(false),
                v["addedBy"].asString().unwrapOrDefault(),
                v["addedAt"].asInt().unwrapOr(0)
            );
        }
    );
}

matjson::Value ManageListPopup::listBody() {
    matjson::Value list;
    list["id"] = m_list->m_listID;
    list["name"] = std::string(m_list->m_listName);
    list["description"] = m_list->getUnpackedDescription();
    list["author"] = std::string(m_list->m_creatorName);
    list["accountId"] = m_list->m_accountID;
    list["difficulty"] = m_list->m_difficulty;
    list["diamonds"] = m_list->m_diamonds;
    list["levelsRequired"] = m_list->m_levelsToClaim;
    list["featured"] = m_list->m_featured;
    list["likes"] = m_list->m_likes;
    list["downloads"] = m_list->m_downloads;
    list["uploadDate"] = m_list->m_uploadDate;
    std::vector<matjson::Value> levels;
    for (auto const& l : m_levels) {
        matjson::Value lv;
        lv["id"] = l.id;
        lv["stars"] = l.stars;
        lv["moon"] = l.moon;
        lv["demon"] = l.demon;
        lv["coins"] = l.coins;
        lv["loaded"] = l.loaded;
        levels.push_back(lv);
    }
    list["levels"] = levels;
    list["tags"] = m_tags;
    return list;
}

void ManageListPopup::onAdd() {
    if (m_levels.empty()) {
        Notification::create("This list has no levels", NotificationIcon::Error)->show();
        return;
    }
    if (m_tags == 0) {
        Notification::create("Pick at least one filter", NotificationIcon::Error)->show();
        return;
    }
    auto body = GrindClansManager::authBody();
    body["list"] = this->listBody();
    Ref<ManageListPopup> self = this;
    bool wasAdded = m_added;
    GrindClansManager::getInstance().runAction(
        "list_add", std::move(body), wasAdded ? "Updating list..." : "Adding list...",
        wasAdded ? "List updated!" : "List added to Grind Lists!",
        [self](matjson::Value const&) {
            if (self->getParent()) self->loadInfo();
        }
    );
}

void ManageListPopup::onRemove() {
    Ref<ManageListPopup> self = this;
    createQuickPopup(
        "Delete List",
        fmt::format("Remove <cy>{}</c> from Grind Lists?", std::string(m_list->m_listName)),
        "Cancel", "Delete",
        [self](auto, bool yes) {
            if (!yes) return;
            auto body = GrindClansManager::authBody();
            body["listId"] = self->m_list->m_listID;
            GrindClansManager::getInstance().runAction(
                "list_delete", std::move(body), "Deleting list...", "List deleted.",
                [self](matjson::Value const&) {
                    if (self->getParent()) self->setAdded(false, "", 0);
                }
            );
        }
    );
}

}
