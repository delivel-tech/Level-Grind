#include "SyncLevelsPopup.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/MDPopup.hpp"
#include "Geode/ui/NineSlice.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/ui/TextInput.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/UploadActionPopup.hpp>
#include <UIBuilder.hpp>
#include <algorithm>

#include "../../managers/APIClient.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

namespace levelgrind {

namespace {

constexpr int kMinThreshold = -99;
constexpr int kMaxThreshold = 99;

struct ThresholdDef {
    const char* key;
    const char* label;
    int defaultValue;
    ccColor3B color;
};

constexpr ThresholdDef kThresholds[] = {
    { "sync-add",         "Add",     5, { 120, 255, 120 } },
    { "sync-delete",      "Delete", -3, { 255, 110, 110 } },
    { "sync-lock",        "Lock",   -7, { 255, 180,  80 } },
    { "sync-coin-add",    "Add",     3, { 120, 255, 120 } },
    { "sync-coin-delete", "Delete", -3, { 255, 110, 110 } },
};

int savedThreshold(const ThresholdDef& def) {
    if (!Mod::get()->hasSavedValue(def.key)) return def.defaultValue;
    return std::clamp(Mod::get()->getSavedValue<int>(def.key), kMinThreshold, kMaxThreshold);
}

CCNode* makePanel(CCNode* parent, CCPoint pos, CCSize size, const char* title, const char* iconFrame) {
    auto panel = Build<CCNode>::create()
        .contentSize(size)
        .anchorPoint({ 0.5f, 0.5f })
        .ignoreAnchorPointForPos(false)
        .pos(pos)
        .parent(parent)
        .collect();

    Build(NineSlice::create("square02_small.png"))
        .contentSize(size)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .opacity(75)
        .parent(panel);

    float titleX = 10.f;
    if (auto icon = CCSprite::createWithSpriteFrameName(iconFrame)) {
        icon->setScale(14.f / icon->getContentHeight());
        icon->setPosition({ 17.f, size.height - 14.f });
        panel->addChild(icon);
        titleX = 29.f;
    }

    Build(CCLabelBMFont::create(title, "goldFont.fnt"))
        .scale(0.55f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ titleX, size.height - 13.f })
        .parent(panel);

    Build(CCLayerColor::create({ 0, 0, 0, 60 }, size.width - 16.f, 1.f))
        .pos({ 8.f, size.height - 26.f })
        .parent(panel);

    return panel;
}

}

