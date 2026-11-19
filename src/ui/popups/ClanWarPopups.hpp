#pragma once
#include <Geode/Geode.hpp>
#include "../BasePopup.hpp"
#include "../../managers/GrindClansManager.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ClanWarPopup : public BasePopup {
public:
    static ClanWarPopup* create(ClanDetails const& d, long long now);

private:
    bool init(ClanDetails const& d, long long now);
};

class ClanTreasuryPopup : public BasePopup {
public:
    static ClanTreasuryPopup* create(ClanDetails const& d, GPWallet const& wallet, long long now, geode::Function<void()> onChanged);

private:
    bool init(ClanDetails const& d, GPWallet const& wallet, long long now, geode::Function<void()> onChanged);
    void donate(int amount);
    void buy(std::string const& item, int cost);

    ClanDetails m_details;
    GPWallet m_wallet;
    geode::Function<void()> m_onChanged;
    CCNode* m_root = nullptr;
    long long m_now = 0;
    void build();
};

}
