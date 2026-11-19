#include "Consent.hpp"
#include "../ui/popups/ConsentPopup.hpp"

namespace levelgrind::consent {

namespace {

geode::Function<void()>& onChange() {
    static geode::Function<void()> callback;
    return callback;
}

}

bool decided() {
    return Mod::get()->getSavedValue<int>("consent-version", 0) >= kVersion;
}

bool granted() {
    return decided() && Mod::get()->getSavedValue<bool>("consent-data", false);
}

void set(bool accepted) {
    Mod::get()->setSavedValue<int>("consent-version", kVersion);
    Mod::get()->setSavedValue<bool>("consent-data", accepted);
    if (onChange()) onChange()();
}

void setOnChange(geode::Function<void()> callback) {
    onChange() = std::move(callback);
}

void showPopup(geode::Function<void()> onAnswered) {
    ConsentPopup::create(std::move(onAnswered))->show();
}

bool guard() {
    if (granted()) return true;
    createQuickPopup(
        "Consent required",
        "You declined data collection. Accept the <cy>Privacy Policy</c> to use this feature.",
        "Cancel", "Review",
        [](auto, bool review) {
            if (review) showPopup();
        }
    );
    return false;
}

const char* const kRules =
    "# <cy>Level Grind Rules</c>\n\n"
    "By using the online features of <cp>Level Grind</c> you agree to these rules.\n\n"
    "### Be respectful\n\n"
    "- No harassment, hate speech, slurs or threats in clan chat, clan names, descriptions, pet names or anywhere else.\n"
    "- No NSFW, spam or advertising.\n"
    "- Don't pretend to be someone else, including staff.\n\n"
    "### Play fair\n\n"
    "- Don't use hacks, cheats or modified completions to earn stars, moons, coins or <cy>Grind Points</c>.\n"
    "- Don't use alt accounts to farm Grind Points, clan progress or leaderboard spots.\n"
    "- Don't abuse bugs. If you find one, report it on our Discord.\n\n"
    "### Staff\n\n"
    "- Staff can remove content and ban players from clans, leaderboards or pets when these rules are broken.\n"
    "- Staff decisions are final, but you can appeal on our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server.\n";

const char* const kPrivacy =
    "# <cy>Level Grind Privacy Policy</c>\n\n"
    "This explains what <cp>Level Grind</c> stores about you and why. Nothing is collected until you accept it.\n\n"
    "### What we store\n\n"
    "- Your GD <cy>account ID</c>, <cy>username</c> and icon (cube and colors).\n"
    "- Levels you beat with the mod: level ID, stars or moons, collected user coins and the time.\n"
    "- Your <cy>Grind Points</c>, achievements, Stat Tracker stats, pet and Grind Pack progress.\n"
    "- Your clan membership, clan activity and the clan chat messages you send.\n"
    "- Reports you send and staff actions on your account.\n\n"
    "### Why\n\n"
    "To run Grind Points, Grind Clans, the Stat Tracker, achievements, pets, leaderboards and Grind Profiles.\n\n"
    "### Who can see it\n\n"
    "Your username, icon, Grind Profile, leaderboard places and clan info are <cg>public</c> to other Level Grind players. Clan chat is visible to your clan and to staff.\n\n"
    "### Other services\n\n"
    "- <cl>Argon</c> is used to verify that your GD account is yours.\n"
    "- Player lookups go to the GD servers and GDBrowser.\n"
    "- We never sell your data or share it with anyone else.\n\n"
    "### Keeping and deleting\n\n"
    "Data is kept on the Level Grind server while the feature it belongs to exists. You can withdraw consent at any time in <cy>Settings</c>: the mod stops sending data right away. To have your stored data deleted, contact us on our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server.\n";

const char* const kTerms =
    "# <cy>Summary</c>\n\n"
    "- Accepting lets <cp>Level Grind</c> store your completions and stats to power its online features.\n"
    "- Declining keeps the level database, packs, lists, events and notes available, but turns off Clans, Grind Points, Calendar, Leaderboards, Profiles, Pets and Achievements.\n"
    "- You can change your choice at any time in <cy>Settings</c>.\n";

}
