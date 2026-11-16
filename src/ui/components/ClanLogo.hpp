#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace levelgrind::clanlogo {
    
void load(long long logoId, geode::Function<void(CCTexture2D*)> onLoaded);

CCTexture2D* textureFromBytes(std::vector<uint8_t> const& data);

CCNode* circle(CCTexture2D* texture, float diameter);

}
