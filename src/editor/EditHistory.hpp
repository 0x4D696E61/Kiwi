#pragma once

#include "EditAction.hpp"
#include <vector>

class EditHistory {
public:
    void push(const EditAction& action);

    bool canUndo() const;
    bool canRedo() const;

    EditAction undo();
    EditAction redo();

    void clear();

private:
    std::vector<EditAction> undoStack_;
    std::vector<EditAction> redoStack_;
};