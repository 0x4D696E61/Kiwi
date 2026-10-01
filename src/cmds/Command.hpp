#pragma once

#include <string>

enum class CommandType {
    None, New,
    Open, Quit, ForceQuit,
    Save, SaveAs, SaveQuit,
    Unknown
};

struct Command {
    CommandType type = CommandType::None;
    std::string argument;
};