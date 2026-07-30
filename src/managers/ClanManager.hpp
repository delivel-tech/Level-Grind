#pragma once
#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"

using namespace geode::prelude;

namespace levelgrind {

// Pure local state cache for the clan feature - no networking here, that's
// APIClient's job. UI code fetches via APIClient, parses the response, and
// pushes it in here (same split as DataManager::setSharedData), so any
// screen can read the current clan/messages/invites without refetching.
class ClanManager : public Singleton<ClanManager> {
    friend class Singleton<ClanManager>;

private:
    ClanManager() = default;

    int m_myClanID = 0;
    LGClanRole m_myRole = LGClanRole::Member;
    LGFullClanData m_clan{};
    bool m_canManage = false;

    std::vector<ClanJoinRequestInfo> m_joinRequests; // pending join requests to *this* clan (officer/leader view)
    std::vector<ClanInviteInfo> m_invites;           // pending invites sent out by *this* clan (officer/leader view)
    std::vector<ClanMessageInfo> m_messages;         // last known chat page (<=50, oldest -> newest)
    std::vector<MyClanInviteInfo> m_myInvites;       // invites addressed to *me*, across all clans

    void recomputeMyRole();

public:
    bool isInClan() const;

    void setMyClanID(int clanID);
    int getMyClanID() const;

    // Feed the result of APIClient::getClanParse here. Recomputes myRole
    // from the member list and clears canManage/joinRequests/invites if the
    // caller isn't an officer/leader (mirrors what the server omits).
    void setClanData(GetClanResponse const& data);
    LGFullClanData const& getClan() const;
    LGClanRole getMyRole() const;
    bool canManage() const;
    std::vector<ClanJoinRequestInfo> const& getJoinRequests() const;
    std::vector<ClanInviteInfo> const& getInvites() const;

    void setMessages(std::vector<ClanMessageInfo> messages);
    std::vector<ClanMessageInfo> const& getMessages() const;

    void setMyInvites(std::vector<MyClanInviteInfo> invites);
    std::vector<MyClanInviteInfo> const& getMyInvites() const;

    // Call after leave_clan/disband_clan succeeds, or when view_clans comes
    // back with myClanID == 0, to drop all cached clan-scoped state.
    void clear();
};

}
