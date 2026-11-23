#include "GuidePopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/MDTextArea.hpp"
#include "Geode/ui/ScrollLayer.hpp"
#include "Geode/ui/Scrollbar.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCScrollLayerExt.hpp>
#include <cstddef>
#include <fmt/format.h>

#define SAFE_REMOVE(id) if (auto x = getChildByIDRecursive(id)) x->removeFromParent()

using namespace geode::prelude;

#include <UIBuilder.hpp>
#include <cue/ListNode.hpp>

namespace levelgrind {

GuidePopup* GuidePopup::create(GuidePage page, GuidePopupState state) {
    auto ret = new GuidePopup;
    if (ret && ret->init(page, state)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GuidePopup::init(GuidePage page, GuidePopupState state) {
    if (!BasePopup::init({ 380.f, 260.f }, "GJ_square01.png")) return false;

    m_state = state;

    removePrevious();

    if (!initPage(page)) return false;

    return true;
}

void GuidePopup::addReturnButton() {
    auto reqBtnMenu = Build(CCMenu::create())
        .parent(m_mainLayer)
        .pos(
            30.f,
            (m_mainLayer->getContentHeight() / 2) + 20.f
        )
        .id("return-btn-menu")
        .collect();
    auto retBtn = Build(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"))
        .scale(0.7f)
        .intoMenuItem([this] {
            initPage(GuidePage::MainPage);
        })
        .parent(reqBtnMenu)
        .scaleMult(1.1f)
        .id("return-btn")
        .collect();
    return;
}

void GuidePopup::removePrevious() {
    SAFE_REMOVE("grind-logo");
    SAFE_REMOVE("list-node");
    for (size_t i; i < 12; i++) {
        SAFE_REMOVE(fmt::format("section-{}", i));
        SAFE_REMOVE(fmt::format("section-button-{}", i));
    }
    SAFE_REMOVE("md-text-area");
    SAFE_REMOVE("return-btn");
    SAFE_REMOVE("return-btn-menu");
    SAFE_REMOVE("guidebook-label");
    SAFE_REMOVE("scrollbar");
}

void GuidePopup::addTextArea(std::string text) {
    auto textArea = Build(MDTextArea::create(
        text,
        { 280.f, 160.f }
    ))
        .pos({centerX(), centerY() - 20.f})
        .parent(m_mainLayer)
        .id("md-text-area")
        .collect();
    
    return;
}

void GuidePopup::addScrollbar() {
    auto scrollLayer = m_listNode->getScrollLayer();
    auto scrollBar = Build(Scrollbar::create(scrollLayer))
        .pos(
            m_mainLayer->getContentWidth() - 40.f,
            m_mainLayer->getContentHeight() / 2.f - 20.f
        )
        .scale(0.9f)
        .parent(m_mainLayer)
        .id("scrollbar")
        .collect();

    return;
}

void GuidePopup::addScrollbar(CCScrollLayerExt* scrollLayer) {
    auto scrollBar = Build(Scrollbar::create(scrollLayer))
        .pos(
            m_mainLayer->getContentWidth() - 40.f,
            m_mainLayer->getContentHeight() / 2.f - 20.f
        )
        .scale(0.9f)
        .parent(m_mainLayer)
        .id("scrollbar")
        .collect();

    return;
}

void GuidePopup::addGrindLogo() {
    auto grindLogo = Build(CCSprite::create("lg-logo.png"_spr))
        .pos(
            m_mainLayer->getContentWidth() / 2.f,
            m_mainLayer->getContentHeight() / 1.2f + 17.f
        )
        .parent(m_mainLayer)
        .scale(0.8f)
        .id("grind-logo")
        .collect();

    auto guidebookLabel = Build(CCLabelBMFont::create("Mod Guidebook", "bigFont.fnt"))
        .pos(
            m_mainLayer->getContentWidth() / 2.f,
            m_mainLayer->getContentHeight() / 1.2f - 13.f
        )
        .parent(m_mainLayer)
        .scale(0.5f)
        .id("guidebook-label")
        .collect();

    return;
}

bool GuidePopup::initPage(GuidePage page) {
    removePrevious();

    switch (page) {
        case GuidePage::MainPage:
            return initMainPage();
        case GuidePage::MainGuide:
            return initMainGuidePage();
        case GuidePage::PetGuide:
            return initPetGuidePage();
        case GuidePage::ClansGuide:
            return initClansGuidePage();
        case GuidePage::CalendarGuide:
            return initCalendarGuidePage();
        case GuidePage::LeaderboardsGuide:
            return initLeaderboardsGuidePage();
        case GuidePage::GrindListsGuide:
            return initGrindListsGuidePage();
        case GuidePage::WeeklyAchievementsGuide:
            return initWeeklyAchievementsGuidePage();
        case GuidePage::NotesGuide:
            return initNotesGuidePage();
        case GuidePage::OwnerRoleGuide:
            return initOwnerRoleGuidePage();
        case levelgrind::GuidePage::DeveloperRoleGuide:
            return initDeveloperRoleGuidePage();
        case GuidePage::AdminRoleGuide:
            return initAdminRoleGuidePage();
        case GuidePage::HelperRoleGuide:
            return initHelperRoleGuidePage();
        case GuidePage::ArtistRoleGuide:
            return initArtistRoleGuidePage();
        case GuidePage::ContribRoleGuide:
            return initContribRoleGuidePage();
        case GuidePage::BoosterRoleGuide:
            return initBoosterRoleGuidePage();
        case GuidePage::EventsGuide:
            return initEventsGuidePage();
        case GuidePage::GrindPacksGuide:
            return initGrindPacksGuide();
        case GuidePage::HelperSuggestionsGuide:
            return initHelperSuggestionsGuide();
        case GuidePage::LevelIndicatorsGuide:
            return initLevelIndicatorsGuide();
        case GuidePage::CreditsInfo:
            return initCreditsInfoGuide();
    }

    return false;
}

bool GuidePopup::initMainPage() {
    addGrindLogo();

    auto listNode = Build(cue::ListNode::create({280.f, 160.f}))
        .pos({centerX(), centerY() - 20.f})
        .parent(m_mainLayer)
        .id("list-node")
        .collect();
        
    m_listNode = listNode;

    addScrollbar();

    auto createGuideSection = [listNode, this](std::string name, auto callback, int sectionId) {
        auto section = Build(CCMenu::create())
            .id(fmt::format("section-{}", sectionId))
            .contentSize({ 280.f, 40.f })
            .collect();

        auto sectionButton = Build(ButtonSprite::create(name.c_str(), 250.f, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 0.7f))
            .scale(0.8f)
            .intoMenuItem(callback)
            .scaleMult(1.1f)
            .parent(section)
            .id(fmt::format("section-button-{}", sectionId))
            .center()
            .collect();

        return section;
    };
    
    listNode->addCell(createGuideSection("Grind Info", [this] { initPage(GuidePage::MainGuide); }, 1));
    listNode->addCell(createGuideSection("Pet Info", [this] { initPage(GuidePage::PetGuide); }, 2));
    listNode->addCell(createGuideSection("Grind Clans", [this] { initPage(GuidePage::ClansGuide); }, 17));
    listNode->addCell(createGuideSection("Stat Tracker", [this] { initPage(GuidePage::CalendarGuide); }, 18));
    listNode->addCell(createGuideSection("Grind Leaderboards", [this] { initPage(GuidePage::LeaderboardsGuide); }, 19));
    listNode->addCell(createGuideSection("Grind Lists", [this] { initPage(GuidePage::GrindListsGuide); }, 20));
    listNode->addCell(createGuideSection("Event Levels", [this]{initPage(GuidePage::EventsGuide);}, 11));
    listNode->addCell(createGuideSection("Grind Packs", [this]{initPage(GuidePage::GrindPacksGuide);}, 12));
    listNode->addCell(createGuideSection("Weekly Achievements", [this] { initPage(GuidePage::WeeklyAchievementsGuide); }, 3));
    listNode->addCell(createGuideSection("Helper Suggestions", [this]{initPage(GuidePage::HelperSuggestionsGuide);}, 13));
    listNode->addCell(createGuideSection("Level Indicators", [this]{initPage(GuidePage::LevelIndicatorsGuide);}, 14));
    listNode->addCell(createGuideSection("Credits Info", [this]{initPage(GuidePage::CreditsInfo);}, 15));
    listNode->addCell(createGuideSection("Notes Info", [this] { initPage(GuidePage::NotesGuide); }, 4));
    listNode->addCell(createGuideSection("Coordinator Role", [this] { initPage(GuidePage::OwnerRoleGuide); }, 5));
    listNode->addCell(createGuideSection("Developer Role", [this]{initPage(GuidePage::DeveloperRoleGuide);}, 16));
    listNode->addCell(createGuideSection("Admin Role", [this] { initPage(GuidePage::AdminRoleGuide); }, 6));
    listNode->addCell(createGuideSection("Helper Role", [this] { initPage(GuidePage::HelperRoleGuide); }, 7));
    listNode->addCell(createGuideSection("Artist Role", [this] { initPage(GuidePage::ArtistRoleGuide); }, 8));
    listNode->addCell(createGuideSection("Contributor Role", [this] { initPage(GuidePage::ContribRoleGuide); }, 9));
    listNode->addCell(createGuideSection("Booster Role", [this] { initPage(GuidePage::BoosterRoleGuide); }, 10));

    listNode->getScrollLayer()->m_contentLayer->updateLayout();
    listNode->updateLayout();
    listNode->scrollToTop();

    m_listNode->getScrollLayer()->m_contentLayer->updateLayout();
    m_listNode->scrollToTop();
    
    return true;
}

void GuidePopup::keyBackClicked() {
    if (m_state == GuidePopupState::FromMainLayer) {
        if (auto textArea = getChildByIDRecursive("md-text-area")) {
            initPage(GuidePage::MainPage);
        } else {
            this->onClose(nullptr);
        }
    } else {
        this->onClose(nullptr);
    }
}

bool GuidePopup::initMainGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Level Grind Guide</c>\n\n"
        "<cp>Level Grind</c> is a mod designed specifically for stats grinders who want a streamlined way to find easy, readable, and fun levels.\n\n"
        "The mod offers a large <cl>database</c> full of <cy>grinding-worthy</c> levels that are chosen by our staff team based on various criteria.\n\n"
        "The database is being actively filled, making it a perfect place to evolve your account stats!\n\n"
        "### Search Options\n\n"
        "- <cy>**Star**</c>: Searching for classic levels.\n"
        "- <cb>**Moon**</c>: Searching for platformer levels.\n"
        "- <cc>**Coin**</c>: Searching for levels with easy or free coins.\n"
        "- <cr>**Demon**</c>: Searching for demon levels.\n"
        "- <cf>**Random Button**</c>: Searching for a level selected at random based on your currently active filters.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initLevelIndicatorsGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cf>Level Indicators Guide</c>\n\n"
        "<cp>Level Grind</c> adds a button next to the level's name that tells you if the level is added to the <cl>database</c>.\n\n"
        "### Indicators\n\n"
        "- <cc>**Coin Indicator**</c>: The level has easy or free coins.\n"
        "- <ca>**Event Indicator**</c>: The level was, at one point, a <cy>Daily</c>, <co>Weekly</c>, or <cr>Monthly</c> level.\n"
        "- <cf>**Grind Pack Indicator**</c>: The level is part of a Grind Pack.\n"
        "- <cf>**Lock Indicator**</c>: Indicates whether the level is locked, which prevents further changes to it in the database.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initGrindPacksGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cf>Grind Packs Guide</c>\n\n"
        "<cf>Grind Packs</c> are collections of 3 levels from the <cp>Level Grind</c> database.\n\n"
        "Featuring both <cy>Classic</c> and <cb>Platformer</c> packs, with new ones being added regularly.\n\n"
        "Grind Packs are usually added from suggestions in our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server, so consider joining! :D\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initCreditsInfoGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Credits Guide</c>\n\n"
        "These users contribute to <cp>Level Grind</c> in various ways! You can find out more about a specific role by clicking the <cj>**Info Button**</c> next to its title.\n\n"
        "Have any suggestions or want to report a bug? Join our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server and let us know!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initPetGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cd>Grinding Pet Guide</c>\n\n"
        "<cd>Grinding Pet</c> is your own companion that evolves when you grind stats!\n\n"
        "### How does it work?\n\n"
        "- <cy>**Grind Points (GP)**</c>: Upgrade your pet using <cy>GP</c> earned from levels, events, packs, achievements and clan tasks.\n"
        "- <ca>**Evolution**</c>: Each level-up makes your pet grow larger (up to level 1000) and may change its style. Each level costs slightly more GP.\n"
        "- <cj>**Stages**</c>: Baby, Toddler, Teen, Adult, Elder, Master, and <cy>Ascended</c> (level 30+).\n"
        "- <cf>**Rarity**</c>: Common, <co>Rare</c> (1,500 GP), <cp>Legendary</c> (4,000 GP), and <cl>Mythic</c> (8,000 GP), followed by <cl>Mythic I, II, III</c>+ (each 1.5x the previous price).\n"
        "- <cs>**Customization**</c>: <co>Rare</c> unlocks the customization menu. Higher rarities give you more options, like choosing your pet's icon.\n"
        "- <cg>**Feeding**</c>: Feed your pet once a day for <cy>25 GP</c>, or beat any level. Fed today: <cg>Happy</c>, yesterday: <cy>Okay</c>, longer: <cr>Hungry</c>.\n"
        "- <co>**Streak**</c>: Feed it every day for <cy>+150 GP</c> at 7 days and <cy>+1000 GP</c> at 30 days.\n\n"
        "Old <cy>Pet Stars</c> and <cb>Pet Moons</c> were turned into GP automatically.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initClansGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Grind Clans Guide</c>\n\n"
        "Team up with other grinders, climb the clan leaderboards together and earn <cy>Grind Points</c>!\n\n"
        "### Grind Points (GP)\n\n"
        "<cy>Grind Points</c> (<cy>GP</c>) are your currency. Used for both your <cd>pet</c> and your clan.\n\n"
        "- <cy>**Any Rated Level**</c>: 1 GP per star/moon, once per level.\n"
        "- <cg>**Level Grind Levels**</c>: 2 GP per star/moon.\n"
        "- <co>**Event Levels**</c>: x2 daily, x3 weekly, and x4 monthly while the event is live.\n"
        "- <cp>**Grind Packs**</c>: +50 GP for beating all three levels.\n"
        "- <cl>**Clan Weekly Tasks**</c>: GP awarded to every member who helped that week.\n"
        "- <ca>**Achievements**</c>: GP for every achievement you unlock.\n\n"
        "You can earn up to <cy>400 GP a day</c> from levels. Packs, tasks and achievements do not count toward this limit.\n\n"
        "### Clans\n\n"
        "- <cg>**Creating**</c>: Creating a clan costs <cy>300 GP</c>. A new color or emblem costs the leader <cy>50 GP</c>, and at level 30, the leader can submit a custom clan logo that requires admin approval.\n"
        "- <co>**Power**</c>: A clan's power is the GP its members earned while in it. It sets the clan level (up to 30), member slots (20, +2 per level, up to 50), and a GP bonus for everyone (+1% per level, up to +20%).\n"
        "- <cl>**Weekly Tasks**</c>: Every week, the clan gets <cl>12 tasks</c> across four tiers: <cg>Easy</c>, <cy>Medium</c>, <co>Hard</c>, and <cr>Very Hard</c>. Harder tiers require much more, but pay more GP.\n"
        "- <cj>**Roles**</c>: The leader runs the clan, officers accept requests and can remove members.\n"
        "- <ca>**Clan Chat**</c>: Chat with your clan right on the clan screen.\n"
        "- <cr>**Clan Wars**</c>: Every week your clan is paired with a similar one. The clan that earns more GP wins <cy>+100 GP</c> for every member.\n"
        "- <cy>**Treasury**</c>: Members donate GP, and the leader spends it on a <cg>GP Boost</c> or a <cy>Golden Name</c>.\n"
        "- <cg>**Joining**</c>: Clans can be <cg>Open</c>, <cy>By request</c>, or <cr>Invite only</c>. Officers can invite players from their Grind Profile.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());

    return true;
}

bool GuidePopup::initCalendarGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Stat Tracker Guide</c>\n\n"
        "See what you grinded each day on a monthly grid.\n\n"
        "- <cy>**Stats**</c>: Switch between <cy>Stars</c>, <cb>Moons</c>, <cg>Levels</c>, <cr>Demons</c>, <cc>Coins</c> and <co>Grind Points</c>.\n"
        "- <cl>**Days**</c>: Press a specific day to see everything you earned on it.\n"
        "- <co>**Streak**</c>: Days in a row with at least one level beaten.\n"
        "- <cg>**Goals**</c>: Set daily, weekly, and monthly goals for any stat. You get a notification when you reach one.\n\n"
        "Only levels beaten after installing the mod count towards the stat tracker.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    return true;
}

bool GuidePopup::initLeaderboardsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Grind Leaderboards Guide</c>\n\n"
        "See the top grinders for each time period. Every cycle starts from zero, so anyone can reach the top!\n\n"
        "- <cy>**Daily**</c>: Resets every day at 00:00 UTC.\n"
        "- <co>**Weekly**</c>: Resets every Monday.\n"
        "- <cr>**Monthly**</c>: Resets on the 1st of every month.\n\n"
        "Rank by <cy>Stars</c>, <cb>Moons</c>, <cr>Demons</c>, <cc>Coins</c> or <co>Grind Points</c>. Tap a player to open their **Grind Profile**.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    return true;
}

bool GuidePopup::initGrindListsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cl>Grind Lists Guide</c>\n\n"
        "Both Unrated and Rated lists, picked for grinding.\n\n"
        "- <cr>**Unrated**</c>: Any stat-grinding lists, like star/moon grinding lists, lists with easy coins, or lists with short platformers.\n"
        "- <cy>**Rated**</c>: Usually lists that are easy to claim the diamond reward from.\n\n"
        "### Search Options\n\n"
        "- **Max Levels**: How many levels the list has.\n"
        "- **Max Left**: How many levels you still need to beat to complete the list, or to claim the diamond reward on rated lists.\n"
        "- **Hide Done**: Hides lists you have already finished or claimed the reward from.\n"
        "- **Sort**: <cj>Best Value</c> (diamonds per level), <cy>Almost Done</c>, <cj>Most Diamonds</c>, or <cg>Newest</c>.\n\n"
        "Lists can be used as an alternative way to grind stats. List data is refreshed every day.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    return true;
}

