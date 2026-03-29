#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AddNotePopup : public Popup {
public:
    static AddNotePopup* create(int levelId, std::string levelName);

private:
    bool init(int levelId, std::string levelName);

    TextInput* m_input = nullptr;
    std::string m_levelName;
    int m_levelId;

    void onAddBtn(CCObject* sender);

protected:
    async::TaskHolder<geode::utils::web::WebResponse> m_listener;
    ~AddNotePopup() { m_listener.cancel(); }
};