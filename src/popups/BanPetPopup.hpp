#include <Geode/Geode.hpp>

using namespace geode::prelude;

class BanPetPopup : public Popup {
public:
    static BanPetPopup* create(int accountId);

private:
    bool init(int accountId);

    TextInput* m_input = nullptr;
    int m_targetAccountId;

    void onBanBtn(CCObject* sender);

protected:
    async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    ~BanPetPopup() { m_listener.cancel(); }
};