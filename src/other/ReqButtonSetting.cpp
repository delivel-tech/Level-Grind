#include "../other/ReqButtonSetting.hpp"

#include "../other/ReqAction.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

namespace {
    class ReqButtonSettingV3 : public SettingV3 {
    public:
        static Result<std::shared_ptr<SettingV3>> parse(
            std::string const& key,
            std::string const& modID,
            matjson::Value const& json
        ) {
            auto res = std::make_shared<ReqButtonSettingV3>();
            auto root = checkJson(json, "ReqButtonSettingV3");

            res->init(key, modID, root);
            res->parseNameAndDescription(root);
            res->parseEnableIf(root);

            root.checkUnknownKeys();
            return root.ok(std::static_pointer_cast<SettingV3>(res));
        }

        bool load(matjson::Value const& json) override {
            return true;
        }

        bool save(matjson::Value& json) const override {
            return true;
        }

        bool isDefaultValue() const override {
            return true;
        }

        void reset() override {}

        SettingNodeV3* createNode(float width) override;
    };

    class ReqButtonSettingNodeV3 : public SettingNodeV3 {
    protected:
        ButtonSprite* m_buttonSprite;
        CCMenuItemSpriteExtra* m_button;

        bool init(std::shared_ptr<ReqButtonSettingV3> setting, float width) {
            if (!SettingNodeV3::init(setting, width))
                return false;

            m_buttonSprite = ButtonSprite::create("Req", "goldFont.fnt", "GJ_button_04.png", 0.8f);
            m_buttonSprite->setScale(0.55f);

            m_button = CCMenuItemSpriteExtra::create(
                m_buttonSprite,
                this,
                menu_selector(ReqButtonSettingNodeV3::onButton)
            );

            this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center, { 14.f, 0.f });
            this->getButtonMenu()->setContentWidth(60);
            this->getButtonMenu()->updateLayout();

            this->updateState(nullptr);
            return true;
        }

        void updateState(CCNode* invoker) override {
            SettingNodeV3::updateState(invoker);

            auto shouldEnable = this->getSetting()->shouldEnable();
            m_button->setEnabled(shouldEnable);
            m_buttonSprite->setCascadeColorEnabled(true);
            m_buttonSprite->setCascadeOpacityEnabled(true);
            m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
            m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
        }

        void onButton(CCObject*) {
            lg::runReqCheck();
        }

        void onCommit() override {}

        void onResetToDefault() override {}

    public:
        static ReqButtonSettingNodeV3* create(std::shared_ptr<ReqButtonSettingV3> setting, float width) {
            auto ret = new ReqButtonSettingNodeV3();
            if (ret->init(setting, width)) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }

        bool hasUncommittedChanges() const override {
            return false;
        }

        bool hasNonDefaultValue() const override {
            return false;
        }

        std::shared_ptr<ReqButtonSettingV3> getSetting() const {
            return std::static_pointer_cast<ReqButtonSettingV3>(SettingNodeV3::getSetting());
        }
    };

    SettingNodeV3* ReqButtonSettingV3::createNode(float width) {
        return ReqButtonSettingNodeV3::create(
            std::static_pointer_cast<ReqButtonSettingV3>(shared_from_this()),
            width
        );
    }
}

void registerReqButtonSettingType() {
    (void)Mod::get()->registerCustomSettingType("req-button", &ReqButtonSettingV3::parse);
}