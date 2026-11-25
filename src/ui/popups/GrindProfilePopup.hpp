#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <optional>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class GrindProfilePopup : public BasePopup {
public:
    static GrindProfilePopup* create(int accountId, std::string const& username = "");

    struct Data {
        bool exists = false;
        int accountId = 0;
        std::string username;
        int icon = 0, color1 = 0, color2 = 3, color3 = 0;
        long long gpLifetime = 0, gpWeek = 0;
        int achUnlocked = 0, achTotal = 0;
        int streak = 0;
        int packs = 0;
        int last30Levels = 0, last30Stars = 0, last30Moons = 0, last30Demons = 0;
        bool hasClan = false;
        long long badgeColors = -1;
        int clanId = 0, clanColor = 0, clanEmblem = 1, clanLevel = 1, clanRole = 0, clanJoinType = 0;
        std::string clanName;
        bool hasPet = false;
        std::string petName;
        int petLevel = 1, petRarity = 1;
        bool hasAppearance = false;
        int petMood = 1;
        matjson::Value appearance;
    };

private:
    bool init(int accountId, std::string const& username);
    void load();
    void loadGD();
    void build();
    void buildHeader(CCNode* root, float w, float h);
    void buildStats(CCNode* root, float w, float y);
    void buildChips(CCNode* root, float w, float y);

    Data m_data;
    bool m_loaded = false;
    bool m_failed = false;
    std::string m_nameHint;
    CCNode* m_root = nullptr;
    CCNode* m_spinner = nullptr;
    TaskHolder<web::WebResponse> m_task;
    TaskHolder<web::WebResponse> m_gdTask;

    ~GrindProfilePopup() {
        m_task.cancel();
        m_gdTask.cancel();
    }
};

}
