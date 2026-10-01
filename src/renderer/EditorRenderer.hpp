#pragma once

#include <string>

class Buffer;
class Editor;
class Terminal;
class Painter;

class EditorRenderer {
public:
    explicit EditorRenderer(Terminal& terminal);

    void render(const Buffer& buffer, const Editor& editor);
    void renderLine(const Buffer& buffer, const Editor& editor);
    void renderFromLine(const Buffer& buffer, const Editor& editor, int startY);

    void placeCursor(const Buffer& buffer, const Editor& editor);
    
    void renderStatus(const Buffer& buffer, const Editor& editor);
    void renderCmdBar(const std::string& text);
    void renderMessage(const std::string& message);

    int gutterWidth(const Buffer& buffer) const;
    void renderTui(const Buffer& buffer, const Editor& editor, Painter& painter, int scrollX, int scrollY);

private:
    Terminal& terminal_;
};