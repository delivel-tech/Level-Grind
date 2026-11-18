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
        "The <cl>database</c> is actively filled by <cc>Grind Admins</c> making it a perfect place to evolve your account stats!\n\n"
        "### Searching options explanations:\n\n"
        "- <cy>**Star**</c>: searching for classic levels.\n"
        "- <cb>**Moon**</c>: searching for platformer levels.\n"
        "- <cc>**Coin**</c>: searching for levels with easy or free coins.\n"
        "- <cr>**Demon**</c>: searching for demon levels.\n"
        "- <cf>**Random Button**</c>: searching for a level selected at random based on your currently active filters.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initLevelIndicatorsGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cf>Level Grind Indicators</c>\n\n"
        "<cp>Level Grind</c> adds a button next to the level's name that tells you if the level is added to the <cl>database</c>.\n\n"
        "### Indicators:\n\n"
        "- <cc>**Coin Indicator**</c>: The level has easy or free <cc>coins</c>.\n"
        "- <ca>**Event Indicator**</c>: The level was, at one point, a <cy>Daily</c>, <co>Weekly</c>, or <cr>Monthly</c> level.\n"
        "- <cy>**Grind Pack Indicator**</c>: The level is part of a <cf>Grind Pack</c>.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initGrindPacksGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cf>Grind Packs Guide</c>\n\n"
        "Similar to <ca>RobTop's</c> \"Map Packs\", but with actually fun and <cy>grindable</c> levels!\n\n"
        "Featuring both <cy>Classic</c> and <cb>Platformer</c> packs, with <cg>new ones</c> being added regularly.\n\n"
        "Want to suggest a <cf>Grind Pack</c>? Consider joining our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initCreditsInfoGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Credits Info</c>\n\n"
        "These users contribute to <cp>Level Grind</c> in various ways! You can find out more about a specific role by clicking the <cj>Info Button</c> next to its title.\n\n"
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
        "<cd>Grinding Pet</c> is your own <cj>companion</c> that evolves when you <cy>grind stats</c>!\n\n"
        "### How does it work?\n\n"
        "- <cy>**Grind Points**</c>: Your pet is upgraded with <cy>Grind Points (GP)</c>, the same GP you earn from levels, events, packs, achievements and clan tasks.\n"
        "- <ca>**Evolution**</c>: There are <co>30 levels</c> to achieve. With every level up, your pet <cg>grows larger</c> and may change its <cj>style</c>! Each level costs a bit more GP than the last.\n"
        "- <cj>**Stages**</c>: Baby, Toddler, Teen, Adult, Elder, Master and finally <cy>Ascended</c> at level 30.\n"
        "- <cf>**Rarity**</c>: Common, then <cl>Rare</c> (1,500 GP), <co>Legendary</c> (4,000 GP) and <cr>Mythic</c> (8,000 GP).\n"
        "- <cs>**Customization**</c>: <cl>Rare</c> unlocks the <cs>Customization Menu</c>. Higher rarities give you more <cg>options</c>, like choosing your pet's icon.\n\n"
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
        "Team up with other grinders, climb the <cy>clan leaderboards</c> together and earn <cy>Grind Points</c>!\n\n"
        "### Grind Points (GP)\n\n"
        "<cy>Grind Points</c> are your own currency. You spend them on your <cd>pet</c> and your <cy>clan</c>, and they never leave your account.\n\n"
        "- <cy>**Any rated level**</c>: 1 GP per star/moon, once per level.\n"
        "- <cg>**Level Grind levels**</c>: 2 GP per star/moon.\n"
        "- <co>**Event levels**</c>: x2 daily, x3 weekly, x4 monthly while the event is live.\n"
        "- <cp>**Grind Packs**</c>: +50 GP for beating all three levels.\n"
        "- <cl>**Clan weekly tasks**</c>: GP for every member who helped that week.\n"
        "- <ca>**Achievements**</c>: GP for every achievement you unlock.\n\n"
        "You can earn up to <cy>400 GP a day</c> from levels. Packs, tasks and achievements don't count towards it.\n\n"
        "### Clans\n\n"
        "- <cg>**Creating**</c> a clan costs <cy>300 GP</c>. A new color or emblem costs the leader <cy>50 GP</c>.\n"
        "- A clan's <cy>power</c> is the GP its members earned while in it. It sets the clan <co>level</c> (up to 30), the <cj>member slots</c> (20, +2 per level, up to 50) and a <cg>GP bonus</c> for everyone (+1% per level, up to +20%).\n"
        "- Clans are ranked <cy>this week</c> and <cy>all time</c>.\n"
        "- Every week the clan gets <cl>12 tasks</c> in four tiers: <cg>Easy</c>, <cy>Medium</c>, <co>Hard</c> and <cr>Very Hard</c>. Harder tiers need much more, but pay much more GP.\n"
        "- <cj>**Roles**</c>: the <cy>Leader</c> runs the clan, <cl>Officers</c> accept requests and can remove members.\n"
        "- Chat with your clan right on the clan screen.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());

    return true;
}

