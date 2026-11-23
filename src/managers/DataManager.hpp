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

    std::vector<DiscordPopup::DiscordPageInfo> discordPages {
        {
            "Stay notified!",
            "By joining the <cp>Level Grind</c> server, you will get all the latest <cf>updates</c>, <cf>announcements</c>, and <cf>news</c> about the mod!"
        },
        {
            "Talk with the Level Grind Bot!",
            "Chat with the <cp>Level Grind</c> Bot! Ask questions about the mod, or just have a casual conversation :)"
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
            "The <cp>Level Grind</c> server has suggestion channels for <cy>levels</c>, <ca>notes</c>, <cj>lists</c>, <cf>grind packs</c>, as well as <cl>server</c> and <cp>mod</c> suggestions! Share your ideas and bring us something new! :D"
        },
        {
            "Talk to other mod players!",
            "And finally, <cg>engage</c> with the awesome <cp>Level Grind</c> community!"
        }
    };

public:
    void setSharedData(BootupGetResponse sharedDataArg);
    void setUserPosition(GrindPosition pos);
    void clearSharedData();

    BootupGetResponse const& getSharedData() const;
    GrindPosition getUserPosition();
    GrindPosition getUserPosFromSaved();
    std::vector<DiscordPopup::DiscordPageInfo> getDiscordPages();

    void initPermsOnBootup();
    void initTokenOnBootup();

};

}
