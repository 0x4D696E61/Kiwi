#pragma once

#include "Cell.hpp"
#include "Rect.hpp"

#include <string>
#include <vector>

class Screen {
public:
    Screen(int width, int height);

    int w() const;
    int h() const;

    void resize(int width, int height);
    void clear();

    void set(int x, int y, char ch, const Style& style = {});
    void text(int x, int y, const std::string& text, const Style& style = {});

    void glyph(int x, int y, const std::string& value, const Style& style = {}) {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        
        Cell& cell = cells_[index(x, y)];
        cell.ch = ' ';
        cell.style = style;
        cell.glyph = value;
    }
    
    void clearGlyphs() {
        for (Cell& cell : cells_) cell.glyph.clear();
    }

    const Cell& cell(int x, int y) const;

private:
    int width_;
    int height_;

    std::vector<Cell> cells_;

    int index(int x, int y) const;
};