bool GuidePopup::initWeeklyAchievementsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Weekly Achievements Guide</c>\n\n"
        "Highlighting major achievements from the <cp>Level Grind</c> community! Each week, 3 new accomplishments are chosen for display.\n\n"
        "### How to get featured?\n\n"
        "Weekly achievements are selected by <cc>Grind Admins</c> from achievements posted in our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server, so consider joining! :D\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initHelperSuggestionsGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Helper Suggestions Guide</c>\n\n"
        "This tab features levels voted on by our <cg>Helpers</c>! These may eventually be added to the main database if they are considered grindable.\n\n"
        "### Bounty Levels\n\n"
        "If a level stays in this tab for a while without gaining enough votes to be added, it can become a <cy>bounty</c>, indicated by a <cy>gold cell</c>. Bounty levels grant a large GP bonus when completed!\n\n"
        "Want to suggest a level? Consider joining our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initEventsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cr>Event Levels Guide</c>\n\n"
        "A pair of grind-worthy <cy>Classic</c> and <cb>Platformer</c> levels, selected regularly by <cc>Grind Admins</c>.\n\n"
        "### Event Types\n\n"
        "- <cy>**Daily**</c>: Levels ranging in difficulty from <cs>Auto</c> to <cp>Insane</c>.\n"
        "- <co>**Weekly**</c>: Levels ranging in difficulty from <cj>Easy Demon</c> to <cl>Medium Demon</c>.\n"
        "- <cr>**Monthly**</c>: Levels ranging in difficulty from <co>Hard Demon</c> to low-end <cr>Extreme Demon</c>.\n"
    );

    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());

    return true;
}

