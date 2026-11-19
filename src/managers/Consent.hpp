#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind::consent {

constexpr int kVersion = 1;

bool decided();
bool granted();
void set(bool accepted);

bool guard();
void showPopup(geode::Function<void()> onAnswered = {});

void setOnChange(geode::Function<void()> callback);

extern const char* const kRules;
extern const char* const kPrivacy;
extern const char* const kTerms;

}
