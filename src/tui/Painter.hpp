#pragma once

#include "Rect.hpp"
#include "Screen.hpp"

#include <string>

class Painter {
public:
    Painter(Screen& screen, const Rect& area);

    int w() const;
    int h() const;

    void set(int x, int y, char ch, const Style& style = {});
    void text(int x, int y, const std::string& text, const Style& style = {});
    void fill(char ch = ' ', const Style& style = {});

private:
    Screen& screen_;
    Rect area_;
};