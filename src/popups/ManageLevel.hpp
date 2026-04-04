#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include "Geode/ui/NineSlice.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <Geode/binding/GJGameLevel.hpp>

using namespace geode::prelude;

class ManageLevel : public Popup, public ::UploadPopupDelegate {
public:
    static ManageLevel* create(GJGameLevel* level, GJDifficultySprite* diffSprite);

    bool m_star = false;
    bool m_moon = false;
    bool m_coin = false;
    bool m_demon = false;
    int m_demonDifficulty = 0;

    int m_levelID = 0;
    std::string m_levelName = "";
    int m_levelDifficulty = 0;
    int m_levelLength = 0;

    std::vector<NineSlice*> m_containers;
    NineSlice* m_levelInfoContainer = nullptr;
    NineSlice* m_levelInfoLGContainer = nullptr;
    NineSlice* m_helperActionsContainer = nullptr;
    NineSlice* m_adminActionsContainer = nullptr;
    NineSlice* m_notesContainer = nullptr;

    std::vector<CCMenu*> m_menus;
    CCMenu* m_levelInfoMenu = nullptr;
    CCMenu* m_levelInfoLGMenu = nullptr;
    CCMenu* m_helperActionsMenu = nullptr;
    CCMenu* m_adminActionsMenu = nullptr;
    CCMenu* m_notesMenu = nullptr;

    CCSprite* m_starSpr = nullptr;
    CCSprite* m_moonSpr = nullptr;
    CCSprite* m_demonSpr = nullptr;
    CCSprite* m_coinSpr = nullptr;

    LoadingSpinner* m_starLoading = nullptr;
    LoadingSpinner* m_moonLoading = nullptr;
    LoadingSpinner* m_demonLoading = nullptr;
    LoadingSpinner* m_coinLoading = nullptr;

    CCMenu* m_statusRowMenu = nullptr;

    std::vector<std::string> m_containerIDs = {
        "level_info_container",
        "level_info_lg_container",
        "helper_actions_container",
        "admin_actions_container",
        "notes_container"
    };

    std::vector<std::string> m_menuIDs = {
        "level_info_menu",
        "level_info_lg_menu",
        "helper_actions_menu",
        "admin_actions_menu",
        "notes_menu"
    };

    int m_i = 0;

    CCSprite* m_addBtnSpr = nullptr;

    CCMenuItemSpriteExtra* m_addBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteBtn = nullptr;
    CCMenuItemSpriteExtra* m_reAddBtn = nullptr;

    CCMenuItemSpriteExtra* m_lockBtn = nullptr;
    CCMenuItemSpriteExtra* m_unlockBtn = nullptr;

    CCMenuItemSpriteExtra* m_addNoteBtn = nullptr;

    void onClosePopup(::UploadActionPopup* popup) override;

protected:
    bool init(GJGameLevel* level, GJDifficultySprite* diffSprite);

    void onAddButton(CCObject* sender);
    void onDeleteBtn(CCObject* sender);
    void onLockBtn(CCObject* sender);
    void onUnlockBtn(CCObject* sender);

    void onCoinSwitcher(CCObject* sender);
    void onDemonSwitcher(CCObject* sender);
    void onMoonSwitcher(CCObject* sender);
    void onStarSwitcher(CCObject* sender);

    void onLockedBtn(CCObject* sender);
    void onNotReadyBtn(CCObject* sender);

    void onAddNoteButton(CCObject* sender);
    void onDeleteNoteButton(CCObject* sender);

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_listener2;

    bool m_isDaily = false;
    bool m_isDailyPlat = false;
    bool m_isWeekly = false;
    bool m_isWeeklyPlat = false;

    CCMenuItemSpriteExtra* m_setDailyBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteDailyBtn = nullptr;
    CCMenuItemSpriteExtra* m_setWeeklyBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteWeeklyBtn = nullptr;

    geode::async::TaskHolder<geode::utils::web::WebResponse> m_eventListener;

    void onSetEventBtn(CCObject* sender);
    void onDeleteEventBtn(CCObject* sender);

    ~ManageLevel() {
        m_listener.cancel();
        m_listener2.cancel();
        m_eventListener.cancel();
    }
};