#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/binding/EmotePickerDelegate.hpp>
#include <Geode/binding/EmotePickerPopup.hpp>
#include <cue/ListNode.hpp>
#include "../BaseLayer.hpp"
#include "../../managers/GrindClansManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ClansLayer : public BaseLayer, public EmotePickerDelegate {
public:
    static ClansLayer* create();

    void reload();

private:
    bool init() override;
    void onExit() override;
    void onBack(CCObject* sender) override;
    void playMusic(bool in);

    CCNode* makeSection(CCPoint origin, CCSize size, const char* title);
    void clearSection(CCNode* section);
    void showLoading();
    void showError(std::string const& error);
    void updateWallet(GPWallet const& w);

    void buildBrowse();
    void loadBrowse();
    void fillBrowse(ClanBrowse const& res);
    CCNode* makeClanCell(ClanInfo const& clan, float width, bool ranked);
    void onJoin(ClanInfo const& clan);

    void buildClanView();
    void fetchChat();
    void appendChat(std::vector<ClanChatMessage> const& messages);
    CCNode* makeChatCell(ClanChatMessage const& msg, float width);
    void onSend();
    void pollChat(float);

    void emotePickerClosed(EmotePickerPopup* picker) override;
    void emotePicked(EmotePickerPopup* picker, EmoteDef const* def) override;
    void emotePickerKeyDown(EmotePickerPopup* picker, enumKeyCodes key, double timestamp) override;
    void closeEmotePicker();
    bool syncEmoteBridge();
    void pollEmoteBridge(float);
    void onEnter() override;
    void textChanged();
    void openSharedLevel(int id);
    void pollSharedLevel(float dt);
    void openMention(std::string const& name);
    std::string m_pendingLevelKey;
    float m_pendingLevelWait = 0.f;
    CCMenu* m_mentionBar = nullptr;

    void showPinned(bool has, ClanChatMessage const& msg);
    CCNode* m_pinnedBar = nullptr;
    int m_taskTier = 0;
    bool m_browsingInClan = false;
    long long m_pinnedId = 0;
    EmotePickerPopup* m_emotePicker = nullptr;
    Ref<CCTextInputNode> m_emoteBridge;
    std::string m_emoteBridgeText;

    void buildTasks();
    void buildActions();

    CCNode* m_panel = nullptr;
    CCNode* m_left = nullptr;
    CCNode* m_tasks = nullptr;
    CCNode* m_actions = nullptr;
    CCNode* m_walletNode = nullptr;

    ClanHome m_home;
    bool m_loaded = false;

    std::string m_sort = "week";
    std::string m_search;
    int m_page = 0;
    cue::ListNode* m_list = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    TextInput* m_searchInput = nullptr;
    CCNode* m_listHint = nullptr;

    cue::ListNode* m_chat = nullptr;
    TextInput* m_chatInput = nullptr;
    long long m_lastMessageId = 0;
    long long m_serverNow = 0;
    bool m_chatBusy = false;
    bool m_sending = false;
    CCNode* m_chatEmpty = nullptr;

    TaskHolder<web::WebResponse> m_homeTask;
    TaskHolder<web::WebResponse> m_listTask;
    TaskHolder<web::WebResponse> m_chatTask;
    TaskHolder<web::WebResponse> m_sendTask;
};

}