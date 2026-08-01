#pragma once
#include "Geode/cocos/cocoa/CCObject.h"
#include "ccTypes.h"
#include <Geode/Enums.hpp>
#include <Geode/binding/GJDifficultySprite.hpp>
#include <string>

namespace levelgrind {

    // Filters body for get_levels request
    struct GetLevelsBody {
        std::vector<int> difficulties;
        std::vector<int> lengths;
        std::vector<int> demonDifficulties;
        std::vector<std::string> grindTypes;
        std::vector<int> versions;
        bool isNewerFirst = true;
        bool isRecentlyAdded = false;
    };

    // Body that is returned from get_levels
    struct GetLevelsResponse {
        bool ok;
        int count;
        std::vector<int> ids;
    };

    struct LevelSuggestion {
        int id;
        int points;
    };

    struct NoteInfo {
        int levelID;
        std::string note;
        std::string senderUsername;
    };
    
    // Body returned from bootup_get
    struct BootupGetResponse {
        bool ok;
        std::vector<int> owners;
        std::vector<int> helpers;
        std::vector<int> admins;
        std::vector<int> artists;
        std::vector<int> boosters;
        std::vector<int> contributors;
        std::unordered_multimap<int, NoteInfo> notes;
        std::unordered_set<int> levelsWithCoins;
        std::unordered_set<int> levelsWithoutCoins;
    };

    enum GrindPosition {
        User = 0,
        Helper = 1,
        Admin = 2,
        Owner = 3
    };

    struct PointInfo {
        int point;
        int coinPoint;
        int staffId;
        std::string staffUsername;
    };

    struct GetLevelInfoResponse {
        bool ok;
        bool isAdded;
        bool isLocked;
        bool star;
        bool moon;
        bool coin;
        bool demon;
        std::string addedBy;
        bool noteExists;
        std::string note;
        bool isDaily;
        bool isDailyPlat;
        bool isWeekly;
        bool isWeeklyPlat;
        bool hasPoints;
        int points;
        int coinPoints;
        std::vector<PointInfo> pointsInfo;
    };

    enum PointType {
        AcceptPoint = 1,
        RejectPoint = -1
    };

    enum CoinPointType {
        AcceptCoinPoint = 1,
        RejectCoinPoint = -1
    };

    struct ChangePointResponse {
        bool ok;
    };

    struct SyncLevelsResponse {
        bool ok;
        int deleted;
        int inserted;
        int coinUpdates;
    };

    struct ManageLevelBody {
        int id = 0;
        std::string name = "";
        int difficulty = 0;
        int length = 0;
        int demonDifficulty = 0;
        bool star = false;
        bool moon = false;
        int coin = 0;
        bool demon = false;
    };

    struct CancelVoteResponse {
        bool ok;
    };

    enum GuidePage {
        MainGuide = 0,
        PetGuide = 1,
        WeeklyAchievementsGuide = 2,
        NotesGuide = 3,
        OwnerRoleGuide = 4,
        AdminRoleGuide = 5,
        HelperRoleGuide = 6,
        ArtistRoleGuide = 7,
        ContribRoleGuide = 8,
        BoosterRoleGuide = 9,
        MainPage = 11,
        EventsGuide = 12,
        GrindPacksGuide = 13,
        HelperSuggestionsGuide = 14,
        LevelIndicatorsGuide = 15,
        CreditsInfo = 16
    };

    struct Indicators {
        bool ok;
        bool added;
        bool coin;
        bool event;
        bool pack;
    };

    enum GuidePopupState {
        FromMainLayer = 0,
        FromOutside = 1
    };

    struct EventInfo {
        bool exists;
        int levelId;
        int id;
        double secondsLeft;
    };

    struct Events {
        bool ok;
        EventInfo classicEvent;
        EventInfo platEvent;
    };

    enum EventType {
        Daily = 0,
        Weekly = 1,
        Monthly = 2
    };

    enum CustomBrowserType {
        Search = 0, EventsHistory = 1
    };

    enum AchievementCellType {
        First = 0, Second = 1, Third = 2
    };

    enum AchievementType {
        Star = 0,
        Moon = 1,
        Coin = 2,
        Demon = 3
    };

    struct AchievementCellInfo {
        std::string username;
        int accountID;
        int icon;
        int colorFirst;
        int colorSecond;
        int glow;
        std::string title;
        int type;
        std::string info;
    };

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

    struct WeeklyAchResponse {
        bool ok;
        AchievementCellInfo cell1;
        AchievementCellInfo cell2;
        AchievementCellInfo cell3;
    };

    struct GrindPackLevels {
        int id1;
        int id2;
        int id3;
    };

    enum CustomDifficultyEnum {
        Auto = 1,
        Easy = 2,
        Normal = 3,
        Hard = 4,
        Harder = 5,
        Insane = 6,
        EasyDemon = 7,
        MediumDemon = 8,
        HardDemon = 9,
        InsaneDemon = 10,
        ExtremeDemon = 11
    };

    struct GrindPack {
        int id;
        std::string title;
        cocos2d::ccColor3B color;
        GrindPackLevels levels;
        CustomDifficultyEnum difficulty;
        bool isStar;
    };

    struct GetGrindPacksResponse {
        bool ok;
        std::vector<GrindPack> packs;
    };

    struct AnnouncementInfo {
        int id;
        std::string title;
        std::string content;
        std::string addedBy;
        std::string createdAt;
    };

    struct AnnouncementsResponse {
        std::vector<AnnouncementInfo> announcements;
        bool ok;
    };

    struct AddAnnouncementResponse {
        bool ok;
    };

    struct DeleteAnnouncementResponse {
        bool ok;
    };

    struct UserRoles {
        bool isOwner;
        bool isAdmin;
        bool isHelper;
        bool isArtist;
        bool isBooster;
        bool isContributor;
    };

    struct GetUserRolesResponse {
        bool ok;
        UserRoles roles;
        bool petExists;
        bool isPetBanned;
    };

    struct SetRolesBody {
        int accountID;
        std::string token;
        int targetAccountID;
        std::string targetUsername;
        int targetIcon;
        int targetColor1;
        int targetColor2;
        int targetColor3;
        bool isAdmin;
        bool isHelper;
        bool isArtist;
        bool isContributor;
        bool isBooster;
    };

    struct SetRolesResponse {
        bool ok;
    };

    struct WipePetResponse {
        bool ok;
    };

    struct BanPetResponse {
        bool ok;
    };

    struct UnbanPetResponse {
        bool ok;
    };

    enum Length {
        Short = 1,
        Medium = 2,
        Long = 3,
        XL = 4
    };

    struct LengthInfo {
        Length length;
        std::string lengthName;
        std::string lengthId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct VersionInfo {
        int version;
        std::string versionName;
        std::string versionId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct FilterInfo {
        std::string filterName;
        std::string top;
        std::string filterId;
        cocos2d::SEL_MenuHandler cb;
    };

    struct DemonDifficultyInfo {
        int demonDifficulty;
        std::string sprite;
        std::string id;
        cocos2d::SEL_MenuHandler cb;
    };

    struct NewGrindPackBody {
        std::string title;
        bool star;
        int levelId1;
        int levelId2;
        int levelId3;
        int color1;
        int color2;
        int color3;
        int difficulty;
    };

    struct ReqAccessResponse {
        bool ok;
        int pos;
    };
}