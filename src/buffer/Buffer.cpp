#include "Buffer.hpp"
#include <fstream>
#include <utility>

bool Buffer::create(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) { // Previously, an existing file couldve been wiped, thats changed here
        return false;
    }

    std::ofstream file(path);

    if (!file) {
        return false;
    }

    path_ = path;
    lines_.clear();
    lines_.push_back("");
    modified_ = false;
    return true;
}

bool Buffer::open(const std::filesystem::path& path) {
    std::ifstream file(path);

    if (!file) {
        return false;
    }

    std::vector<std::string> loadedLines;
    std::string line;

    while (std::getline(file, line)) {
        loadedLines.push_back(line);
    }

    if (loadedLines.empty()) {
        loadedLines.push_back("");
    }

    path_ = path;
    lines_ = std::move(loadedLines);
    modified_ = false;

    return true;
}

void Buffer::insertChar(int x, int y, char character) {
    if (y < 0 || y >= static_cast<int>(lines_.size())) {
        return;
    }

    std::string& line = lines_[y];

    if (x < 0 || x > static_cast<int>(line.length())) {
        return;
    }

    line.insert(line.begin() + x, character);
    modified_ = true;
    revision_++;
}

void Buffer::insertText(int x, int y, const std::string& text) {
    if (y < 0 || y >= static_cast<int>(lines_.size())) {
        return;
    }

    if (x < 0 || x > static_cast<int>(lines_[y].length())) {
        return;
    }

    std::string before = lines_[y].substr(0, x);
    std::string after = lines_[y].substr(x);

    std::vector<std::string> parts;
    std::string part;

    for (char c : text) {
        if (c == '\r') {
            continue;
        }
        
        if (c == '\n') {
            parts.push_back(part);
            part.clear();
        } else {
            part += c;
        }
    }

    parts.push_back(part);

    if (parts.size() == 1) {
        lines_[y] = before + parts[0] + after;
        modified_ = true;
        revision_++;
        return;
    }

    lines_[y] = before + parts[0];

    int insertY = y + 1;

    for (std::size_t i = 1; i + 1 < parts.size(); i++) {
        lines_.insert(lines_.begin() + insertY, parts[i]);
        insertY++;
    }

    lines_.insert(lines_.begin() + insertY, parts.back() + after);
    modified_ = true;
    revision_++;
}

void Buffer::deleteChar(int x, int y) {
    if (y < 0 || y >= static_cast<int>(lines_.size())) {
        return;
    }

    std::string& line = lines_[y];

    if (x <= 0 || x > static_cast<int>(line.length())) {
        return;
    }

    line.erase(line.begin() + x - 1);
    modified_ = true;
    revision_++;
}

void Buffer::insertLine(int x, int y) {
    if (y < 0 || y >= static_cast<int>(lines_.size())) {
        return;
    }

    std::string& line = lines_[y];

    if (x < 0 || x > static_cast<int>(line.length())) {
        return;
    }

    std::string newl = line.substr(x);
    line.erase(x);

    lines_.insert(lines_.begin() + y + 1, newl);
    modified_ = true;
    revision_++;
}

void Buffer::deleteRange(int start, int end, int y) {
    if (y < 0 || y >= static_cast<int>(lines_.size())) {
        return;
    }

    std::string& line = lines_[y];

    if (start < 0 || end > static_cast<int>(line.length()) || start >= end) {
        return;
    }

    line.erase(start, end - start);
    modified_ = true;
    revision_++;
}

int Buffer::mergeLine(int y) {
    if (y <= 0 || y >= static_cast<int>(lines_.size())) {
        return 0;
    }

    std::string& prevLine = lines_[y - 1];
    const int ncx = static_cast<int>(prevLine.length()); // new x pos of cursor

    prevLine += lines_[y];
    lines_.erase(lines_.begin() + y);
    modified_ = true;
    revision_++;

    return ncx;
}

bool Buffer::modified() const {
    return modified_;
}

bool Buffer::save() {
    if (path_.empty()) {
        return false;
    }

    std::ofstream file(path_);

    if (!file) {
        return false;
    }

    for (std::size_t i = 0; i < lines_.size(); i++) {
        file << lines_[i];

        if (i + 1 < lines_.size()) {
            file << '\n';
        }
    }

    if (!file) {
        return false;
    }

    modified_ = false;
    return true;
}

bool Buffer::saveAs(const std::filesystem::path& path) {
    if (path.empty()) {
        return false;
    }

    const std::filesystem::path oldPath = path_;
    path_ = path;

    if (!save()) {
        path_ = oldPath;
        return false;
    }

    return true;
}

std::size_t Buffer::revision() const {
    return revision_;
}

const std::filesystem::path& Buffer::path() const {
    return path_;
}

const std::vector<std::string>& Buffer::lines() const {
    return lines_;
}