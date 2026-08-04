#include "Geode/c++stl/string.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;

namespace levelgrind {

class PackBrowserLayer : public LevelBrowserLayer {
public:
    static PackBrowserLayer* create(std::string name, GJSearchObject* search) {
        auto ret = new PackBrowserLayer;
        if (ret && ret->init(name, search)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    std::string m_name;

private:
    void updateTitle() {
        if (auto node = this->getChildByIDRecursive("title")) {
            if (auto label = typeinfo_cast<CCLabelBMFont*>(node)) {
                label->setString(m_name.c_str());
            };
        }
    }
    bool init(std::string name, GJSearchObject* search) {
        if (!LevelBrowserLayer::init(search)) return false;

        m_name = name;
        this->updateTitle();

        auto refreshMenu = this->getChildByIDRecursive("refresh-menu");
        if (!refreshMenu) return true;

        refreshMenu->setVisible(false);

        auto nextPageMenu = this->getChildByIDRecursive("next-page-menu");
        if (!nextPageMenu) return true;

        nextPageMenu->setVisible(false);

        auto pageMenu = this->getChildByIDRecursive("page-menu");
        if (!pageMenu) return true;

        pageMenu->setVisible(false);

        return true;
    }
};

class $modify(GrindPackBrowserLayer, LevelBrowserLayer) {
public:
    gd::string getSearchTitle() {
        if (auto it = typeinfo_cast<PackBrowserLayer*>(this)) {
            return gd::string(it->m_name);
        }
        return LevelBrowserLayer::getSearchTitle();
    }
};

}