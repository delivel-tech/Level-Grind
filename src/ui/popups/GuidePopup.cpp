#include "GuidePopup.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/platform/windows.hpp"
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
            m_mainLayer->getContentHeight() / 1.2f + 10.f
        )
        .parent(m_mainLayer)
        .scale(0.9f)
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
        case GuidePage::WeeklyAchievementsGuide:
            return initWeeklyAchievementsGuidePage();
        case GuidePage::NotesGuide:
            return initNotesGuidePage();
        case GuidePage::OwnerRoleGuide:
            return initOwnerRoleGuidePage();
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
        case GuidePage::RandomButtonGuide:
            return initRandomButtonGuidePage();
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
    listNode->addCell(createGuideSection("Weekly Achievements", [this] { initPage(GuidePage::WeeklyAchievementsGuide); }, 3));
    listNode->addCell(createGuideSection("Notes Info", [this] { initPage(GuidePage::NotesGuide); }, 4));
    listNode->addCell(createGuideSection("Random Button", [this] { initPage(GuidePage::RandomButtonGuide); }, 11));
    listNode->addCell(createGuideSection("Owner Role", [this] { initPage(GuidePage::OwnerRoleGuide); }, 5));
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
        "<cp>Level Grind</c> is a mod that allows you to search for <cy>grinding-worthy levels</c> based on various criteria, such as <cg>length</c>, <cj>difficulty</c>, and <co>type</c>. " \
        "It also offers additional <cl>filters</c> and options to help you find the perfect level to grind.\n\n" \
        "<ca>Searching options explanations</c>:\n\n" \
        "<cy>Star</c>: searching for classic levels.\n\n" \
        "<cp>Moon</c>: searching for platformer levels.\n\n" \
        "<co>Coin</c>: searching for levels with easy or free coins.\n\n" \
        "<cr>Demon</c>: searching for demon levels.\n\n" \
        "Other settings are quite self-explanatory, but if you need more info, feel free to join our [<cp>Discord server</c>](https://discord.gg/tmf5xtCX5y) by clicking the Discord button at the bottom right corner of this menu!"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initPetGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Grinding Pet Guide</c>\n\n"
        "<cg>Grinding Pet</c> is your own <cj>companion</c> that evolves when you <cy>grind stats</c>!\n\n"
        "## <cp>How does it work?</c>\n\n"
        "- <cg>Currencies</c>: Earn <cy>Pet Stars and Pet Moons</c> by <cl>playing levels and grinding stats</c>.\n"
        "- <cy>Evolution</c>: There are <cr>30 levels</c> to achieve. With every level-up, you <cg>pet grows larger and may change its style!</c>\n"
        "- <cj>The Pet Shop</c>: Once your pet reaches <cy>Level 15</c>, you unlock the <cr>Pet Shop</c>.\n"
        "- <cr>Rarity Bonus</c>: Higher pet rarity <cg>grants you discounts</c> on all items in the Pet Shop!"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initWeeklyAchievementsGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Weekly Achievements Guide</c>\n\n"
        "<cp>Highlighting major achievements</c> from the grinding community!\n"
        "Each week, a <cg>new accomplishment</c> will be featured here!\n"
        "Weekly achievements are chosen by <cr>Grind Admins</c>"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initNotesGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Notes Guide</c>\n\n"
        "<cy>Helpers</c> can <cj>attach notes</c> to levels that help players by <co>pointing out coin locations, tricky timings, difficult sections</c>, or anything else worth noting."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initOwnerRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cp>Owner Role</c>\n\n"
        "<cp>Owners</c> are <cy>responsible for</c> leading the <cg>Level Grind</c> project.\n"
        "They manage <cr>development</c>, make <cj>final decisions</c>, and oversee all roles."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initAdminRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cy>Admin Role</c>\n\n"
        "<cy>Admins</c> have <cg>additional permissions</c> for locking levels, setting dailies / weeklies, and more.\n"
        "They are also responsible for <cp>managing the helper team</c> to ensure the project stays organized."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initHelperRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cg>Helper Role</c>\n\n"
        "<cg>Helpers</c> manage the <cj>Level Grind database</c>.\n"
        "They are responsible for <cy>adding and deleting levels</c>, as well as providing helpful level notes."
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
        "We <cg>greatly appreciate their work</c>!"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initContribRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cb>Contributor Role</c>\n\n"
        "<cb>Contributors</c> are the people who have supported the project through <cy>Boosty</c> donations "
        "or providing other meaningful help to the <cg>Level Grind mod</c>."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initBoosterRoleGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cb>Booster Role</c>\n\n"
        "<cb>Boosters</c> help our <cp>Discord server</c> by providing server boosts.\n"
        "Their support <cg>is greatly appreciated</c>!"
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

bool GuidePopup::initRandomButtonGuidePage() {
    addReturnButton();
    addGrindLogo();
    addTextArea(
        "# <cl>Random Button</c>\n\n"
        "<cl>Random Button</c> is made to search for a level selected based on your currently active filters."
    );
    addScrollbar(typeinfo_cast<geode::MDTextArea*>(getChildByIDRecursive("md-text-area"))->getScrollLayer());
    
    return true;
}

}