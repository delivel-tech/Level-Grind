#pragma once
#include "../utils/singleton.hpp"
#include "../features/levels/LevelTypes.hpp"
#include "../features/clans/ClanTypes.hpp"
#include "../features/events/EventTypes.hpp"
#include "../features/weekly-achievements/WeeklyAchTypes.hpp"
#include "../features/announcements/AnnouncementTypes.hpp"
#include "../features/admin/AdminTypes.hpp"
#include "../features/pets/PetTypes.hpp"
#include "CoreTypes.hpp"
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <arc/future/Future.hpp>
#include <chrono>
#include <fmt/format.h>
#include <optional>
#include <string>

namespace levelgrind {

class BackendManager : public Singleton<BackendManager> {
    friend class Singleton<BackendManager>;
    BackendManager() = default;

    static constexpr const char* baseUrl = "https://api.delivel.tech";

    // Defined inline (rather than in a .cpp) so that each XxxImpl.cpp
    // instantiates request<T>/requestWithReq<T> in its own translation unit,
    // where its matjson::Serialize<T> specialization is actually visible.
    template <typename T>
    arc::Future<T> requestWithReq(geode::utils::web::WebRequest req, std::string method, std::string endpoint) {
        auto res = (method == "GET")
            ? co_await req.get(fmt::format("{}{}", baseUrl, endpoint))
            : co_await req.post(fmt::format("{}{}", baseUrl, endpoint));

        if (!res.ok()) co_return T{};

        auto jsonRes = res.json();
        if (!jsonRes) co_return T{};

        auto parsed = jsonRes.unwrap().template as<T>();
        co_return parsed.isOk() ? parsed.unwrap() : T{};
    }

    template <typename T>
    arc::Future<T> request(std::string method, std::string endpoint, std::optional<matjson::Value> body = std::nullopt, std::optional<std::chrono::seconds> timeout = std::nullopt) {
        geode::utils::web::WebRequest req;
        if (timeout) req.timeout(*timeout);
        if (body) req.bodyJSON(*body);

        co_return co_await requestWithReq<T>(std::move(req), method, endpoint);
    }

public:
    arc::Future<GetLevelsResponse> getLevels(GetLevelsBody& body);
    arc::Future<BootupGetResponse> bootupGet();
    arc::Future<GetUserRolesResponse> getUserRoles(int accountID);
    arc::Future<SetRolesResponse> setRoles(const SetRolesBody& body);
    arc::Future<SyncPetResponse> syncPet();
    arc::Future<WipePetResponse> wipePet(int accountID);
    arc::Future<BanPetResponse> banPet(int accountID, std::string reason);
    arc::Future<UnbanPetResponse> unbanPet(int accountID);
    arc::Future<AnnouncementsResponse> getAnnouncements();
    arc::Future<AddAnnouncementResponse> addAnnouncement(std::string title, std::string content);
    arc::Future<DeleteAnnouncementResponse> deleteAnnouncement(int announcementId);
    arc::Future<ReqAccessResponse> requestStaffAccess();
    arc::Future<GetLevelInfoResponse> getLevelInfo(int levelID);
    arc::Future<ChangePointResponse> changePoint(PointType type, int coinType, ManageLevelBody levelBody);
    arc::Future<SyncLevelsResponse> syncLevels(int addThreshold, int deleteThreshold, int coinAddThreshold, int coinDeleteThreshold);
    arc::Future<NewLevelResponse> newlevel(ManageLevelBody body);
    arc::Future<GetGrindPacksResponse> getGrindPacks();
    arc::Future<SetEventsResponse> setEvents(int mode, int classicID, int platID);
    arc::Future<Events> getEvents(int mode);
    arc::Future<ViewClansResponse> viewClans(bool shouldUpdateStars, int starsDelta, bool shouldUpdateMoons, int moonsDelta, int page = 1, int limit = 50);
    arc::Future<GetClanResponse> getClan(int clanID);
    arc::Future<ClanDataResponse> createClan(std::string name, std::string description, LGClanType type, int colorID, LGClanJoinType joinType, int statRequirement);
    arc::Future<ClanDataResponse> editClan(int clanID, std::string name, std::string description, int colorID, LGClanJoinType joinType, int statRequirement);
    arc::Future<ClanActionResponse> joinClan(int clanID);
    arc::Future<ClanActionResponse> requestJoinClan(int clanID);
    arc::Future<ClanActionResponse> respondJoinRequest(int clanID, int targetAccountID, bool accept);
    arc::Future<ClanActionResponse> inviteToClan(int clanID, int targetAccountID, std::string targetUsername);
    arc::Future<GetMyClanInvitesResponse> getMyClanInvites();
    arc::Future<ClanActionResponse> respondClanInvite(int clanID, bool accept);
    arc::Future<ClanActionResponse> setClanRating(int clanID, LGClanRating rating);
    arc::Future<ClanActionResponse> leaveClan();
    arc::Future<ClanActionResponse> transferLeadership(int clanID, int targetAccountID);
    arc::Future<ClanActionResponse> kickClanMember(int clanID, int targetAccountID);
    arc::Future<ClanActionResponse> setClanMemberRole(int clanID, int targetAccountID, LGClanRole role);
    arc::Future<ClanActionResponse> disbandClan(int clanID);
    arc::Future<ClanActionResponse> sendClanMessage(int clanID, std::string message);
    arc::Future<GetClanMessagesResponse> getClanMessages(int clanID);
    arc::Future<ClaimClanGoalResponse> claimClanGoal(int clanID, int goalType);
    arc::Future<HealthResponse> health();
    arc::Future<NewGrindPackResponse> newGrindPack(NewGrindPackBody body);
    arc::Future<NewWeeklyAchResponse> newWeeklyAch(AchievementCellInfo info1, AchievementCellInfo info2, AchievementCellInfo info3);
    arc::Future<WeeklyAchResponse> getWeeklyAch();
    arc::Future<DeleteLevelResponse> deleteLevel(int levelId);
    arc::Future<LockLevelResponse> lockLevel(int levelId, std::string levelName);
    arc::Future<UnlockLevelResponse> unlockLevel(int levelId, std::string levelName);
    arc::Future<DeleteNotesResponse> deleteNotes(int levelId, std::string levelName);
    arc::Future<Indicators> getIndicators(int levelID);
    arc::Future<DeleteGrindPackResponse> deleteGrindPack(int ID);
    arc::Future<CancelVoteResponse> cancelVote(int levelID);

    // argon
    void performGetToken();
};

}
