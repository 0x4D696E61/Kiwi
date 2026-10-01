#pragma once

#include "Rect.hpp"

#include <vector>

enum class LayoutDirection {
    Horizontal,
    Vertical
};

struct LayoutItem {
    Rect area;
    int size = 0;
    bool flex = false;
};

class Layout {
public:
    static std::vector<LayoutItem> split(const Rect& area, LayoutDirection direction, std::vector<LayoutItem> items);
};