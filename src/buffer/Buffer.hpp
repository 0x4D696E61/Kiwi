#pragma once

#include <filesystem>
#include <string>
#include <vector>

class Buffer {
public:
    bool create(const std::filesystem::path& path);
    bool open(const std::filesystem::path& path);
    bool modified() const;

    std::size_t revision() const;

    bool save();
    bool saveAs(const std::filesystem::path& path);

    void insertChar(int x, int y, char character);
    void insertText(int x, int y, const std::string& text);

    void deleteChar(int x, int y);
    void deleteRange(int start, int end, int y);
    void insertLine(int x, int y);

    int mergeLine(int y);

    const std::filesystem::path& path() const;
    const std::vector<std::string>& lines() const;

private:
    std::filesystem::path path_;
    std::vector<std::string> lines_;
    bool modified_ = false;
    std::size_t revision_ = 0;
};