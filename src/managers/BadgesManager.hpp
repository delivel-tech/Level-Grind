#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace levelgrind::badges {

enum class Kind { Daily, Weekly, Monthly, WeeklyAch, Packs, Clans };

void refresh(geode::Function<void()> onDone);

bool isNew(Kind kind);

void markSeen(Kind kind);

void markClanChatSeen(long long lastMessageId);
void markClanTasksSeen(long long weekEndsAt, int tasksDone);

void setMark(CCNode* button, bool on);

}
