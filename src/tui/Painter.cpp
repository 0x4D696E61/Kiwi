#include "Painter.hpp"

Painter::Painter(Screen& screen, const Rect& area) : screen_(screen), area_(area) {}

int Painter::w() const {
    return area_.w;
}

int Painter::h() const {
    return area_.h;
}

void Painter::set(int x, int y, char ch, const Style& style) {
    if (x < 0 || y < 0 || x >= area_.w || y >= area_.h) {
        return;
    }

    screen_.set(area_.x + x, area_.y + y, ch, style);
}

void Painter::text(int x, int y, const std::string& text, const Style& style) {
    if (y < 0 || y >= area_.h) {
        return;
    }

    for (int i = 0; i < static_cast<int>(text.length()) && x + i < area_.w; i++) {
        if (x + i >= 0) {
            set(x + i, y, text[i], style);
        }
    }
}

void Painter::fill(char ch, const Style& style) {
    for (int y = 0; y < area_.h; y++) {
        for (int x = 0; x < area_.w; x++) {
            set(x, y, ch, style);
        }
    }
}