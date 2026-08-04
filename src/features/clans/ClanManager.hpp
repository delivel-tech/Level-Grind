#pragma once
#include "../../utils/singleton.hpp"
#include "ClanTypes.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ClanManager : public Singleton<ClanManager> {
    friend class Singleton<ClanManager>;

private:
    ClanManager() = default;

    int m_myClanID = 0;
    LGClanRole m_myRole = LGClanRole::Member;
    LGFullClanData m_clan{};
    bool m_canManage = false;

    std::vector<ClanJoinRequestInfo> m_joinRequests;
    std::vector<ClanInviteInfo> m_invites;
    std::vector<ClanMessageInfo> m_messages;
    std::vector<MyClanInviteInfo> m_myInvites;

    void recomputeMyRole();

public:
    bool isInClan() const;

    void setMyClanID(int clanID);
    int getMyClanID() const;

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

    int getStat(LGClanData data);

    void clear();
};

}
