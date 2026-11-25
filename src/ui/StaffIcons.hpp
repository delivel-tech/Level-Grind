#pragma once
#include <Geode/Geode.hpp>
#include <string_view>

using namespace geode::prelude;

namespace levelgrind {

constexpr float kStaffIconScaleBoost = 0.075f;

struct StaffIconPlacement {
    CCPoint pos;
    float scale;
};

inline StaffIconPlacement staffIconPlacement(std::string_view icon) {
    auto is = [&](const char* name) { return icon.find(name) != std::string_view::npos; };
    if (is("staff_activity")) return { { 23.f, 23.5f }, 1.385f };
    if (is("reports")) return { { 23.5f, 23.5f }, 1.2f };
    if (is("find_player")) return { { 22.5f, 23.5f }, 1.2f };
    if (is("sync_levels")) return { { 25.f, 23.f }, 1.3f };
    if (is("queue_events")) return { { 23.f, 23.5f }, 1.3f };
    if (is("weekly_ach")) return { { 19.5f, 20.f }, 0.9f };
    if (is("ban_pet")) return { { 20.5f, 19.f }, 1.1f };
    if (is("reset_pet_name")) return { { 20.5f, 19.f }, 1.1f };
    if (is("clear_pet")) return { { 19.25f, 20.5f }, 1.2f };
    if (is("ban_clan")) return { { 19.5f, 20.f }, 1.f };
    if (is("lb_ban")) return { { 19.5f, 20.f }, 1.f };
    return { { 0.f, 0.f }, 0.f };
}

inline CircleButtonSprite* staffIconButton(const char* icon, bool active = false, CircleBaseSize size = CircleBaseSize::Small) {
    auto spr = CircleButtonSprite::createWithSpriteFrameName(icon, 1.f, active ? CircleBaseColor::Cyan : CircleBaseColor::Green, size);
    auto place = staffIconPlacement(icon);
    if (auto top = spr->getTopNode(); top && place.scale > 0.f) {
        top->setPosition(place.pos);
        top->setScale(place.scale + kStaffIconScaleBoost);
    }
    return spr;
}

}
