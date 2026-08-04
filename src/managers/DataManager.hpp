#pragma once
#include "../utils/singleton.hpp"
#include "../features/levels/LevelTypes.hpp"
#include "../core/CoreTypes.hpp"
#include "../ui/popups/DiscordPopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class DataManager : public Singleton<DataManager> {
    friend class Singleton<DataManager>;

private:
    DataManager() = default;
    BootupGetResponse sharedData;
    GrindPosition userPosition;
    std::string userToken;

    std::vector<DiscordPopup::DiscordPageInfo> discordPages {
        {
            "Stay notified!",
            "By joining the <cp>Level Grind</c> server, you will get all the latest <cf>updates</c>, <cf>announcements</c>, and <cf>news</c> about the mod!"
        },
        {
            "Talk to Level Grind (AI)",
            "Chat with the <cp>Level Grind</c> <cr>Bot</c> AI! Ask questions about the mod, or just have a casual conversation :)"
        },
        {
            "Become a Booster!",
            "By boosting the <cp>Level Grind</c> <cb>Discord</c> Server, you'll receive an exclusive <ca>Booster Badge</c>!"
        },
        {
            "Share your progress!",
            "Tell everyone how you've progressed in GD! Share your <cy>milestones</c>, <cy>accomplishments</c>, and more!"
        },
        {
            "Suggest new ideas!",
            "The <cp>Level Grind</c> server has suggestion channels for the <cp>Mod</c>, <cg>Server</c>, <ca>Level Notes</c>, and <cf>Grind Packs</c>! Share your ideas and bring us something new! :D"
        },
        {
            "Talk to other mod players",
            "And finally, <cg>engage</c> with the awesome <cp>Level Grind</c> community!"
        }
    };

public:
    void setSharedData(BootupGetResponse sharedDataArg);
    void setUserPosition(GrindPosition pos);
    void setUserToken(const std::string& token);
    void clearSharedData();

    BootupGetResponse const& getSharedData() const;
    GrindPosition getUserPosition();
    GrindPosition getUserPosFromSaved();
    std::string getUserToken();
    std::string getUserTokenFromSaved();
    std::vector<DiscordPopup::DiscordPageInfo> getDiscordPages();

    void initPermsOnBootup();
    void initTokenOnBootup();

};

}
