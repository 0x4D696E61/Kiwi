#pragma once

struct KeyEvent {
    char character = 0;
    unsigned short keyCode = 0;

    bool leftCtrl = false;
    bool leftCtrlDown = false;
    bool keyDown = false;

    bool ctrlUsed = false;

    bool focSwitch = false;
    
    bool mouse = false;
    int mouseX = 0;
    int mouseY = 0;
    bool mouseLeft = false;
    
    bool mouseMove = false;
    bool mouseRelease = false;
    int mouseClicks = 0;

    int mouseWheel = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};