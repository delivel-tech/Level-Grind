#include "FindPlayerPopup.hpp"
#include <Geode/ui/Label.hpp>
#include "GrindProfilePopup.hpp"
#include "../../managers/GDUsers.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <algorithm>

namespace levelgrind {

FindPlayerPopup* FindPlayerPopup::create() {
    auto ret = new FindPlayerPopup;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool FindPlayerPopup::init() {
    if (!BasePopup::init(280.f, 140.f)) return false;
    this->setTitle("Find Player");

    m_input = TextInput::create(220.f, "Username or account ID", "bigFont.fnt");
    m_input->setCommonFilter(CommonFilter::Any);
    m_input->setMaxCharCount(20);
    m_input->setPosition({ centerX(), centerY() + 6.f });
    m_mainLayer->addChild(m_input);

    m_status = geode::Label::create("", "bigFont.fnt");
    m_status->setScale(0.3f);
    m_status->setPosition({ centerX(), centerY() - 20.f });
    m_mainLayer->addChild(m_status);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu);

    auto spr = ButtonSprite::create("Find", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    spr->setScale(0.75f);
    Ref<FindPlayerPopup> self = this;
    auto btn = CCMenuItemExt::createSpriteExtra(spr, [self](CCObject*) {
        if (self) self->find();
    });
    btn->setPosition({ centerX(), 22.f });
    menu->addChild(btn);
    return true;
}

void FindPlayerPopup::find() {
    if (m_busy) return;
    auto query = string::trim(m_input->getString());
    if (query.empty()) {
        return;
    }

    bool numeric = std::all_of(query.begin(), query.end(), [](unsigned char c) { return std::isdigit(c); });
    if (numeric) {
        if (auto id = numFromString<int>(query).unwrapOr(0); id > 0) {
            GrindProfilePopup::create(id)->show();
            return;
        }
    }

    m_busy = true;
    m_status->setString("Searching...");
    Ref<FindPlayerPopup> self = this;
    m_task.spawn(gdusers::search(query), [self, query](web::WebResponse res) {
        if (!self) return;
        self->m_busy = false;
        auto user = gdusers::parseSearch(res);
        if (user) {
            self->m_status->setString("");
            GrindProfilePopup::create(user->accountId, user->name)->show();
            return;
        }
        self->m_status->setString("");
        GrindProfilePopup::create(0, query)->show();
    });
}

}
