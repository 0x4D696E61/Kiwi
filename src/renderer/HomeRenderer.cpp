#include "HomeRenderer.hpp"
#include "../terminal/Terminal.hpp"
#include "Version.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

HomeRenderer::HomeRenderer(Terminal& terminal) : terminal_(terminal) {
    loadRecent();
}

void HomeRenderer::loadRecent() {
    const char* appdata = std::getenv("APPDATA");
    const auto base = appdata ? std::filesystem::path(appdata) : std::filesystem::current_path();

    std::ifstream file(base / "Kiwi" / "recent.txt");
    std::string line;

    while (std::getline(file, line) && recent_.size() < 9) {
        if (line.empty()) continue;

        const std::filesystem::path path(line);
        std::error_code ec;

        if (!std::filesystem::exists(path, ec)) continue;
        if (std::find(recent_.begin(), recent_.end(), path) != recent_.end()) continue;

        recent_.push_back(path);
    }
}

void HomeRenderer::savRecent() const {
    const char* appdata = std::getenv("APPDATA");
    const auto base = appdata ? std::filesystem::path(appdata) : std::filesystem::current_path();
    const auto path = base / "Kiwi" / "recent.txt";

    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) return;

    std::ofstream file(path, std::ios::trunc);
    if (!file) return;

    for (const auto& entry : recent_) file << entry.string() << '\n';
}

void HomeRenderer::addRecent(const std::filesystem::path& path) {
    if (path.empty()) return;

    std::error_code ec;
    const auto absolute = std::filesystem::absolute(path, ec);
    const auto normalized = (ec ? path : absolute).lexically_normal();

    recent_.erase(std::remove(recent_.begin(), recent_.end(), normalized), recent_.end());
    recent_.insert(recent_.begin(), normalized);

    if (recent_.size() > 9) recent_.resize(9);

    savRecent();
}

std::filesystem::path HomeRenderer::recent(int index) const {
    if (index < 0 || index >= static_cast<int>(recent_.size())) return {};
    return recent_[index];
}

void HomeRenderer::render(int offset) {
    const int width = terminal_.w() - offset;
    if (width <= 0) return;

    const int height = terminal_.h();
    const int panelWidth = std::min(76, std::max(20, width - 4));
    const int left = (width - panelWidth) / 2;
    const int top = std::max(1, (height - 27) / 2);

    const std::string reset = "\x1b[0m";
    const std::string blue = "\x1b[38;2;143;185;215m";
    const std::string green = "\x1b[38;2;165;198;151m";
    const std::string purple = "\x1b[38;2;190;156;220m";
    const std::string orange = "\x1b[38;2;222;176;127m";
    const std::string white = "\x1b[38;2;220;224;232m";
    const std::string muted = "\x1b[38;2;145;151;166m";

    auto draw = [&](int x, int y, const std::string& value, const std::string& color) {
        if (y < 0 || y >= height || x >= width) return;

        x = std::max(0, x);
        const int available = width - x;
        if (available <= 0) return;

        terminal_.movCursor(offset + x, y);
        std::cout << color << value.substr(0, available) << reset;
    };

    auto center = [&](int y, const std::string& value, const std::string& color) {
        draw(std::max(0, (width - static_cast<int>(value.size())) / 2), y, value, color);
    };

    if (offset == 0) terminal_.clear();

    center(top, "KIWI", blue);
    center(top + 3, "v" + std::string(kiwiversion) + " by 0x4D696E61 (Github)", muted);

    const std::string actions[] = {"[N] New", "[O] Open", "[S] Settings", "[H] Help"};
    const std::string colors[] = {green, blue, orange, purple};

    int actionsWidth = 0;
    for (const auto& action : actions) actionsWidth += static_cast<int>(action.size()) + 3;

    int actionX = std::max(0, (width - actionsWidth) / 2);

    for (int i = 0; i < 4; i++) {
        draw(actionX, top + 6, actions[i], colors[i]);
        actionX += static_cast<int>(actions[i].size()) + 3;
    }

    int y = top + 9;

    draw(left, y++, "RECENT FILES", blue);
    y++;

    const int maxRecent = std::clamp(height - 23, 1, 9);
    const int visible = std::min(static_cast<int>(recent_.size()), maxRecent);

    if (visible == 0) {
        draw(left + 2, y++, "No recent files yet", muted);
    } else {
        const char* user = std::getenv("USERPROFILE");
        const std::string home = user ? std::string(user) : "";

        for (int i = 0; i < visible; i++) {
            std::string path = recent_[i].string();

            if (!home.empty() && path.size() >= home.size() && path.compare(0, home.size(), home) == 0) {
                path = "~" + path.substr(home.size());
            }

            const int available = std::max(1, panelWidth - 6);

            if (static_cast<int>(path.size()) > available) {
                path = "..." + path.substr(path.size() - available + 3);
            }

            draw(left + 2, y, "[" + std::to_string(i + 1) + "]", green);
            draw(left + 7, y, path, white);
            y++;
        }
    }

    y += 2;

    draw(left, y++, "QUICK COMMANDS", blue);
    y++;

    draw(left + 2, y++, ".new <path>       Create a file", white);
    draw(left + 2, y++, ".open <path>      Open a file", white);
    draw(left + 2, y++, ".settings         Open preferences", white);
    draw(left + 2, y++, ".quit             Exit Kiwi", white);

    if (height >= 27) center(height - 3, "One Terminal, one Workspace.", muted);

    renderCmdBar("");
}

void HomeRenderer::renderCmdBar(const std::string& text) {
    const int y = terminal_.h() - 1;
    const int width = terminal_.w();

    if (width <= 0 || y < 0) return;

    const int available = std::max(0, width - 8);
    const std::string shown = text.substr(0, available);

    std::string line;

    // build cmdbar with ANSI colors without clearing = no flicker :)
    if (width >= 8) {
        line += "\x1b[48;2;94;112;140m\x1b[38;2;255;255;255m CMD ";
        line += "\x1b[38;2;94;112;140m\x1b[48;2;36;36;36m\xEE\x82\xB0";
        line += "\x1b[48;2;36;36;36m\x1b[38;2;220;224;232m ";
        line += shown;
        line += std::string(width - 1 - 7 - shown.size(), ' ');
    } else {
        line += "\x1b[48;2;36;36;36m";
        line += std::string(width - 1, ' ');
    }

    line += "\x1b[0m";

    terminal_.movCursor(0, y);
    std::cout << line;
    terminal_.movCursor(width >= 8 ? std::min(width - 1, 7 + static_cast<int>(shown.size())) : 0, y);
    terminal_.showCursor();
    std::cout << std::flush;
}

void HomeRenderer::renderMsg(const std::string& msg) {
    const int y = terminal_.h() - 1;

    terminal_.clearLine(y);
    terminal_.movCursor(0, y);

    std::cout << msg;
    std::cout.flush();
}