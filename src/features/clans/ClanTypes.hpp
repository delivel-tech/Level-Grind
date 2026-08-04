#pragma once
#include <string>
#include <vector>

namespace levelgrind {

enum class LGClanRole {
    Member = 0,
    Officer = 1,
    Leader = 2
};

struct LGClanMember {
    int m_accountID = 0;
    std::string m_username = "";
    LGClanRole m_role = LGClanRole::Member;

    int m_stats = 0;
};

class MembersArray : public std::vector<LGClanMember> {
    public:
    const LGClanMember& getMemberAtPos(int pos) const { return this->at(pos - 1); }
};

enum class LGClanType {
    Star = 0,
    Moon = 1
};

struct LGClanStatGoal {
    bool m_statGoalCompleted = false;
    int m_statGoal = 0;
};

struct LGClanVisuals {
    int m_colorID = 0;
};

enum class LGClanRating {
    Common = 0,
    Featured = 1,
    Epic = 2
};

struct LGClanProgression {
    int m_clanLevel = 1;
    int m_clanStats = 0;
};

enum class LGClanJoinType {
    Open = 0,
    ByRequest = 1,
    Closed = 2
};

struct LGClanData {
    int m_clanID = 0;
    int m_ownerAccountID = 0;
    std::string m_clanName = "";
    std::string m_clanDescription = "";

    LGClanType m_clanType = LGClanType::Star;
    LGClanRating m_clanRating = LGClanRating::Common;

    LGClanVisuals m_clanVisuals{};

    LGClanProgression m_clanProgression{};

    int m_maxMembers = 50;

    int m_membersAmount = 0;
    LGClanJoinType m_clanJoinType = LGClanJoinType::Closed;

    int m_statRequirementForRequest = 0;

    LGClanStatGoal m_dailyStatGoal{};
    LGClanStatGoal m_weeklyStatGoal{};
    LGClanStatGoal m_monthlyStatGoal{};
};

struct LGFullClanData {
    LGClanData m_data{};
    MembersArray m_members{};
};

struct ViewClansResponse {
    bool ok = false;
    int petStars = 0;
    int petMoons = 0;
    int myClanID = 0;
    std::vector<LGClanData> clans;
};

struct ClanJoinRequestInfo {
    int m_accountID = 0;
    std::string m_username = "";
};

struct ClanInviteInfo {
    int m_accountID = 0;
    std::string m_username = "";
    int m_invitedBy = 0;
};

struct GetClanResponse {
    bool ok = false;
    LGFullClanData clan{};
    bool canManage = false;
    std::vector<ClanJoinRequestInfo> joinRequests;
    std::vector<ClanInviteInfo> invites;
};

struct ClanActionResponse {
    bool ok = false;
    std::string error = "";
    bool disbanded = false;
};

struct ClanDataResponse {
    bool ok = false;
    std::string error = "";
    LGClanData clan{};
};

struct ClanMessageInfo {
    int m_accountID = 0;
    std::string m_username = "";
    std::string m_message = "";
    std::string m_createdAt = "";
};

struct GetClanMessagesResponse {
    bool ok = false;
    std::vector<ClanMessageInfo> messages;
};

struct ClaimClanGoalResponse {
    bool ok = false;
    std::string error = "";
    int rewardStars = 0;
    int rewardMoons = 0;
};

struct MyClanInviteInfo {
    int m_clanID = 0;
    std::string m_clanName = "";
    int m_invitedBy = 0;
};

struct GetMyClanInvitesResponse {
    bool ok = false;
    std::vector<MyClanInviteInfo> invites;
};

}
