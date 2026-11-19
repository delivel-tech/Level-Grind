#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cue/ListNode.hpp>
#include "../BasePopup.hpp"
#include "../../managers/GrindClansManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

using ClanChanged = geode::Function<void()>;

class ClanMessagePopup : public BasePopup {
public:
    static ClanMessagePopup* create(ClanChatMessage const& msg, bool canDelete, bool canPin, bool isPinned, geode::Function<void()> onChanged);
private:
    bool init(ClanChatMessage const& msg, bool canDelete, bool canPin, bool isPinned, geode::Function<void()> onChanged);
    ClanChatMessage m_msg;
    geode::Function<void()> m_onDeleted;
};

class ShareLevelPopup : public BasePopup {
public:
    static ShareLevelPopup* create(geode::Function<void(int, std::string const&)> onPicked);
private:
    bool init(geode::Function<void(int, std::string const&)> onPicked);
    void onShare();
    void update(float dt) override;
    void finish(GJGameLevel* level);
    geode::Function<void(int, std::string const&)> m_onPicked;
    TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    std::string m_pendingKey;
    int m_levelId = 0;
    float m_waited = 0.f;
};

class ClanLogoPopup : public BasePopup {
public:
    static ClanLogoPopup* create(ClanInfo const& clan, bool pending, ClanChanged onChanged);
private:
    bool init(ClanInfo const& clan, bool pending, ClanChanged onChanged);
    void onPick();
    void onUpload();
    void setStatus(std::string const& text, bool error);
    ClanInfo m_clan;
    ClanChanged m_onChanged;
    std::vector<uint8_t> m_bytes;
    CCNode* m_preview = nullptr;
    CCLabelBMFont* m_status = nullptr;
    CCMenuItemSpriteExtra* m_uploadBtn = nullptr;
    TaskHolder<file::PickResult> m_pickTask;
};

class StaffLogosPopup : public BasePopup {
public:
    static StaffLogosPopup* create();
    void load();
private:
    bool init() override;
    cue::ListNode* m_list = nullptr;
    CCLabelBMFont* m_footer = nullptr;
    TaskHolder<web::WebResponse> m_task;
};

class EarnGPPopup : public BasePopup {
public:
    static EarnGPPopup* create(GPWallet const& wallet);
private:
    bool init(GPWallet const& wallet);
};

class CreateClanPopup : public BasePopup {
public:
    static CreateClanPopup* create(int cost, GPWallet const& wallet, ClanChanged onCreated);
private:
    bool init(int cost, GPWallet const& wallet, ClanChanged onCreated);
    void refreshLook();
    void onCreate();

    int m_cost = 300;
    ClanChanged m_onCreated;
    TextInput* m_name = nullptr;
    TextInput* m_desc = nullptr;
    int m_color = 0;
    int m_emblem = 1;
    int m_joinType = 0;
    int m_minGp = 0;
    CCNode* m_preview = nullptr;
    CCNode* m_previewHolder = nullptr;
    std::vector<CCSprite*> m_swatches;
    CCLabelBMFont* m_joinLabel = nullptr;
    CCLabelBMFont* m_minLabel = nullptr;
};

class ClanSettingsPopup : public BasePopup {
public:
    static ClanSettingsPopup* create(ClanInfo const& clan, ClanChanged onChanged);
private:
    bool init(ClanInfo const& clan, ClanChanged onChanged);
    void refreshLook();
    void onSave();

    ClanInfo m_clan;
    ClanChanged m_onChanged;
    TextInput* m_desc = nullptr;
    int m_color = 0;
    int m_emblem = 1;
    int m_joinType = 0;
    int m_minGp = 0;
    CCNode* m_previewHolder = nullptr;
    std::vector<CCSprite*> m_swatches;
    CCLabelBMFont* m_joinLabel = nullptr;
    CCLabelBMFont* m_minLabel = nullptr;
    CCLabelBMFont* m_costLabel = nullptr;
};

class ClanViewPopup : public BasePopup {
public:
    static ClanViewPopup* create(int clanId, geode::Function<void(ClanInfo const&)> onJoin);
private:
    bool init(int clanId, geode::Function<void(ClanInfo const&)> onJoin);
    void build(ClanDetails const& d);
    geode::Function<void(ClanInfo const&)> m_onJoin;
    TaskHolder<web::WebResponse> m_task;
    ~ClanViewPopup() { m_task.cancel(); }
};

class ClanMembersPopup : public BasePopup {
public:
    static ClanMembersPopup* create(ClanDetails const& d, ClanChanged onChanged);
private:
    bool init(ClanDetails const& d, ClanChanged onChanged);
    void memberActions(ClanMemberInfo const& m);
    ClanDetails m_details;
    ClanChanged m_onChanged;
};

class ClanTasksPopup : public BasePopup {
public:
    static ClanTasksPopup* create(ClanDetails const& d, long long weekEndsAt, long long now);
private:
    bool init(ClanDetails const& d, long long weekEndsAt, long long now);
    void buildCards();
    ClanDetails m_details;
    int m_tier = 0;
    CCNode* m_cards = nullptr;
    CCLabelBMFont* m_tierLabel = nullptr;
};

class ClanLeaderboardPopup : public BasePopup {
public:
    static ClanLeaderboardPopup* create(int myClanId);
private:
    bool init(int myClanId);
    void load();
    int m_myClanId = 0;
    std::string m_sort = "week";
    int m_page = 0;
    cue::ListNode* m_list = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    std::vector<CCSprite*> m_tabSprites;
    bool m_hasMore = false;
    TaskHolder<web::WebResponse> m_task;
    ~ClanLeaderboardPopup() { m_task.cancel(); }
};

class ClanFeedPopup : public BasePopup {
public:
    static ClanFeedPopup* create(ClanDetails const& d, long long now);
private:
    bool init(ClanDetails const& d, long long now);
};

class ClanInvitesPopup : public BasePopup {
public:
    static ClanInvitesPopup* create(std::vector<ClanInviteInfo> const& invites, ClanChanged onChanged);
private:
    bool init(std::vector<ClanInviteInfo> const& invites, ClanChanged onChanged);
    ClanChanged m_onChanged;
};

class ClanRequestsPopup : public BasePopup {
public:
    static ClanRequestsPopup* create(ClanDetails const& d, ClanChanged onChanged);
private:
    bool init(ClanDetails const& d, ClanChanged onChanged);
    ClanChanged m_onChanged;
};

}
