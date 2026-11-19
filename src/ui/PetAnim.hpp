#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind::petanim {

constexpr int kMoodTag = 7101;

inline bool enabled() {
    return !Mod::get()->getSavedValue<bool>("disable-pet-animations", false);
}

inline void stopMood(CCNode* node) {
    if (!node) return;
    node->stopActionByTag(kMoodTag);
}

inline void playMood(CCNode* node, int mood) {
    if (!node || !enabled()) return;
    stopMood(node);
    float const s = node->getScale();
    CCAction* action = nullptr;
    if (mood >= 2) {
        auto hop = CCSequence::create(
            CCEaseSineOut::create(CCScaleTo::create(0.08f, s * 1.12f, s * 0.86f)),
            CCSpawn::create(
                CCJumpBy::create(0.42f, { 0.f, 0.f }, 9.f, 1),
                CCSequence::create(
                    CCEaseSineOut::create(CCScaleTo::create(0.14f, s * 0.9f, s * 1.12f)),
                    CCEaseSineInOut::create(CCScaleTo::create(0.28f, s, s)),
                    nullptr
                ),
                nullptr
            ),
            CCEaseSineOut::create(CCScaleTo::create(0.07f, s * 1.1f, s * 0.9f)),
            CCEaseBackOut::create(CCScaleTo::create(0.2f, s, s)),
            nullptr
        );
        auto doubleHop = CCSequence::create(
            static_cast<CCFiniteTimeAction*>(hop->copy()->autorelease()),
            static_cast<CCFiniteTimeAction*>(hop->copy()->autorelease()),
            nullptr
        );
        auto spin = CCSequence::create(
            CCSpawn::create(
                CCJumpBy::create(0.55f, { 0.f, 0.f }, 14.f, 1),
                CCEaseSineInOut::create(CCRotateBy::create(0.55f, 360.f)),
                nullptr
            ),
            CCEaseBackOut::create(CCScaleTo::create(0.15f, s, s)),
            nullptr
        );
        action = CCRepeatForever::create(CCSequence::create(
            CCDelayTime::create(1.1f), hop,
            CCDelayTime::create(1.6f), doubleHop,
            CCDelayTime::create(2.2f), hop,
            CCDelayTime::create(1.8f), spin,
            nullptr
        ));
    } else if (mood == 1) {
        auto breathe = CCSequence::create(
            CCEaseSineInOut::create(CCScaleTo::create(1.1f, s * 1.02f, s * 1.05f)),
            CCEaseSineInOut::create(CCScaleTo::create(1.1f, s, s)),
            nullptr
        );
        auto blink = CCSequence::create(
            CCScaleTo::create(0.06f, s * 1.06f, s * 0.9f),
            CCEaseBackOut::create(CCScaleTo::create(0.18f, s, s)),
            nullptr
        );
        action = CCRepeatForever::create(CCSequence::create(
            breathe, static_cast<CCFiniteTimeAction*>(breathe->copy()->autorelease()), blink, CCDelayTime::create(0.4f), nullptr
        ));
    } else {
        float const low = s * 0.94f;
        node->setScaleY(low);
        auto drift = CCSequence::create(
            CCSpawn::create(
                CCEaseSineInOut::create(CCMoveBy::create(1.8f, { 0.f, 5.f })),
                CCEaseSineInOut::create(CCRotateTo::create(1.8f, -5.f)),
                nullptr
            ),
            CCSpawn::create(
                CCEaseSineInOut::create(CCMoveBy::create(1.8f, { 0.f, -5.f })),
                CCEaseSineInOut::create(CCRotateTo::create(1.8f, 5.f)),
                nullptr
            ),
            nullptr
        );
        auto shake = CCSequence::create(
            CCRotateTo::create(0.07f, -9.f), CCRotateTo::create(0.07f, 9.f),
            CCRotateTo::create(0.07f, -6.f), CCRotateTo::create(0.07f, 0.f),
            nullptr
        );
        action = CCRepeatForever::create(CCSequence::create(
            drift, static_cast<CCFiniteTimeAction*>(drift->copy()->autorelease()), shake, nullptr
        ));
    }
    action->setTag(kMoodTag);
    node->runAction(action);
}

