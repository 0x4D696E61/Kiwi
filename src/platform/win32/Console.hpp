#pragma once

#include "../../input/KeyEvent.hpp"

class Console {
public:
    Console();
    ~Console();

    KeyEvent readKey();

private:
    void* inputHandle_;
    unsigned long originalMode_;

    bool ctrlHeld_ = false;
    bool ctrlUsed_ = false;
    bool focUsed_ = false;
    bool mouseHeld_ = false;
    int mouseX_ = 0;
    int mouseY_ = 0;

    KeyEvent repeatEvent_{};
    unsigned short repeatsL_ = 0;
};
