#include "ClanManager.hpp"
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>

namespace levelgrind {

void ClanManager::recomputeMyRole() {
    int myAccountId = GJAccountManager::sharedState()->m_accountID;
    m_myRole = LGClanRole::Member;

    for (auto const& member : m_clan.m_members) {
        if (member.m_accountID == myAccountId) {
            m_myRole = member.m_role;
            break;
        }
    }
}

bool ClanManager::isInClan() const {
    return m_myClanID != 0;
}

void ClanManager::setMyClanID(int clanID) {
    m_myClanID = clanID;
}

int ClanManager::getMyClanID() const {
    return m_myClanID;
}

int ClanManager::getStat(LGClanData data) {
    if (data.m_clanType == LGClanType::Star) return GameStatsManager::get()->getStat("6");
    else return GameStatsManager::get()->getStat("28");
}

void ClanManager::setClanData(GetClanResponse const& data) {
    if (!data.ok) return;

    m_clan = data.clan;
    m_canManage = data.canManage;
    m_joinRequests = data.canManage ? data.joinRequests : std::vector<ClanJoinRequestInfo>{};
    m_invites = data.canManage ? data.invites : std::vector<ClanInviteInfo>{};
    m_myClanID = m_clan.m_data.m_clanID;

    recomputeMyRole();
}

LGFullClanData const& ClanManager::getClan() const {
    return m_clan;
}

LGClanRole ClanManager::getMyRole() const {
    return m_myRole;
}

bool ClanManager::canManage() const {
    return m_canManage;
}

std::vector<ClanJoinRequestInfo> const& ClanManager::getJoinRequests() const {
    return m_joinRequests;
}

std::vector<ClanInviteInfo> const& ClanManager::getInvites() const {
    return m_invites;
}

void ClanManager::setMessages(std::vector<ClanMessageInfo> messages) {
    m_messages = std::move(messages);
}

std::vector<ClanMessageInfo> const& ClanManager::getMessages() const {
    return m_messages;
}

void ClanManager::setMyInvites(std::vector<MyClanInviteInfo> invites) {
    m_myInvites = std::move(invites);
}

std::vector<MyClanInviteInfo> const& ClanManager::getMyInvites() const {
    return m_myInvites;
}

void ClanManager::clear() {
    m_myClanID = 0;
    m_myRole = LGClanRole::Member;
    m_clan = LGFullClanData{};
    m_canManage = false;
    m_joinRequests.clear();
    m_invites.clear();
    m_messages.clear();
}

}