inline void jiggle(CCNode* node, float baseScale) {
    if (!node || !enabled()) return;
    float const s = baseScale;
    node->runAction(CCSequence::create(
        CCScaleTo::create(0.06f, s * 1.18f, s * 0.82f),
        CCScaleTo::create(0.08f, s * 0.88f, s * 1.14f),
        CCScaleTo::create(0.07f, s * 1.08f, s * 0.94f),
        CCEaseBackOut::create(CCScaleTo::create(0.2f, s, s)),
        nullptr
    ));
}

inline void gulp(CCNode* node, float baseScale) {
    if (!node || !enabled()) return;
    float const s = baseScale;
    node->runAction(CCSequence::create(
        CCEaseSineOut::create(CCScaleTo::create(0.1f, s * 1.25f, s * 0.75f)),
        CCEaseSineOut::create(CCScaleTo::create(0.12f, s * 0.82f, s * 1.25f)),
        CCSpawn::create(
            CCJumpBy::create(0.4f, { 0.f, 0.f }, 12.f, 1),
            CCEaseBackOut::create(CCScaleTo::create(0.4f, s, s)),
            nullptr
        ),
        CCScaleTo::create(0.06f, s * 1.1f, s * 0.9f),
        CCEaseBackOut::create(CCScaleTo::create(0.15f, s, s)),
        nullptr
    ));
}

inline void pop(CCNode* node, float baseScale, bool big) {
    if (!node || !enabled()) return;
    float const s = baseScale;
    float const peak = big ? 1.45f : 1.2f;
    node->runAction(CCSequence::create(
        CCScaleTo::create(0.08f, s * 0.8f, s * 0.8f),
        CCEaseBackOut::create(CCScaleTo::create(0.25f, s * peak, s * peak)),
        CCEaseSineInOut::create(CCScaleTo::create(0.35f, s, s)),
        nullptr
    ));
    if (big) node->runAction(CCSequence::create(CCEaseSineInOut::create(CCRotateBy::create(0.6f, 360.f)), nullptr));
}

inline void burst(CCNode* parent, CCPoint pos, ccColor3B color, int count) {
    if (!parent || !enabled()) return;
    for (int i = 0; i < count; i++) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        spr->setColor(color);
        spr->setScale(0.35f + 0.25f * static_cast<float>(i % 3) / 2.f);
        spr->setPosition(pos);
        parent->addChild(spr, 20);
        float angle = 6.2831853f * static_cast<float>(i) / static_cast<float>(count);
        float dist = 45.f + static_cast<float>((i * 37) % 25);
        spr->runAction(CCSequence::create(
            CCSpawn::create(
                CCEaseSineOut::create(CCMoveBy::create(0.6f, { std::cos(angle) * dist, std::sin(angle) * dist })),
                CCRotateBy::create(0.6f, 180.f),
                CCFadeOut::create(0.6f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
}

inline void hearts(CCNode* parent, CCPoint pos) {
    if (!parent || !enabled()) return;
    for (int i = 0; i < 3; i++) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
        spr->setScale(0.4f);
        spr->setColor({ 255, 110, 160 });
        spr->setOpacity(0);
        spr->setPosition(pos + CCPoint{ (i - 1) * 12.f, 0.f });
        parent->addChild(spr, 20);
        spr->runAction(CCSequence::create(
            CCDelayTime::create(0.12f * i),
            CCSpawn::create(
                CCFadeIn::create(0.15f),
                CCEaseSineOut::create(CCMoveBy::create(0.9f, { (i - 1) * 6.f, 28.f })),
                nullptr
            ),
            CCFadeOut::create(0.3f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
}

}
