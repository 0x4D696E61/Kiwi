#pragma once

#include "Style.hpp"
#include <string>

struct Cell {
    char ch = ' ';
    Style style;
    std::string glyph;
};