#include "CommandBar.hpp"

void CmdBar::start() {
    active_ = true;
    text_ = ".";
}

void CmdBar::cancel() {
    active_ = false;
    text_.clear();
}

void CmdBar::handleKey(char key) {
    if (!active_) {
        return;
    }

    // Handles Backspace
    if (key == '\b') {
        if (text_.length() > 1) {
            text_.pop_back();
        }
        
        return;
    }

    if (key >= 32 && key <= 126) {
        text_ += key;
    }
}

bool CmdBar::active() const {
    return active_;
}

const std::string& CmdBar::text() const {
    return text_;
}