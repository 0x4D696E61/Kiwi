#pragma once

#include "Cell.hpp"
#include <vector>

class Screen;
class Terminal;

class TuiRenderer {
public:
    explicit TuiRenderer(Terminal& terminal);

    void render(const Screen& screen, int cursorX, int cursorY, bool showCursor);
    void invalidate();

private:
    Terminal& terminal_;

    std::vector<Cell> prev_;

    int prevW_ = 0;
    int prevH_ = 0;

    int prevCursorX_ = 0;
    int prevCursorY_ = 0;

    bool valid_ = false;
    bool curKnown_ = false;
    bool curVis_ = false;
};
