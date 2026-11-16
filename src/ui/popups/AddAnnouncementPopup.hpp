#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../BasePopup.hpp"
#include "Geode/ui/MDTextArea.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"
#include <cue/ListNode.hpp>

using namespace geode::prelude;

namespace levelgrind {

class AddAnnouncementPopup : public BasePopup {
public:
    static AddAnnouncementPopup* create();

    enum class LineKind { Text = 0, Heading, Subheading, Bullet, Divider };

    struct Span {
        std::string text;
        int color = 0;
        bool bold = false;
        bool italic = false;
    };

    struct Line {
        LineKind kind = LineKind::Text;
        std::vector<Span> spans;
    };

private:
    bool init() override;

    void buildToolbar(CCNode* parent, float y);
    void buildComposer(CCNode* parent, float y);

    void onAddText();
    void onNewLine();
    void onUndoPiece();
    void onClearAll();
    void onPreview();
    void onPost();

    void setKind(LineKind kind);
    void setBold(bool bold);
    void setItalic(bool italic);

    void selectLine(int index);
    void deleteLine(int index);
    void moveLine(int index, int delta);

    void refresh(bool scrollToSelected = true);
    void refreshLineList(bool scrollToSelected);
    void refreshPreview();
    std::string buildMarkdown() const;

    void saveDraft();
    void loadDraft();

    std::vector<Line> m_lines;
    int m_selected = -1;

    LineKind m_kind = LineKind::Text;
    int m_color = 0;
    bool m_bold = false;
    bool m_italic = false;

    TextInput* m_titleTextInput = nullptr;
    TextInput* m_pieceInput = nullptr;
    cue::ListNode* m_lineList = nullptr;
    CCLabelBMFont* m_emptyLabel = nullptr;
    MDTextArea* m_preview = nullptr;
    CCSprite* m_boldSpr = nullptr;
    CCSprite* m_italicSpr = nullptr;
    bool m_posting = false;

    TaskHolder<web::WebResponse> m_listener;

    ~AddAnnouncementPopup() {
        m_listener.cancel();
    }
};

}