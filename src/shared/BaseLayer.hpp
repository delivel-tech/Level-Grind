#pragma once
#include <Geode/Geode.hpp>
#include <cue/RepeatingBackground.hpp>

using namespace geode::prelude;

namespace levelgrind {

class BaseLayer : public CCLayer {
public:
    static BaseLayer* create();
    void open();
    CCSize m_winSize;
    CCMenuItemSpriteExtra* m_backBtn = nullptr;
    cue::RepeatingBackground* m_bg = nullptr;

    void replaceBgToClassic();

private:
    void keyBackClicked() override;
    virtual void onBack(CCObject* sender);

protected:
    bool init() override;
    /*
        A bunch of different positioning helpers to simplify making UI.
        Thanks to dankmeme for this part of code, taken from Globed GitHub repo.
    */
    float left() { return 0.f; }
    float right() { return this->getContentSize().width; }
    float top() { return this->getContentSize().height; }
    float bottom() { return 0.f; }
    float centerX() { return this->right() / 2.f; }
    float centerY() { return this->top() / 2.f; }
    CCPoint center() { return { this->centerX(), this->centerY() }; }
    CCPoint centerLeft() { return { this->left(), this->centerY() }; }
    CCPoint centerRight() { return { this->right(), this->centerY() }; }
    CCPoint centerTop() { return { this->centerX(), this->top() }; }
    CCPoint centerBottom() { return { this->centerX(), this->bottom() }; }
    CCPoint bottomLeft() { return { this->left(), this->bottom() }; }
    CCPoint bottomRight() { return { this->right(), this->bottom() }; }
    CCPoint topLeft() { return { this->left(), this->top() }; }
    CCPoint topRight() { return { this->right(), this->top() }; }

    CCPoint fromTop(float y) { return fromTop(CCSize{0.f, y}); }
    CCPoint fromTop(CCSize off) {
        return this->centerTop() + CCPoint{off.width, -off.height};
    }

    CCPoint fromBottom(float y) { return fromBottom(CCSize{0.f, y}); }
    CCPoint fromBottom(CCSize off) {
        return this->centerBottom() + off;
    }

    CCPoint fromLeft(float x) { return fromLeft(CCSize{x, 0.f}); }
    CCPoint fromLeft(CCSize off) {
        return this->centerLeft() + off;
    }

    CCPoint fromRight(float x) { return fromRight(CCSize{x, 0.f}); }
    CCPoint fromRight(CCSize off) {
        return this->centerRight() + CCPoint{-off.width, off.height};
    }

    CCPoint fromCenter(float x, float y) { return fromCenter({x, y}); }
    CCPoint fromCenter(CCSize off) {
        return this->center() + off;
    }

    CCPoint fromBottomRight(float x, float y) { return fromBottomRight({x, y}); }
    CCPoint fromBottomRight(CCSize off) {
        return this->bottomRight() + CCPoint{-off.width, off.height};
    }

    CCPoint fromTopRight(float x, float y) { return fromTopRight({x, y}); }
    CCPoint fromTopRight(CCSize off) {
        return this->topRight() - off;
    }

    CCPoint fromBottomLeft(float x, float y) { return fromBottomLeft({x, y}); }
    CCPoint fromBottomLeft(CCSize off) {
        return this->bottomLeft() + off;
    }

    CCPoint fromTopLeft(float x, float y) { return fromTopLeft({x, y}); }
    CCPoint fromTopLeft(CCSize off) {
        return this->topLeft() + CCPoint{off.width, -off.height};
    }
};

}