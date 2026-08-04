#pragma once

#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/LoadingSpinner.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/CCBlockLayer.hpp>

using namespace geode::prelude;

namespace levelgrind {

// made by delivel xDD

class LoadingLayer : public CCLayer {
public:
    static LoadingLayer* create(std::string loadingLabel);
    void show();
    void hide();

private:
    bool init(std::string loadingLabel);
    void keyBackClicked() override;
    void onHideFinished();

    CCBlockLayer* m_blockLayer = nullptr;
    LoadingSpinner* m_spinner = nullptr;
    CCLabelBMFont* m_label = nullptr;
};

}