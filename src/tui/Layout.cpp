#include "Layout.hpp"

#include <algorithm>

std::vector<LayoutItem> Layout::split(const Rect& area, LayoutDirection direction, std::vector<LayoutItem> items) {
    const int available = direction == LayoutDirection::Horizontal ? area.w : area.h;

    int fixed = 0;
    int fCount = 0;

    for (const LayoutItem& item : items) {
        if (item.flex) {
            fCount++;
        } else {
            fixed += item.size;
        }
    }

    const int remaining = std::max(0, available - fixed);
    const int flexSize = fCount > 0 ? remaining / fCount : 0;
    int cursor = direction == LayoutDirection::Horizontal ? area.x : area.y;

    for (LayoutItem& item : items) {
        const int size = item.flex ? flexSize : item.size;

        if (direction == LayoutDirection::Horizontal) {
            item.area = {cursor, area.y, size, area.h};
        } else {
            item.area = {area.x, cursor, area.w, size};
        }

        cursor += size;
    }

    return items;
}