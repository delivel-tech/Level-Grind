#pragma once
#include "../utils/singleton.hpp"
#include "../utils/globals.hpp"
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
            "By joining the <cp>Level Grind</c> server, you will receive <cy>all informations</c> regarding any updates, annoucements etc about the mod!"
        },
        {
            "Talk to Level Grind (AI)",
            "Level Grind <cr>Bot</c> has an AI you can talk to! Ask it any questions about the mod, or just have a casual conversation :)"
        },
        {
            "Become a Booster!",
            "By boosting <cp>Level Grind</c> Discord Server, you'll receive an <co>Exclusive Booster Badge</c>!"
        },
        {
            "Share your progress!",
            "Tell everyone how you progressed in GD! Share your <cy>milestones</c>, accomplishments etc!"
        },
        {
            "Suggest new ideas!",
            "<cp>Level Grind</c> server has <cy>mod, server, note, and grind pack</c> suggestion channels! Offer us something new :D"
        },
        {
            "Talk to other mod players",
            "And finally, <cp>engage</c> with the greatest <cy>Level Grind</c> community!!"
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