#include "SuggestionsFilterPopup.hpp"
#include "../../managers/DataManager.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/general.hpp"
#include "cue/RadioLogic.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <UIBuilder.hpp>

namespace levelgrind {

SuggestionsFilterPopup* SuggestionsFilterPopup::create(SuggestionsLayer *owner) {
    auto ret = new SuggestionsFilterPopup;
    if (ret && ret->init(owner)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SuggestionsFilterPopup::init(SuggestionsLayer* owner) {
    if (!BasePopup::init(240.f, 150.f)) return false;

    m_owner = owner;
    GrindPosition pos = DataManager::getInstance().getUserPosition();

    if (pos == GrindPosition::User) setTitle("Unknown Role: Filter");
    else if (pos == GrindPosition::Helper) setTitle("Grind Helper: Filter");
    else if (pos == GrindPosition::Admin) setTitle("Grind Admin: Filter");
    else if (pos == GrindPosition::Owner) setTitle("Grind Owner: Filter");

    enum class States {
        More,
        Equals,
        Less
    };

    cue::RadioLogic<States> radio;
    
    // yeah another wall of codeshit sorry for anyone who reads this :)))
    auto togglerMoreSprOn = ButtonSprite::create(">", "bigFont.fnt", "GJ_button_02.png");
    auto togglerMoreSprOff = ButtonSprite::create(">", "bigFont.fnt", "GJ_button_04.png");

    auto togglerEqualsSprOn = ButtonSprite::create("=", "bigFont.fnt", "GJ_button_02.png");
    auto togglerEqualsSprOff = ButtonSprite::create("=", "bigFont.fnt", "GJ_button_04.png");

    auto togglerLessSprOn = ButtonSprite::create("<", "bigFont.fnt", "GJ_button_02.png");
    auto togglerLessSprOff = ButtonSprite::create("<", "bigFont.fnt", "GJ_button_04.png");

    auto moreToggler = radio.createToggler(
        States::More,
        togglerMoreSprOn, togglerMoreSprOff
    );

    auto equalsToggler = radio.createToggler(
        States::Equals,
        togglerEqualsSprOn, togglerEqualsSprOff
    );
    
    auto lessToggler = radio.createToggler(
        States::Less,
        togglerLessSprOn, togglerLessSprOff
    );

    auto rowMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setGap(10))
        .updateLayout()
        .parent(m_buttonMenu)
        .pos({ m_buttonMenu->getContentWidth() / 2, m_buttonMenu->getContentSize().height / 2 + 5.f })
        .width(m_buttonMenu->getContentWidth() - 20.f)
        .scale(0.8f)
        .collect();

    auto togglersMenu = Build<CCMenu>::create()
        .layout(RowLayout::create()->setAutoGrowAxis(true))
        .children(moreToggler, equalsToggler, lessToggler)
        .updateLayout()
        .parent(rowMenu)
        .pos({ m_buttonMenu->getContentWidth() / 2, m_title->getPositionY() - 25.f })
        .scale(0.7f)
        .collect();

    auto textInput = TextInput::create(50.f, "0", "bigFont.fnt");
    textInput->setCommonFilter(CommonFilter::Int);
    rowMenu->addChild(textInput);
    textInput->setPosition({ m_buttonMenu->getContentWidth() / 2, m_buttonMenu->getContentSize().height / 2 - 5.f });
    textInput->setScale(0.8f);

    rowMenu->updateLayout();

    if (m_owner) {
        switch (m_owner->getPointsFilterOp()) {
            case SuggestionsLayer::PointsFilterOp::More:
                radio.select(States::More);
                break;
            case SuggestionsLayer::PointsFilterOp::Equals:
                radio.select(States::Equals);
                break;
            case SuggestionsLayer::PointsFilterOp::Less:
                radio.select(States::Less);
                break;
        }
        textInput->setString(numToString(m_owner->getPointsFilterValue()).c_str());
    } else {
        radio.select(States::More);
    }

    auto okBtn = Build(ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_01.png"))
        .intoMenuItem([this, radio, textInput] {
            if (!m_owner) return;
            States selected = radio.getSelected();
            if (!numFromString<int>(textInput->getString())) {
                Notification::create("Please input valid number!", NotificationIcon::Error)->show();
                return;
            }

            auto val = numFromString<int>(textInput->getString()).unwrap();
            switch (selected) {
                case States::More:
                    m_owner->setPointsFilter(SuggestionsLayer::PointsFilterOp::More, val);
                    break;
                case States::Equals:
                    m_owner->setPointsFilter(SuggestionsLayer::PointsFilterOp::Equals, val);
                    break;
                case States::Less:
                    m_owner->setPointsFilter(SuggestionsLayer::PointsFilterOp::Less, val);
                    break;
            }
            m_owner->applyPointsFilter();
            this->onClose(nullptr);
        })
        .parent(m_buttonMenu)
        .pos({
            m_buttonMenu->getContentWidth() / 2,
            28.f
        })
        .scale(0.8f)
        .collect();

    return true;
}

}