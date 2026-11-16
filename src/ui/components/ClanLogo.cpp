#include "ClanLogo.hpp"
#include "../../managers/APIClient.hpp"

#include <Geode/utils/web.hpp>
#include <fmt/format.h>
#include <unordered_map>

namespace levelgrind::clanlogo {

namespace {

struct Loading {
    TaskHolder<web::WebResponse> task;
    std::vector<std::shared_ptr<geode::Function<void(CCTexture2D*)>>> waiting;
};

std::unordered_map<long long, Ref<CCTexture2D>>& cache() {
    static std::unordered_map<long long, Ref<CCTexture2D>> c;
    return c;
}

std::unordered_map<long long, std::unique_ptr<Loading>>& loading() {
    static std::unordered_map<long long, std::unique_ptr<Loading>> l;
    return l;
}

}

CCTexture2D* textureFromBytes(std::vector<uint8_t> const& data) {
    if (data.empty()) return nullptr;
    auto image = new CCImage();
    if (!image->initWithImageData(const_cast<uint8_t*>(data.data()), static_cast<int>(data.size()))) {
        image->release();
        return nullptr;
    }
    auto texture = new CCTexture2D();
    bool ok = texture->initWithImage(image);
    image->release();
    if (!ok) {
        texture->release();
        return nullptr;
    }
    texture->autorelease();
    return texture;
}

void load(long long logoId, geode::Function<void(CCTexture2D*)> onLoaded) {
    if (logoId <= 0) return;
    if (auto it = cache().find(logoId); it != cache().end()) {
        onLoaded(it->second);
        return;
    }
    auto cb = std::make_shared<geode::Function<void(CCTexture2D*)>>(std::move(onLoaded));
    auto& map = loading();
    if (auto it = map.find(logoId); it != map.end()) {
        it->second->waiting.push_back(cb);
        return;
    }
    auto entry = std::make_unique<Loading>();
    entry->waiting.push_back(cb);
    auto& t = entry->task;
    map.emplace(logoId, std::move(entry));
    t.spawn(
        web::WebRequest().get(fmt::format("{}/clan_logo?id={}", APIClient::getInstance().baseUrl, logoId)),
        [logoId](web::WebResponse res) {
            auto& map = loading();
            auto it = map.find(logoId);
            if (it == map.end()) return;
            auto waiting = std::move(it->second->waiting);

            Loader::get()->queueInMainThread([logoId] { loading().erase(logoId); });

            CCTexture2D* texture = res.ok() ? textureFromBytes(res.data()) : nullptr;
            if (!texture) return;
            cache()[logoId] = texture;
            for (auto& w : waiting) {
                if (w && *w) (*w)(texture);
            }
        }
    );
}

CCNode* circle(CCTexture2D* texture, float diameter) {
    auto node = CCNode::create();
    node->setContentSize({ diameter, diameter });
    node->setAnchorPoint({ 0.5f, 0.5f });
    if (!texture) return node;

    float const r = diameter / 2.f;
    std::vector<CCPoint> pts;
    for (int i = 0; i < 48; i++) {
        float a = static_cast<float>(i) / 48.f * 6.2831853f;
        pts.push_back({ r + std::cos(a) * r, r + std::sin(a) * r });
    }
    auto stencil = CCDrawNode::create();
    stencil->drawPolygon(pts.data(), static_cast<unsigned>(pts.size()), { 1.f, 1.f, 1.f, 1.f }, 0.f, { 0.f, 0.f, 0.f, 0.f });

    auto clip = CCClippingNode::create(stencil);
    clip->setContentSize({ diameter, diameter });

    auto sprite = CCSprite::createWithTexture(texture);
    auto size = sprite->getContentSize();
    sprite->setScale(diameter / std::max(1.f, std::min(size.width, size.height)));
    sprite->setPosition({ r, r });
    clip->addChild(sprite);
    node->addChild(clip);
    return node;
}

}
