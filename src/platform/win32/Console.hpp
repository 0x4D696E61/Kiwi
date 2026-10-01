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

    KeyEvent repeatEvent_{};
    unsigned short repeatsL_ = 0;
};
