#include "CreateClanPopup.hpp"
#include "../../managers/APIClient.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/ColorChannelSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/general.hpp"
#include <UIBuilder.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

namespace levelgrind {

CreateClanPopup* CreateClanPopup::create(std::function<void()> onCreated) {
    auto ret = new CreateClanPopup;
    if (ret && ret->init(onCreated)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CreateClanPopup::init(std::function<void()> onCreated) {
    if (!BasePopup::init({ 340.f, 270.f }, "GJ_square05.png")) return false;

    m_onCreated = onCreated;
    setTitle("Create Clan");

    m_nameInput = Build(TextInput::create(280.f, "Clan name", "chatFont.fnt"))
        .scale(0.9f)
        .pos(centerX(), centerY() + 85.f)
        .parent(m_mainLayer)
        .id("name-input")
        .with([](TextInput* ti) {
            ti->setCommonFilter(CommonFilter::Any);
            ti->setMaxCharCount(32);
        })
        .collect();

    m_descInput = Build(TextInput::create(280.f, "Description (optional)", "chatFont.fnt"))
        .scale(0.9f)
        .pos(centerX(), centerY() + 55.f)
        .parent(m_mainLayer)
        .id("desc-input")
        .with([](TextInput* ti) {
            ti->setCommonFilter(CommonFilter::Any);
            ti->setMaxCharCount(150);
        })
        .collect();

    Build(CCLabelBMFont::create("Tracks", "bigFont.fnt"))
        .scale(0.45f)
        .anchorPoint({ 0.f, 0.5f })
        .pos(centerX() - 50.f, centerY() + 24.f)
        .parent(m_mainLayer)
        .collect();

    auto typeMenu = Build(CCMenu::create())
        .pos(centerX() + 30.f, centerY() + 23.f)
        .layout(RowLayout::create()->setGap(8))
        .id("type-menu")
        .scale(0.7f)
        .parent(m_mainLayer)
        .collect();

    Build(m_typeRadio.createToggler(
        LGClanType::Star,
        CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"),
        Build(CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png")).color({ 100, 100, 100 }).collect()
    ))
        .scale(0.9f)
        .id("star-toggler")
        .parent(typeMenu)
        .collect();

    Build(m_typeRadio.createToggler(
        LGClanType::Moon,
        CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png"),
        Build(CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png")).color({ 100, 100, 100 }).collect()
    ))
        .scale(0.9f)
        .id("moon-toggler")
        .parent(typeMenu)
        .collect();

    typeMenu->updateLayout();

    Build(CCLabelBMFont::create("Join type", "bigFont.fnt"))
        .scale(0.45f)
        .anchorPoint({ 0.f, 0.5f })
        .pos(centerX() - 140.f, centerY() - 5.f)
        .parent(m_mainLayer)
        .collect();

    auto joinMenu = Build(CCMenu::create())
        .pos(centerX() + 40.f, centerY() - 5.f)
        .layout(RowLayout::create()->setGap(6))
        .scale(0.5f)
        .id("join-menu")
        .parent(m_mainLayer)
        .collect();

    Build(m_joinRadio.createToggler(
        LGClanJoinType::Open,
        ButtonSprite::create("Open", "bigFont.fnt", "GJ_button_02.png"),
        ButtonSprite::create("Open", "bigFont.fnt", "GJ_button_04.png")
    ))
        .scale(0.7f)
        .id("open-toggler")
        .parent(joinMenu)
        .collect();

    Build(m_joinRadio.createToggler(
        LGClanJoinType::ByRequest,
        ButtonSprite::create("Request", "bigFont.fnt", "GJ_button_02.png"),
        ButtonSprite::create("Request", "bigFont.fnt", "GJ_button_04.png")
    ))
        .scale(0.7f)
        .id("request-toggler")
        .parent(joinMenu)
        .collect();

    Build(m_joinRadio.createToggler(
        LGClanJoinType::Closed,
        ButtonSprite::create("Closed", "bigFont.fnt", "GJ_button_02.png"),
        ButtonSprite::create("Closed", "bigFont.fnt", "GJ_button_04.png")
    ))
        .scale(0.7f)
        .id("closed-toggler")
        .parent(joinMenu)
        .collect();

    joinMenu->updateLayout();

    Build(CCLabelBMFont::create("Min stat to join:", "bigFont.fnt"))
        .scale(0.4f)
        .anchorPoint({ 0.f, 0.5f })
        .pos(centerX() - 110.f, centerY() - 38.f)
        .parent(m_mainLayer)
        .collect();

    m_statReqInput = Build(TextInput::create(90.f, "0", "chatFont.fnt"))
        .pos(centerX() + 55.f, centerY() - 38.f)
        .parent(m_mainLayer)
        .id("stat-req-input")
        .with([](TextInput* ti) {
            ti->setCommonFilter(CommonFilter::Int);
            ti->setMaxCharCount(9);
            ti->setString("0");
        })
        .collect();

    Build(CCLabelBMFont::create("Cost: 1000 pet stars + 200 moons", "bigFont.fnt"))
        .scale(0.4f)
        .color({ 255, 220, 100 })
        .pos(centerX(), centerY() - 67.f)
        .parent(m_mainLayer)
        .collect();

    Build(ButtonSprite::create("Create", "bigFont.fnt", "GJ_button_01.png"))
        .scale(0.8f)
        .intoMenuItem([this] { onCreate(); })
        .pos(centerX(), 30.f)
        .id("create-btn")
        .parent(m_buttonMenu)
        .collect();

    return true;
}

void CreateClanPopup::onCreate() {
    std::string name = m_nameInput ? m_nameInput->getString() : "";
    if (name.size() < 3) {
        Notification::create("Clan name must be at least 3 characters", NotificationIcon::Error)->show();
        return;
    }

    std::string desc = m_descInput ? m_descInput->getString() : "";

    int statReq = 0;
    if (m_statReqInput) {
        statReq = numFromString<int>(m_statReqInput->getString()).unwrapOrDefault();
    }

    LGClanType type = m_typeRadio.getSelected();
    LGClanJoinType joinType = m_joinRadio.getSelected();

    auto uPopup = UploadActionPopup::create(nullptr, "Creating clan...");
    uPopup->show();

    auto uPopupRef = Ref(uPopup);
    auto onCreated = m_onCreated;

    m_listener.spawn(
        APIClient::getInstance().createClan(name, desc, type, 0, joinType, statReq),
        [uPopupRef, onCreated](web::WebResponse res) {
            if (!uPopupRef) return;
            auto parsed = APIClient::getInstance().createClanParse(res);

            if (!parsed.ok) {
                uPopupRef->showFailMessage(parsed.error.empty() ? "Failed! Try again later." : parsed.error);
                return;
            }

            uPopupRef->showSuccessMessage("Clan created!");
            if (onCreated) onCreated();
        }
    );
}

}
