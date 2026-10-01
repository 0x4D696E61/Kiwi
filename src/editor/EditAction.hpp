#pragma once

#include <string>

enum class EditActionType {
    InsertChar, DeleteChar,
    SplitLine, MergeLine
};

struct EditAction {
    EditActionType type;

    int x = 0;
    int y = 0;

    std::string text;
};