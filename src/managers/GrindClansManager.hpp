#pragma once
#include "Auth.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <set>
#include <string>
#include <vector>

#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

struct GPWallet {
    long long balance = 0;
    long long lifetime = 0;
    int todayEarned = 0;
    int dailyCap = 400;
};

struct ClanInfo {
    int id = 0;
    std::string name;
    std::string description;
    int ownerId = 0;
    int colorId = 0;
    int emblemId = 1;
    int joinType = 0;
    int minGp = 0;
    long long xp = 0;
    int level = 1;
    long long levelXp = 0;
    long long nextLevelXp = 0;
    bool maxLevel = false;
    long long weekXp = 0;
    int members = 0;
    int maxMembers = 20;
    int gpBonus = 0;
    bool featured = false;
    int rank = 0;
    long long logoId = 0;
};

struct ClanMemberInfo {
    int accountId = 0;
    std::string username;
    int role = 0;
    int icon = 1, color1 = 0, color2 = 3, color3 = 0;
    long long contributed = 0;
    int weekContributed = 0;
    long long lastActive = 0;
    long long joinedAt = 0;
};

struct ClanTaskInfo {
    int slot = 0;
    int type = 0;
    int target = 1;
    int progress = 0;
    bool completed = false;
    int rewardGp = 0;
    int tier = 0;
};

struct ClanFeedEntry {
    long long id = 0;
    int kind = 0;
    int accountId = 0;
    std::string username;
    int value = 0;
    std::string text;
    long long at = 0;
};

struct ClanJoinRequestInfo {
    int accountId = 0;
    std::string username;
    int icon = 1, color1 = 0, color2 = 3, color3 = 0;
    long long lifetimeGp = 0;
    long long at = 0;
};

struct ClanChatMessage {
    long long id = 0;
    int accountId = 0;
    std::string username;
    int icon = 1, color1 = 0, color2 = 3, color3 = 0;
    std::string message;
    long long at = 0;
};

struct ClanDetails {
    ClanInfo info;
    bool logoPending = false;
    std::vector<ClanMemberInfo> members;
    std::vector<ClanTaskInfo> tasks;
    int weekRank = 0;
    std::vector<ClanFeedEntry> feed;
    std::vector<ClanJoinRequestInfo> requests;
    int myRole = 0;
    bool hasPinned = false;
    ClanChatMessage pinned;
};

struct ClanHome {
    bool ok = false;
    std::string error;
    GPWallet wallet;
    int createCost = 300;
    long long weekEndsAt = 0;
    long long now = 0;
    std::set<int> requested;
    bool hasClan = false;
    ClanDetails clan;
};

struct ClanBrowse {
    bool ok = false;
    std::string error;
    std::vector<ClanInfo> clans;
    int page = 0;
    bool hasMore = false;
    bool ranked = true;
    long long weekEndsAt = 0;
};

struct ClanPalette {
    const char* name;
    ccColor3B color;
};

class GrindClansManager : public Singleton<GrindClansManager> {
    friend class Singleton<GrindClansManager>;
    GrindClansManager() = default;

public:
    static matjson::Value playerLook();
    static matjson::Value authBody();

    static auth::Future post(std::string const& endpoint, matjson::Value body);

    static ClanInfo parseClan(matjson::Value const& v);
    static ClanDetails parseDetails(matjson::Value const& v);
    static ClanHome parseHome(web::WebResponse& res);
    static ClanBrowse parseBrowse(web::WebResponse& res);
    static std::vector<ClanChatMessage> parseChat(web::WebResponse& res, bool& ok);

    static bool parsePinned(matjson::Value const& v, ClanChatMessage& out);
    static GPWallet parseWallet(matjson::Value const& v);
    static bool parseOk(web::WebResponse& res, std::string& error);

    static std::vector<ClanPalette> const& palette();
    static ccColor3B clanColor(int colorId);
    static std::string roleName(int role);
    static ccColor3B roleColor(int role);
    static std::string taskTitle(ClanTaskInfo const& task);

    static const char* tierName(int tier);
    static ccColor3B tierColor(int tier);
    static int tierCount(std::vector<ClanTaskInfo> const& tasks);
    static const char* taskIcon(int type);
    static std::string feedText(ClanFeedEntry const& e);
    static std::string timeAgo(long long at, long long now);
    static std::string formatGP(long long gp);

    void queueLevelClaim(int levelId, int stars, bool platformer);
    void flushClaims();
    void queueCoinClaim(int levelId, std::vector<int> const& coins);
    void flushCoinClaims();

    void checkPacks(std::vector<GrindPack> const& packs);
    void checkAllPacks();

    // UploadActionPopup one
    void runAction(
        std::string const& endpoint, matjson::Value body,
        std::string const& loadingText, std::string const& successText,
        geode::Function<void(matjson::Value const&)> onSuccess
    );

    GPWallet m_lastWallet;
    bool m_hasWallet = false;

private:
    void sendNextClaim();
    void sendNextPackClaim();
    std::vector<GrindPack> m_packs;
    bool m_packsLoaded = false;
    bool m_packClaiming = false;
    async::TaskHolder<web::WebResponse> m_packTask;
    async::TaskHolder<web::WebResponse> m_packListTask;
    bool m_claiming = false;
    async::TaskHolder<web::WebResponse> m_claimTask;
    async::TaskHolder<web::WebResponse> m_actionTask;
    async::TaskHolder<web::WebResponse> m_coinTask;
    bool m_coinClaiming = false;
};

}