bool GuidePopup::initWeeklyAchievementsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Weekly Achievements Guide</c>\n\n"
        "Highlighting major achievements from the <cp>Level Grind</c> community! Each week, <cy>3</c> new accomplishments are chosen for display.\n\n"
        "Weekly achievements are selected by <cc>Grind Admins</c> from achievements posted in our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initHelperSuggestionsGuide() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Helper Suggestions</c>\n\n"
        "This is a tab with levels suggested by our <cg>Helpers</c>! These may eventually be added to the main <cl>database</c> if they are considered <cy>grindable</c>.\n\n"
        "Want to suggest a level? Consider joining our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initEventsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cr>Event Levels</c>\n\n"
        "A pair of <cy>grind-worthy</c> <cy>Classic</c> and <cb>Platformer</c> levels, selected regularly by <cc>Grind Admins</c>.\n\n"
        "### There are 3 types of event levels:\n\n"
        "- <cy>**Daily**</c>: Grindable levels ranging in difficulty from <cs>Auto</c> to <cp>Insane</c>.\n"
        "- <co>**Weekly**</c>: Grindable levels ranging in difficulty from <cj>Easy Demon</c> to <cl>Medium Demon</c>.\n"
        "- <cr>**Monthly**</c>: Grindable levels ranging in difficulty from <co>Hard Demon</c> to low-end <cr>Extreme Demon</c>.\n"
    );

    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());

    return true;
}

bool GuidePopup::initNotesGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <ca>Level Notes Guide</c>\n\n"
        "Our staff team can add <ca>notes</c> to levels, pointing out details like <cc>coin</c> locations, tricky timings, difficult sections, or anything else that players should know.\n\n"
        "When a level has a <ca>note</c> attached, you can read it by clicking the button located on the left side of the level's page.\n\n"
        "Want to suggest a <ca>note</c>? Consider joining our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server!\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initOwnerRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Coordinator Role</c>\n\n"
        "<cp>Coordinators</c> are responsible for leading the <cp>Level Grind</c> project.\n"
        "They oversee all decisions, roles, and overall responsible for managing the mod."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initDeveloperRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Developer Role</c>\n\n"
        "<cg>Developers</c> are responsible for developing the <cp>Level Grind</c> project.\n"
        "They fix bugs, add new features to the mod etc."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initAdminRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cc>Admin Role</c>\n\n"
        "<cc>Admins</c> manage the <cp>Level Grind</c> <cl>database</c>.\n\n"
        "They are responsible for <cg>adding</c> and <cr>deleting</c> levels, selecting <cr>Event</c> levels, choosing <cy>Weekly Achievements</c>, and more.\n\n"
        "They also manage the <cg>Helper</c> team to ensure the project stays organized.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initHelperRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Helper Role</c>\n\n"
        "<cg>Helpers</c> are an integral part of the <cp>Level Grind</c> mod.\n\n"
        "They are responsible for suggesting levels, providing helpful <ca>Level Notes</c>, and more.\n\n"
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
        "<cp>Artists</c> are responsible for <cy>visual part of the mod</c>, including sprites, textures, and more.\n"
        "Their support is <cg>greatly appreciated <3</c>"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initContribRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Contributor Role</c>\n\n"
        "<cy>Contributors</c> are the people who have supported the project through [<cy>Boosty</c>](https://boosty.to/deliveltech) donations or by providing other meaningful help to the <cp>Level Grind</c> mod.\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initBoosterRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <ca>Booster Role</c>\n\n"
        "<ca>Boosters</c> help our [<cb>Discord</c>](https://discord.gg/Vt5gWZyaP) server by providing server <ca>boosts</c>.\n\n"
        "Their support is greatly appreciated! :3\n"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

}
