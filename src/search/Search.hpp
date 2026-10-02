#pragma once

#include <string>
#include <vector>
#include <cstddef>

class Buffer;

struct SearchMat {
    int x;
    int y;
    int length;
};

class Search {
public:
    void find(const Buffer& buffer, const std::string& query, int x, int y);

    bool next(const Buffer& buffer, int x, int y);
    bool prev(const Buffer& buffer, int x, int y);

    void refresh(const Buffer& buffer, int x, int y);

    void clear();

    bool active() const;
    bool empty() const;

    const std::string& query() const;
    const std::vector<SearchMat>& matches() const;
    const SearchMat* curr() const;

    int idx() const;
    int count() const;

private:
    std::string query_;
    std::vector<SearchMat> matches_;
    int curr_ = -1;
    std::size_t revision_ = 0;
};