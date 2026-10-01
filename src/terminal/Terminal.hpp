#pragma once

class Terminal {
public:
        Terminal();
        ~Terminal();

        void clear();
        void clearLine(int y);
        void movCursor(int x, int y);
        void hideCursor();
        void showCursor();

        // eg Terminal is 120x30 : w=120 , h=30
        int w() const; // width
        int h() const; // height
};