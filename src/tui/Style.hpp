#pragma once

#include <optional>

enum class TuiColor {
    Default, Black, Red,
    Green, Yellow, Blue,
    Magenta, Cyan, White
};

struct Rgb {
    int r = 0;
    int g = 0;
    int b = 0;

    bool operator==(const Rgb& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

struct Style {
    TuiColor fg = TuiColor::Default;
    TuiColor bg = TuiColor::Default;

    std::optional<Rgb> fgRgb;
    std::optional<Rgb> bgRgb;

    bool bold = false;
    bool dim = false;
    bool reverse = false;
};