#include "EditorRenderer.hpp"

#include "../editor/Editor.hpp"

#include "../buffer/Buffer.hpp"

#include "../terminal/Terminal.hpp"

#include "../tui/Painter.hpp"
#include "../tui/Theme.hpp"

#include "../syntax/Syntax.hpp"

#include <iostream>
#include <algorithm>
#include <string>

EditorRenderer::EditorRenderer(Terminal& terminal) : terminal_(terminal) {}

void EditorRenderer::render(const Buffer& buffer, const Editor& editor) {
    terminal_.hideCursor();

    const int width = terminal_.w();
    const int height = terminal_.h() - 1;

    if (width <= 0 || height <= 0) {
        return;
    }

    const Selection& sel = editor.selection();

    int startX = sel.startX;
    int startY = sel.startY;

    int endX = sel.endX;
    int endY = sel.endY;

    // Keep selection orderer even when backwards
    if (startY > endY || (startY == endY && startX > endX)) {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }

    int sY = 0;
    const auto& lines = buffer.lines();

    for (int y = 0; y < static_cast<int>(lines.size()); y++) {
        const std::string& line = lines[y];

        const int lineRows = std::max(1, (static_cast<int>(line.length()) + width - 1) / width);

        for (int row = 0; row <= lineRows; row++) {
            if (sY + row < height) {
                terminal_.clearLine(sY + row);
            }
        }

        for (int x = 0; x < static_cast<int>(line.length()); x++) {
            const int sX = x % width;
            const int charSY = sY + x / width;

            if (charSY >= height) {
                break;
            }

            terminal_.movCursor(sX, charSY);

            bool selec = false;

            if (sel.active) {
                const bool as = y > startY || (y == startY && x >= startX);

                const bool be = y < endY || (y == endY && x < endX);

                selec = as && be; // afterstart, beforend
            }

            if (selec) {
                std::cout << "\x1b[7m";
            }

            std::cout << line[x];

            if (selec) {
                std::cout << "\x1b[0m";
            }
        }

        sY += lineRows;

        if (sY >= height) {
            break;
        }
    }

    std::cout.flush();
}

void EditorRenderer::renderLine(const Buffer& buffer, const Editor& editor) {
    const int width = terminal_.w();
    const int height = terminal_.h() - 1;

    if (width <= 0 || height <= 0) {
        return;
    }

    const auto& lines = buffer.lines();
    const int y = editor.cY();

    if (y < 0 || y >= static_cast<int>(lines.size())) {
        return;
    }

    int sY = 0;

    for (int i = 0; i < y; i++) {
        const int length = static_cast<int>(lines[i].length());
        sY += std::max(1, (length + width - 1) / width);
    }

    const std::string& line = lines[y];
    const int lineRows = std::max(1, (static_cast<int>(line.length()) + width - 1) / width);

    terminal_.hideCursor();

    for (int row = 0; row <= lineRows; row++) {
        if (sY + row < height) {
            terminal_.clearLine(sY + row);
        }
    }

    for (int x = 0; x < static_cast<int>(line.length()); x++) {
        const int sX = x % width;
        const int charSY = sY + x / width;

        if (charSY >= height) {
            break;
        }

        terminal_.movCursor(sX, charSY);
        std::cout << line[x];
    }

    std::cout.flush();
}

void EditorRenderer::renderFromLine(const Buffer& buffer, const Editor& editor, int startY) {
    const int width = terminal_.w();
    const int height = terminal_.h() - 1;

    if (width <= 0 || height <= 0) {
        return;
    }

    const auto& lines = buffer.lines();

    if (startY < 0 || startY >= static_cast<int>(lines.size())) {
        return;
    }

    int sY = 0;

    for (int y = 0; y < startY; y++) {
        const int length = static_cast<int>(lines[y].length());
        sY += std::max(1, (length + width - 1) / width);
    }

    terminal_.hideCursor();

    for (int row = sY; row < height; row++) {
        terminal_.clearLine(row);
    }

    const Selection& sel = editor.selection();

    int startX = sel.startX;
    int selecStartY = sel.startY;

    int endX = sel.endX;
    int endY = sel.endY;

    // Keep selection orderer even when backwards
    if (selecStartY > endY || (selecStartY == endY && startX > endX)) {
        std::swap(startX, endX);
        std::swap(selecStartY, endY);
    }

    for (int y = startY; y < static_cast<int>(lines.size()); y++) {
        const std::string& line = lines[y];
        const int lineRows = std::max(1, (static_cast<int>(line.length()) + width - 1) / width);

        for (int x = 0; x < static_cast<int>(line.length()); x++) {
            const int sX = x % width;
            const int charSY = sY + x / width;

            if (charSY >= height) {
                break;
            }

            terminal_.movCursor(sX, charSY);

            bool selec = false;

            if (sel.active) {
                const bool as = y > selecStartY || (y == selecStartY && x >= startX);

                const bool be = y < endY || (y == endY && x < endX);

                selec = as && be; // as, beforend
            }

            if (selec) {
                std::cout << "\x1b[7m";
            }

            std::cout << line[x];

            if (selec) {
                std::cout << "\x1b[0m";
            }
        }

        sY += lineRows;

        if (sY >= height) {
            break;
        }
    }

    std::cout.flush();
}