SyncLevelsPopup* SyncLevelsPopup::create() {
    auto ret = new SyncLevelsPopup;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

TextInput* SyncLevelsPopup::addThresholdRow(CCNode* panel, float y, Threshold which) {
    auto const& def = kThresholds[which];
    float const width = panel->getContentWidth();

    Build(CCLabelBMFont::create(def.label, "bigFont.fnt"))
        .scale(0.45f)
        .anchorPoint({ 0.f, 0.5f })
        .pos({ 12.f, y })
        .color(def.color)
        .parent(panel);

    float const stepperX = width - 42.f;

    auto input = Build(TextInput::create(46.f, "0", "bigFont.fnt"))
        .scale(0.65f)
        .pos({ stepperX, y })
        .parent(panel)
        .collect();
    input->setMaxCharCount(3);
    input->setFilter("-0123456789");
    input->setString(numToString(savedThreshold(def)));

    auto menu = Build<CCMenu>::create()
        .contentSize(panel->getContentSize())
        .ignoreAnchorPointForPos(false)
        .anchorPoint({ 0.f, 0.f })
        .pos({ 0.f, 0.f })
        .parent(panel)
        .collect();

    auto step = [input](int delta) {
        int value = numFromString<int>(input->getString()).unwrapOr(0);
        input->setString(numToString(std::clamp(value + delta, kMinThreshold, kMaxThreshold)));
    };

    Build<CCSprite>::createSpriteName("edit_leftBtn_001.png")
        .scale(0.7f)
        .intoMenuItem([step] { step(-1); })
        .pos({ stepperX - 27.f, y })
        .parent(menu);

    Build<CCSprite>::createSpriteName("edit_rightBtn_001.png")
        .scale(0.7f)
        .intoMenuItem([step] { step(1); })
        .pos({ stepperX + 27.f, y })
        .parent(menu);

    return input;
}

bool SyncLevelsPopup::init() {
    if (!BasePopup::init(380.f, 240.f)) return false;

    setTitle("Grind Coordinator: Sync Levels");

    Build(CCLabelBMFont::create("Points a level needs before each change is applied.", "chatFont.fnt"))
        .scale(0.55f)
        .pos(fromTop(40.f))
        .opacity(170)
        .parent(m_mainLayer);

    CCSize const panelSize { 168.f, 132.f };
    float const panelY = centerY() + 2.f;

    auto levelsPanel = makePanel(m_mainLayer, { centerX() - 88.f, panelY }, panelSize, "Levels", "GJ_starsIcon_001.png");
    auto coinsPanel = makePanel(m_mainLayer, { centerX() + 88.f, panelY }, panelSize, "Coins", "GJ_coinsIcon2_001.png");

    float const firstRowY = panelSize.height - 45.f;
    float const rowGap = 30.f;

    m_inputs[Add] = addThresholdRow(levelsPanel, firstRowY, Add);
    m_inputs[Delete] = addThresholdRow(levelsPanel, firstRowY - rowGap, Delete);
    m_inputs[Lock] = addThresholdRow(levelsPanel, firstRowY - rowGap * 2, Lock);
    m_inputs[CoinAdd] = addThresholdRow(coinsPanel, firstRowY, CoinAdd);
    m_inputs[CoinDelete] = addThresholdRow(coinsPanel, firstRowY - rowGap, CoinDelete);

    Build(ButtonSprite::create("Sync", "goldFont.fnt", "GJ_button_01.png", 0.9f))
        .scale(0.85f)
        .intoMenuItem([this] { this->onSync(); })
        .pos({ m_buttonMenu->getContentWidth() / 2.f, 26.f })
        .id("sync-btn")
        .parent(m_buttonMenu);

    Build(ButtonSprite::create("Defaults", "goldFont.fnt", "GJ_button_04.png", 0.8f))
        .scale(0.55f)
        .intoMenuItem([this] {
            for (int i = 0; i < ThresholdCount; i++) {
                m_inputs[i]->setString(numToString(kThresholds[i].defaultValue));
            }
        })
        .pos({ m_buttonMenu->getContentWidth() - 48.f, 24.f })
        .id("defaults-btn")
        .parent(m_buttonMenu);

    return true;
}

void SyncLevelsPopup::onSync() {
    if (m_syncing) return;

    int values[ThresholdCount];
    for (int i = 0; i < ThresholdCount; i++) {
        auto parsed = numFromString<int>(m_inputs[i]->getString());
        if (!parsed || parsed.unwrap() < kMinThreshold || parsed.unwrap() > kMaxThreshold) {
            Notification::create(
                fmt::format("Enter a number from {} to {} in every field", kMinThreshold, kMaxThreshold),
                NotificationIcon::Error
            )->show();
            return;
        }
        values[i] = parsed.unwrap();
    }

    Ref<SyncLevelsPopup> self = this;
    std::vector<int> v(values, values + ThresholdCount);

    createQuickPopup(
        "Sync Levels",
        fmt::format(
            "Levels: add <cg>{}</c>, delete <cr>{}</c>, lock <co>{}</c>\n"
            "Coins: add <cg>{}</c>, delete <cr>{}</c>\n\n"
            "This updates the level list for <cy>everyone</c>.",
            v[Add], v[Delete], v[Lock], v[CoinAdd], v[CoinDelete]
        ),
        "Cancel", "Sync",
        [self, v](auto, bool confirmed) {
            if (confirmed && self) self->runSync(v);
        }
    );
}

void SyncLevelsPopup::runSync(std::vector<int> v) {
    m_syncing = true;

    auto uPopup = UploadActionPopup::create(nullptr, "Syncing levels...");
    uPopup->show();
    Ref<UploadActionPopup> uPopupRef = uPopup;
    Ref<SyncLevelsPopup> self = this;

    m_listener.spawn(
        APIClient::getInstance().syncLevels(v[Add], v[Delete], v[CoinAdd], v[CoinDelete], v[Lock]),
        [self, uPopupRef, v](web::WebResponse res) {
            if (!self) return;
            self->m_syncing = false;

            auto parsed = APIClient::getInstance().syncLevelsParse(res);
            if (!parsed.ok) {
                if (uPopupRef) uPopupRef->showFailMessage("Failed! Try again later.");
                return;
            }

            for (int i = 0; i < ThresholdCount; i++) {
                Mod::get()->setSavedValue<int>(kThresholds[i].key, v[i]);
            }

            if (uPopupRef) uPopupRef->closePopup();
            self->onClose(nullptr);

            MDPopup::create(
                "Sync Results",
                fmt::format(
                    "# <cp>Level Changes</c>\n\n"
                    "<cg>Inserted:</c> {}\n\n"
                    "<cr>Deleted:</c> {}\n\n"
                    "<co>Locked:</c> {}\n\n"
                    "<cj>Coins changed:</c> {}\n\n",
                    parsed.inserted, parsed.deleted, parsed.lockUpdates, parsed.coinUpdates
                ),
                "OK"
            )->show();
        }
    );
}

}