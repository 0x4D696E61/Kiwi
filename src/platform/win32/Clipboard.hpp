#pragma once

#include <string>

class Clipboard {
public:
    bool copy(const std::string& text) const;
    std::string paste() const;
};