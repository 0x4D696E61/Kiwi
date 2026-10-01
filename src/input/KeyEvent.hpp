#pragma once

struct KeyEvent {
    char character = 0;
    unsigned short keyCode = 0;

    bool leftCtrl = false;
    bool leftCtrlDown = false;
    bool keyDown = false;

    bool ctrlUsed = false;

    bool focSwitch = false;
};