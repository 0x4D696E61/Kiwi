#include "TuiRenderer.hpp"
#include "Screen.hpp"
#include "../terminal/Terminal.hpp"

#include <iostream>
#include <string>

static bool sameStyle(const Style& a, const Style& b) {
    return a.fg == b.fg && a.bg == b.bg && a.fgRgb == b.fgRgb && a.bgRgb == b.bgRgb && a.bold == b.bold && a.dim == b.dim && a.reverse == b.reverse;
}

TuiRenderer::TuiRenderer(Terminal& terminal) : terminal_(terminal) {}

void TuiRenderer::invalidate() {
    valid_ = false;
    curKnown_ = false;
}

void TuiRenderer::render(const Screen& screen, int cursorX, int cursorY, bool showCursor) {
    const int width = screen.w();
    const int height = screen.h();

    if (width <= 0 || height <= 0) return;

    const bool full = !valid_ || width != prevW_ || height != prevH_;

    if (full) {
        prev_.assign(width * height, Cell{});
        prevW_ = width;
        prevH_ = height;
    }

    std::string out;
    out.reserve(width * height * 2);

    bool changed = false;

    for (int y = 0; y < height; y++) {
        int x = 0;

        while (x < width) {
            if (y == height - 1 && x == width - 1) break;

            const int index = y * width + x;
            const Cell& cell = screen.cell(x, y);
            const Cell& old = prev_[index];

            if (!full && cell.ch == old.ch && cell.glyph == old.glyph && sameStyle(cell.style, old.style)) {
                x++;
                continue;
            }

            if (!changed) {
                out += "\x1b[?25l";
                changed = true;
            }

            out += "\x1b[" + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H";

            Style lastStyle;
            bool hasStyle = false;

            while (x < width) {
                if (y == height - 1 && x == width - 1) break;

                const int i = y * width + x;
                const Cell& current = screen.cell(x, y);
                const Cell& previous = prev_[i];

                if (!full && current.ch == previous.ch && current.glyph == previous.glyph && sameStyle(current.style, previous.style)) break;

                const Style& style = current.style;

                if (!hasStyle || !sameStyle(style, lastStyle)) {
                    out += "\x1b[0m";

                    if (style.bold) out += "\x1b[1m";
                    if (style.dim) out += "\x1b[2m";
                    if (style.reverse) out += "\x1b[7m";

                    if (style.fgRgb) {
                        const Rgb& color = *style.fgRgb;
                        out += "\x1b[38;2;" + std::to_string(color.r) + ";" + std::to_string(color.g) + ";" + std::to_string(color.b) + "m";
                    } else if (style.fg != TuiColor::Default) {
                        out += "\x1b[" + std::to_string(30 + static_cast<int>(style.fg) - 1) + "m";
                    }

                    if (style.bgRgb) {
                        const Rgb& color = *style.bgRgb;
                        out += "\x1b[48;2;" + std::to_string(color.r) + ";" + std::to_string(color.g) + ";" + std::to_string(color.b) + "m";
                    } else if (style.bg != TuiColor::Default) {
                        out += "\x1b[" + std::to_string(40 + static_cast<int>(style.bg) - 1) + "m";
                    }

                    lastStyle = style;
                    hasStyle = true;
                }

                if (current.glyph.empty()) out += current.ch;
                else out += current.glyph;
                            
                prev_[i] = current;
                x++;
            }
        }
    }

    valid_ = true;

    if (changed) out += "\x1b[0m";

    if (showCursor) {
        if (changed || !curKnown_ || cursorX != prevCursorX_ || cursorY != prevCursorY_) {
            out += "\x1b[" + std::to_string(cursorY + 1) + ";" + std::to_string(cursorX + 1) + "H";
        }

        if (changed || !curKnown_ || !curVis_) {
            out += "\x1b[?25h";
        }
    } else if (!changed && (!curKnown_ || curVis_)) {
        out += "\x1b[?25l";
    }

    prevCursorX_ = cursorX;
    prevCursorY_ = cursorY;
    curVis_ = showCursor;
    curKnown_ = true;

    if (out.empty()) return;

    std::cout << out;
    std::cout.flush();
}