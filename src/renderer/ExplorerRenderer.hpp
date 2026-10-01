#pragma once

class Explorer;
class Painter;

class ExplorerRenderer {
public:
    void render(const Explorer& explorer, Painter& painter, int scroll, bool active);
};