void EditorRenderer::placeCursor(const Buffer& buffer, const Editor& editor) {
    const int width = terminal_.w();

    if (width <= 0) {
        return;
    }

    int sY = 0; // screenY

    const auto& lines = buffer.lines();

    for (int y = 0; y < editor.cY(); y++) {
        const int length = static_cast<int>(lines[y].length());

        sY += std::max(1, (length + width - 1) / width);
    }

    const int sX = editor.cX() % width; // screenX
    sY += editor.cX() / width;

    terminal_.movCursor(sX, sY);
    terminal_.showCursor();
}

void EditorRenderer::renderStatus(const Buffer& buffer, const Editor& editor) {
    const int width = terminal_.w();
    const int y = terminal_.h() - 1;

    if (width <= 0) {
        return;
    }

    terminal_.clearLine(y);

    const std::string left =
        buffer.path().filename().string() +
        (buffer.modified() ? " [+]" : "");

    const std::string mode =
        editor.isTypeMode() ? "EDIT" : "NAV"; // TYPE and MOVE are Internal in Kiwi, user exposed variables are EDIT, NAV

    const std::string right =
        "Ln " + std::to_string(editor.cY() + 1) +
        ", Col " + std::to_string(editor.cX() + 1);

    terminal_.movCursor(0, y);
    std::cout << left;

    const int modeX = (width - static_cast<int>(mode.length())) / 2;

    if (modeX >= 0) {
        terminal_.movCursor(modeX, y);
        std::cout << mode;
    }

    const int rightX = width - static_cast<int>(right.length());

    if (rightX >= 0) {
        terminal_.movCursor(rightX, y);
        std::cout << right;
    }

    std::cout.flush();
}

void EditorRenderer::renderCmdBar(const std::string& text) {
    const int y = terminal_.h() - 1;

    terminal_.clearLine(y);
    terminal_.movCursor(0, y);

    std::cout << "> " << text;
    std::cout.flush();
}

void EditorRenderer::renderMessage(const std::string& message) {
    const int y = terminal_.h() -1;

    terminal_.clearLine(y);
    terminal_.movCursor(0, y);

    std::cout << message;
    std::cout.flush();
}


int EditorRenderer::gutterWidth(const Buffer& buffer) const {
    int digits = 1;
    int count = std::max(1, static_cast<int>(buffer.lines().size()));

    while (count >= 10) {
        count /= 10;
        digits++;
    }

    return digits + 3;
}


void EditorRenderer::renderTui(const Buffer& buffer, const Editor& editor, Painter& painter, int scrollX, int scrollY) {
    painter.fill();

    const auto& lines = buffer.lines();
    const Selection& sel = editor.selection();

    const int gutter = gutterWidth(buffer);
    const int textW = std::max(0, painter.w() - gutter);

    int startX = sel.startX;
    int startY = sel.startY;
    int endX = sel.endX;
    int endY = sel.endY;

    if (startY > endY || (startY == endY && startX > endX)) {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }

    Style numbers;
    numbers.fgRgb = kiwiTheme.lineNum;

    Style currNum;
    currNum.fgRgb = kiwiTheme.currLineNum;
    currNum.bold = true;

    SyntaxState syntax;
    
    std::string ext = buffer.path().extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    
    for (int i = 0; i < std::min(scrollY, static_cast<int>(lines.size())); i++) highlightLine(lines[i], ext, syntax);

    for (int y = 0; y < painter.h(); y++) {
        const int lineY = scrollY + y;

        if (lineY < 0 || lineY >= static_cast<int>(lines.size())) break;

        const std::string& line = lines[lineY];
        const auto colors = highlightLine(line, ext, syntax);
        const std::string number = std::to_string(lineY + 1);
        const std::string padding(std::max(0, gutter - 3 - static_cast<int>(number.length())), ' ');

        painter.text(0, y, padding + number + "   ", lineY == editor.cY() ? currNum : numbers);

        const bool lineSelected =
            sel.active &&
            editor.isLineSelec() &&
            lineY >= std::min(sel.startY, sel.endY) &&
            lineY <= std::max(sel.startY, sel.endY);

        for (int x = 0; x < textW; x++) {
            const int lineX = scrollX + x;

            if (lineX < 0) continue;
            if (lineX >= static_cast<int>(line.length()) && !lineSelected) break;

            bool selec = lineSelected;

            if (sel.active && !editor.isLineSelec()) {
                const bool as = lineY > startY || (lineY == startY && lineX >= startX); //afterstart
                const bool be = lineY < endY || (lineY == endY && lineX < endX); //before end

                selec = as && be;
            }

            const char character = lineX < static_cast<int>(line.length()) ? line[lineX] : ' ';

            Style style;
            style.fgRgb = lineX < static_cast<int>(colors.size()) ? colors[lineX] : kiwiTheme.file;
            if (selec) style.bgRgb = kiwiTheme.selec;
                    
            painter.set(gutter + x, y, character, style);
        }
    }
}
