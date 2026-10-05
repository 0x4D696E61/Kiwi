#include "Terminal.hpp"
#include <iostream>
#include <windows.h>

Terminal::Terminal() {
    SetConsoleOutputCP(CP_UTF8);
}

Terminal::~Terminal() {
    std::cout << "\x1b[?1000l\x1b[?1002l\x1b[?1003l\x1b[?1006l" << std::flush;
}

void Terminal::clear() {
    std::cout << "\x1b[2J\x1b[H";
}

void Terminal::clearLine(int y) {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(output, &info)) {
        return;
    }

    const COORD start {
        info.srWindow.Left,
        static_cast<SHORT>(info.srWindow.Top + y)
    };

    const DWORD length = info.srWindow.Right - info.srWindow.Left + 1;
    DWORD written = 0;
    
    FillConsoleOutputCharacter(output, L' ', length, start, &written);
}

void Terminal::movCursor(int x, int y) {
    std::cout << "\x1b[" << y + 1 << ';' << x + 1 << 'H'; // ANSI uses ESC[row;columnH and positions are at 1 -> kiwi starts at 0
    std::cout.flush();
}

void Terminal::hideCursor() {
    std::cout << "\x1b[?25l";
    std::cout.flush();
}

void Terminal::showCursor() {
    std::cout << "\x1b[?25h";
    std::cout.flush();
}
int Terminal::w() const {
    CONSOLE_SCREEN_BUFFER_INFO info{};

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    return info.srWindow.Right - info.srWindow.Left + 1;
}

int Terminal::h() const {
    CONSOLE_SCREEN_BUFFER_INFO info{};

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    return info.srWindow.Bottom - info.srWindow.Top + 1;
}