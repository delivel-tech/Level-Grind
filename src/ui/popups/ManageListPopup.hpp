#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/binding/GJLevelList.hpp>
#include "../BasePopup.hpp"

using namespace geode::prelude;

namespace levelgrind {

class ManageListPopup : public BasePopup {
public:
    static ManageListPopup* create(GJLevelList* list, CCArray* levels);
private:
    bool init(GJLevelList* list, CCArray* levels);
    void collect(CCArray* levels);
    void loadInfo();
    void setAdded(bool added, std::string const& by, long long at);
    void onAdd();
    void onRemove();
    matjson::Value listBody();
    void buildTags();

    Ref<GJLevelList> m_list;
    struct Level { int id = 0; int stars = 0; bool moon = false; bool demon = false; int coins = 0; bool loaded = false; };
    std::vector<Level> m_levels;
    int m_loaded = 0;
    bool m_added = false;
    int m_tags = 0;
    CCMenu* m_tagMenu = nullptr;

    geode::Label* m_status = nullptr;
    CCMenu* m_actions = nullptr;
    TaskHolder<web::WebResponse> m_task;
    ~ManageListPopup() { m_task.cancel(); }
};

}
