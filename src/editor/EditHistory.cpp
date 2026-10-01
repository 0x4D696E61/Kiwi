#include "EditHistory.hpp"

void EditHistory::push(const EditAction& action) {
    undoStack_.push_back(action);
    redoStack_.clear();
}

bool EditHistory::canUndo() const {
    return !undoStack_.empty();
}

bool EditHistory::canRedo() const {
    return !redoStack_.empty();
}

EditAction EditHistory::undo() {
    EditAction action = undoStack_.back();
    undoStack_.pop_back();
    redoStack_.push_back(action);

    return action;
}

EditAction EditHistory::redo() {
    EditAction action = redoStack_.back();
    redoStack_.pop_back();
    undoStack_.push_back(action);

    return action;
}

void EditHistory::clear() {
    undoStack_.clear();
    redoStack_.clear();
}