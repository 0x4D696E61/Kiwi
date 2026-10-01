#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

struct Settings {
    bool tree = false;
    bool separator = false;
    bool blockCursor = false;

    std::filesystem::path path() const {
        const char* appdata = std::getenv("APPDATA");
        const auto base = appdata ? std::filesystem::path(appdata) : std::filesystem::current_path();
        return base / "Kiwi" / "settings.ini";
    }

    void load() {
        std::ifstream file(path());
        std::string line;
        while (std::getline(file, line)) {
            if (line == "tree=1") tree = true;
            else if (line == "tree=0") tree = false;
            else if (line == "separator=1") separator = true;
            else if (line == "separator=0") separator = false;
            else if (line == "blockCursor=1") blockCursor = true;
            else if (line == "blockCursor=0") blockCursor = false;
        }
    }

    bool save() const {
        std::error_code ec;
        std::filesystem::create_directories(path().parent_path(), ec);
        if (ec) return false;
        std::ofstream file(path(), std::ios::trunc);
        if (!file) return false;
        file << "tree=" << tree << '\n';
        file << "separator=" << separator << '\n';
        file << "blockCursor=" << blockCursor << '\n';
        return static_cast<bool>(file);
    }
};
