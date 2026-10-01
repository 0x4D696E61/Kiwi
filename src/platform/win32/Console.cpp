#include "Console.hpp"

// fuck u windows
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

Console::Console() {
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    inputHandle_ = input;

    DWORD mode = 0;

    if (GetConsoleMode(input, &mode)) {
        originalMode_ = mode;

        mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
        mode |= ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS;

        SetConsoleMode(input, mode);
    }
}

Console::~Console() {
    HANDLE input = static_cast<HANDLE>(inputHandle_);
    SetConsoleMode(input, originalMode_);
}

KeyEvent Console::readKey() {
    if (repeatsL_ > 0) {
        repeatsL_--;
        return repeatEvent_;
    }

    HANDLE input = static_cast<HANDLE>(inputHandle_);

    INPUT_RECORD record{};
    DWORD eventsRead = 0;

    while (true) {
        if (!ReadConsoleInputW(input, &record, 1, &eventsRead)) return {};
        if (record.EventType != KEY_EVENT) continue;

        const KEY_EVENT_RECORD& keyEvent = record.Event.KeyEvent;
        const bool ctrlKey = keyEvent.wVirtualKeyCode == VK_CONTROL || keyEvent.wVirtualKeyCode == VK_LCONTROL;

        if (ctrlKey) {
            if (keyEvent.bKeyDown) {
                if (!ctrlHeld_) {
                    ctrlHeld_ = true;
                    ctrlUsed_ = false;
                    focUsed_ = false;
                }
            } else {
                ctrlHeld_ = false;
                focUsed_ = false;

                KeyEvent event;
                event.keyCode = keyEvent.wVirtualKeyCode;
                event.keyDown = false;
                event.leftCtrl = true;
                event.ctrlUsed = ctrlUsed_;

                ctrlUsed_ = false;
                return event;
            }

            continue;
        }

        if ((keyEvent.dwControlKeyState & RIGHT_ALT_PRESSED) != 0) ctrlUsed_ = true;

        if (ctrlHeld_ && (keyEvent.dwControlKeyState & RIGHT_ALT_PRESSED) == 0) {

            ctrlUsed_ = true;

            // Allow lCtrl+Space for focus switch
            if (keyEvent.wVirtualKeyCode == VK_SPACE) {
                if (!keyEvent.bKeyDown) {
                    focUsed_ = false;
                    continue;
                }

                if (!focUsed_) {
                    focUsed_ = true;

                    KeyEvent event;
                    event.keyDown = true;
                    event.focSwitch = true;
                    return event;
                }
            }

            continue;
        }

        if (!keyEvent.bKeyDown) continue;

        KeyEvent event;
        event.keyCode = keyEvent.wVirtualKeyCode;
        event.keyDown = true;
        event.leftCtrlDown = (keyEvent.dwControlKeyState & LEFT_CTRL_PRESSED) != 0 && (keyEvent.dwControlKeyState & RIGHT_ALT_PRESSED) == 0;

        const wchar_t character = keyEvent.uChar.UnicodeChar;

        if (character >= 1 && character <= 127) {
            event.character = static_cast<char>(character);
        }

        if (event.character == 0 && !event.leftCtrlDown) continue;

        if (event.character != 0 && keyEvent.wRepeatCount > 1) {
            repeatEvent_ = event;
            repeatsL_ = keyEvent.wRepeatCount - 1;
        }

        return event;
    }
}
