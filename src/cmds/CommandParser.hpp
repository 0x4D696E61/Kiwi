#pragma once

#include "Command.hpp"
#include <string>

class CmdParser {
public:
    Command parse(const std::string& text) const;
};