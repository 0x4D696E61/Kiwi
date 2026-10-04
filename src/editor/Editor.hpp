#pragma once

#include "EditorMode.hpp"
#include "EditHistory.hpp"
#include "Selection.hpp"

#include <string>

class Buffer;
class Clipboard;

class Editor {
public:
    Editor();

    void handleKey(char key, Buffer& buffer, const Clipboard& clipboard);
    void enterMoveMode();
    void reset();

    //void undo(Buffer& buffer);
    //void redo(Buffer& buffer);

    bool isTypeMode() const;
    bool isRunning() const;

    bool hasSelec() const;
    bool isLineSelec() const;
    const Selection& selection() const;

    //Lets define x and y from the Cursor
    int cX() const;
    int cY() const;

    void movTo(int x, int y, const Buffer& buffer);

private:
    EditorMode mode_;
    //EditHistory history_;
    bool running_;
    bool lineSelec_ = false;

    void movpWord(const Buffer& buffer);
    void movnWord(const Buffer& buffer);

    void movpWORD(const Buffer& buffer);
    void movnWORD(const Buffer& buffer);

    void movMatch(const Buffer& buffer);

    void toggleSelec();
    void updSelec();

    void cpLine(const Buffer& buffer, const Clipboard& clipboard);
    void cpSel(const Buffer& buffer, const Clipboard& clipboard);
    void paste(Buffer& buffer, const Clipboard& clipboard);

    void chwrd(Buffer& buffer); // Worst decision of my life defining it as chwrd, i might or might not remake it at some point so its clearer to read

    int cX_;
    int cY_;

    std::string pendingKeys_;
    Selection selection_;
};