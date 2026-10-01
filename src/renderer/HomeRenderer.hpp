#pragma once

#include <filesystem>
#include <string>
#include <vector>

class Terminal;

class HomeRenderer {
public:
    explicit HomeRenderer(Terminal& terminal);

    void render(int offset = 0);
    void renderCmdBar(const std::string& text);
    void renderMsg(const std::string& msg);

    void addRecent(const std::filesystem::path& path);
    std::filesystem::path recent(int idx) const;

private:
    void loadRecent();
    void savRecent() const;

    Terminal& terminal_;
    std::vector<std::filesystem::path> recent_;
};