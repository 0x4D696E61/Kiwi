#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>

struct ExplorerEntry {
    std::filesystem::path path;
    std::string name;
    bool dir;
    int depth = 0;
    bool expanded = false;
};

class Explorer {
public:
    void open(const std::filesystem::path& path);
    void refresh();

    const std::filesystem::path& root() const;
    const std::vector<ExplorerEntry>& entries() const;

    int selected() const;
    void move(int amount);
    void up();

    bool tree() const;
    void setTree(bool enabled);
    void toggle();

    const ExplorerEntry* curr() const;

private:
    std::filesystem::path root_;
    std::vector<ExplorerEntry> entries_;
    std::set<std::filesystem::path> expanded_;

    int selected_ = 0;
    bool tree_ = false;

    void scan();
};