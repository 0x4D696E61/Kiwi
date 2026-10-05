#include "Screen.hpp"

#include <algorithm>

Screen::Screen(int width, int height) : width_(width), height_(height), cells_(width * height) {}

int Screen::w() const {
    return width_;
}

int Screen::h() const {
    return height_;
}

void Screen::resize(int width, int height) {
    if (width == width_ && height == height_) return;

    width_ = width;
    height_ = height;

    cells_.assign(width_ * height_, {});
}

void Screen::clear() {
    std::fill(cells_.begin(), cells_.end(), Cell{});
}

void Screen::set(int x, int y, char ch, const Style& style) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    
    Cell& cell = cells_[index(x, y)];
    
    cell.ch = ch;
    cell.style = style;
    cell.glyph.clear();
}

void Screen::text(int x, int y, const std::string& text, const Style& style) {
    for (int i = 0; i < static_cast<int>(text.length()); i++) {
        set(x + i, y, text[i], style);
    }
}

const Cell& Screen::cell(int x, int y) const {
    return cells_[index(x, y)];
}

int Screen::index(int x, int y) const {
    return y * width_ + x;
}