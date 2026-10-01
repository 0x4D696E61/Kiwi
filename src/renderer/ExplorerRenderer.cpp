
#include "ExplorerRenderer.hpp"
#include "../explorer/Explorer.hpp"
#include "../tui/Painter.hpp"
#include "../tui/Theme.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>

static std::string displayPath(const std::filesystem::path& root, int width) {
    if (width <= 0) return "";

    std::string path = root.string();
    const char* user = std::getenv("USERPROFILE");

    if (user) {
        const std::filesystem::path home(user);
        const auto relative = root.lexically_relative(home);

        if (root == home) {
            path = "~";
        } else if (!relative.empty() && relative.begin()->string() != "..") {
            path = "~\\" + relative.string();
        }
    }

    if (static_cast<int>(path.size()) <= width) return path;

    if (path.rfind("~\\", 0) == 0) {
        path = "~\\" + root.filename().string();
        if (static_cast<int>(path.size()) <= width) return path;
    }

    if (width <= 3) return path.substr(path.size() - width);
    return "..." + path.substr(path.size() - (width - 3));
}

void ExplorerRenderer::render(const Explorer& explorer, Painter& painter, int scroll, bool active) {
    painter.fill();

    const int width = std::max(0, painter.w() - 2);

    Style pathStyle;
    pathStyle.fgRgb = kiwiTheme.path;

    painter.text(1, 0, displayPath(explorer.root(), std::max(0, width - 1)), pathStyle);

    const auto& entries = explorer.entries();

    for (int y = 1; y < painter.h(); y++) {
        const int index = scroll + y - 1;
        if (index >= static_cast<int>(entries.size())) break;

        const auto& entry = entries[index];
        const bool selected = index == explorer.selected();

        Style style;
        style.fgRgb = entry.dir ? kiwiTheme.folder : kiwiTheme.file;

        if (selected && active) {
            style.bgRgb = kiwiTheme.selec;
            painter.text(0, y, std::string(width, ' '), style);
        }

        const int indent = explorer.tree() ? entry.depth * 2 : 0;
        const std::string prefix = entry.dir ? (explorer.tree() && entry.expanded ? "v " : "> ") : "  ";
        const std::string label = std::string(indent, ' ') + prefix + entry.name;

        if (width > 1) {
            painter.text(1, y, label.substr(0, width - 1), style);
        }
    }
}