bool GuidePopup::initNotesGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <ca>Level Notes Guide</c>\n\n"
        "Our staff team can add notes to levels and lists, pointing out details like <cc>coin</c> locations, tricky timings, difficult sections, or anything else players should know.\n\n"
        "- <cg>**Levels**</c>: When a level has a note attached, press the note button on the left side of the level page to read it.\n"
        "- <cl>**Lists**</c>: Helpers can now also add notes to lists. Look for the note button on the right side of the list page.\n\n"
        "Want to suggest a note? Consider joining our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initOwnerRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Coordinator Role</c>\n\n"
        "<cp>Coordinators</c> are responsible for managing the entire **Grind Staff** team and making key decisions, giving them greater influence over the project.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initDeveloperRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Developer Role</c>\n\n"
        "<cg>Developers</c> are responsible for developing the <cp>Level Grind</c> project. They fix bugs, add new features to the mod, and more.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initAdminRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cc>Admin Role</c>\n\n"
        "<cc>Admins</c> are responsible for maintaining the database, moderating clans, pets, seasonal leaderboards, and more. They also manage the <cg>Helper</c> team to ensure the project stays organized.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initHelperRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Helper Role</c>\n\n"
        "<cg>Helpers</c> are an integral part of the <cp>Level Grind</c> mod. They are responsible for voting on whether levels are grind-worthy, providing helpful notes for levels and lists, and more.\n\n"
        "You can join our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server to learn more about the role and how you can become one too! :)\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initArtistRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Artist Role</c>\n\n"
        "<cp>Artists</c> are responsible for the visual part of the mod, including sprites, textures, and more. Their support is greatly appreciated! <cr><3</c>\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initContribRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Contributor Role</c>\n\n"
        "<cy>Contributor</c> is a role given to [<cy>Boosty</c>](https://boosty.to/deliveltech) subscribers, people who have donated an equivalent of $5 or more, or people who have provided other meaningful help to the <cp>Level Grind</c> mod.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initBoosterRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <ca>Booster Role</c>\n\n"
        "<ca>Boosters</c> help our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server by providing server boosts. Their support is greatly appreciated! :3\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

}
