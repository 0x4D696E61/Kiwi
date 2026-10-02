#include "Search.hpp"

#include "../buffer/Buffer.hpp"

#include <algorithm>
#include <cctype>

static std::string lower(const std::string& text) {
    std::string result = text;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return result;
}

void Search::find(const Buffer& buffer, const std::string& query, int x, int y) {
    query_ = query;
    matches_.clear();
    curr_ = -1;

    if (query_.empty()) return;

    const auto& lines = buffer.lines();
    const std::string find = lower(query_);

    for (int y = 0; y < static_cast<int>(lines.size()); y++) {
        const std::string line = lower(lines[y]);
        std::size_t pos = 0;

        while ((pos = line.find(find, pos)) != std::string::npos) {
            matches_.push_back({
                static_cast<int>(pos),
                y, // argh..
                static_cast<int>(query_.length())
            });

            pos += std::max<std::size_t>(1, query_.length());
        }
    }

    for (int i = 0; i < static_cast<int>(matches_.size()); i++) {
        const SearchMat& match = matches_[i];

        if (match.y > y || (match.y == y && match.x >= x)) {
            curr_ = i;
            break;
        }
    }

    if (curr_ == -1 && !matches_.empty()) curr_ = 0;
}

bool Search::next(const Buffer& buffer, int x, int y) {
    if (query_.empty()) return false;

    find(buffer, query_, x, y);

    if (matches_.empty()) return false;

    for (int i = 0; i < static_cast<int>(matches_.size()); i++) {
        const SearchMat& match = matches_[i];

        if (match.y > y || (match.y == y && match.x > x)) {
            curr_ = i;
            return true;
        }
    }

    curr_ = 0;
    return true;
}

bool Search::prev(const Buffer& buffer, int x, int y) {
    if (query_.empty()) return false;

    find(buffer, query_, x, y);

    if (matches_.empty()) return false;

    for (int i = static_cast<int>(matches_.size()) - 1; i >= 0; i--) {
        const SearchMat& match = matches_[i];

        if (match.y < y || (match.y == y && match.x < x)) {
            curr_ = i;
            return true;
        }
    }

    curr_ = static_cast<int>(matches_.size()) - 1;
    return true;
}

void Search::refresh(const Buffer& buffer, int x, int y) {
    if (!active() || revision_ == buffer.revision()) return;

    find(buffer, query_, x, y);
}

void Search::clear() {
    query_.clear();
    matches_.clear();
    curr_ = -1;
    revision_ = 0;
}

bool Search::active() const {
    return !query_.empty();
}

bool Search::empty() const {
    return matches_.empty();
}

const std::string& Search::query() const {
    return query_;
}

const std::vector<SearchMat>& Search::matches() const {
    return matches_;
}

const SearchMat* Search::curr() const {
    if (curr_ < 0 || curr_ >= static_cast<int>(matches_.size())) return nullptr;
    return &matches_[curr_];
}

int Search::idx() const {
    return curr_;
}

int Search::count() const {
    return static_cast<int>(matches_.size());
}