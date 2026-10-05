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
        mode |= ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT;

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
        const DWORD result = WaitForSingleObject(input, 25);

        if (result == WAIT_TIMEOUT) {
            if (mouseHeld_) {
                KeyEvent event;
                event.mouse = true;
                event.mouseX = mouseX_;
                event.mouseY = mouseY_;
                event.mouseMove = true;
                return event;
            }
        
            return {};
        }

        if (result != WAIT_OBJECT_0) return {};

        if (!ReadConsoleInputW(input, &record, 1, &eventsRead)) return {};

        if (record.EventType == MOUSE_EVENT) {
            const MOUSE_EVENT_RECORD& mouseEvent = record.Event.MouseEvent;

            mouseX_ = mouseEvent.dwMousePosition.X;
            mouseY_ = mouseEvent.dwMousePosition.Y;
            mouseHeld_ = (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

            if (mouseEvent.dwEventFlags == MOUSE_MOVED && (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0) {
                KeyEvent event;
                event.mouse = true;
                event.mouseX = mouseEvent.dwMousePosition.X;
                event.mouseY = mouseEvent.dwMousePosition.Y;
                event.mouseMove = true;
                return event;
            }

            if (mouseEvent.dwEventFlags == 0 && (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) == 0) {
                KeyEvent event;
                event.mouse = true;
                event.mouseX = mouseEvent.dwMousePosition.X;
                event.mouseY = mouseEvent.dwMousePosition.Y;
                event.mouseRelease = true;
                return event;
            }

            if (mouseEvent.dwEventFlags == MOUSE_WHEELED) {
                KeyEvent event;
                event.mouse = true;
                event.mouseX = mouseEvent.dwMousePosition.X;
                event.mouseY = mouseEvent.dwMousePosition.Y;
                event.mouseWheel = GET_WHEEL_DELTA_WPARAM(mouseEvent.dwButtonState);
                event.shift = (mouseEvent.dwControlKeyState & SHIFT_PRESSED) != 0;
                event.ctrl = (mouseEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
                event.alt = (mouseEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
                return event;
            }

            if ((mouseEvent.dwEventFlags == 0 || mouseEvent.dwEventFlags == DOUBLE_CLICK) && (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0) {
                KeyEvent event;
                event.mouse = true;
                event.mouseX = mouseEvent.dwMousePosition.X;
                event.mouseY = mouseEvent.dwMousePosition.Y;
                event.mouseLeft = true;
                if ((mouseEvent.dwEventFlags & DOUBLE_CLICK) != 0) event.mouseClicks = 2;
                else event.mouseClicks = 1;
                return event;
            }

            continue;
        }

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