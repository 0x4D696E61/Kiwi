#include "Editor.hpp"
#include "../buffer/Buffer.hpp"
#include "../platform/win32/Clipboard.hpp"

#include <algorithm>
#include <cctype>

static bool iswc(char c) { // Wordcahr
    const unsigned char ch = static_cast<unsigned char>(c);
    return std::isalnum(ch) || c == '_';
}

Editor::Editor() : mode_(EditorMode::Move), running_(true), cX_(0), cY_(0) {} // internal cursorX, cursorY

void Editor::handleKey(char key, Buffer& buffer, const Clipboard& clipboard) {
    if (mode_ == EditorMode::Move && key == ' ') {
        selection_.active = false;
        lineSelec_ = false;
        mode_ = EditorMode::Type;
        return;
    }

    if (mode_ == EditorMode::Type) {

        // Backspace
        if (key == '\b') {
            if (cX_ > 0) {
                const auto& lines = buffer.lines();
                const std::string& line = lines[cY_];
            
                if (cX_ < static_cast<int>(line.length())) {
                    const char left = line[cX_ - 1];
                    const char right = line[cX_];
                    const bool pair = (left == '(' && right == ')') || (left == '[' && right == ']') || (left == '{' && right == '}') || (left == '"' && right == '"') || (left == '\'' && right == '\'');
                
                    if (pair) {
                        buffer.deleteChar(cX_ + 1, cY_);
                        buffer.deleteChar(cX_, cY_);
                        cX_--;
                        return;
                    }
                }
            
                buffer.deleteChar(cX_, cY_);
                cX_--;
            } else if (cY_ > 0) {
                cX_ = buffer.mergeLine(cY_);
                cY_--;
            }
        
            return;
        }

        // Enter
        if (key == '\r') {
            const auto& lines = buffer.lines();
            const std::string line = lines[cY_];
        
            int indent = 0;
            while (indent < static_cast<int>(line.length()) && line[indent] == ' ') indent++;
        
            const bool bracePair = cX_ > 0 && cX_ < static_cast<int>(line.length()) && line[cX_ - 1] == '{' && line[cX_] == '}';
        
            if (bracePair) {
                buffer.insertLine(cX_, cY_);
                cY_++;
            
                buffer.deleteRange(0, static_cast<int>(buffer.lines()[cY_].length()), cY_);
            
                for (int i = 0; i < indent + 4; i++) buffer.insertChar(i, cY_, ' ');
            
                buffer.insertLine(indent + 4, cY_);
            
                cY_++;
                for (int i = 0; i < indent; i++) buffer.insertChar(i, cY_, ' ');
                buffer.insertChar(indent, cY_, '}');
            
                cY_--;
                cX_ = indent + 4;
                return;
            }
        
            if (line.find_first_not_of(' ') == std::string::npos) {
                buffer.insertLine(cX_, cY_);
                cY_++;
                        
                for (int i = 0; i < indent; i++) buffer.insertChar(i, cY_, ' ');
                        
                cX_ = indent;
                return;
            }
        
            buffer.insertLine(cX_, cY_);
            cY_++;
        
            for (int i = 0; i < indent; i++) buffer.insertChar(i, cY_, ' ');
        
            cX_ = indent;
            return;
        }

        if (key == '\t') {
            constexpr int tabSize = 4;

            for (int i = 0; i < tabSize; i++) {
                buffer.insertChar(cX_, cY_, ' ');
                cX_++;
            }

            return;
        }

        // All normal chars and symbols on a fucking keyboard
        if (key == ')' || key == ']' || key == '}' || key == '"' || key == '\'') {
            const auto& lines = buffer.lines();

            if (cY_ < static_cast<int>(lines.size()) && cX_ < static_cast<int>(lines[cY_].length()) && lines[cY_][cX_] == key) {
                cX_++;
                return;
            }
        }

        char close = 0;

        if (key == '(') close = ')';
        else if (key == '[') close = ']';
        else if (key == '{') close = '}';
        else if (key == '"') close = '"';
        else if (key == '\'') close = '\'';

        if ((key == '"' || key == '\'') && close != 0) {
            int count = 0;
                
            for (int i = 0; i < cX_; i++) {
                if (buffer.lines()[cY_][i] == key) count++;
            }
        
            if (count % 2 != 0) {
                buffer.insertChar(cX_, cY_, key);
                cX_++;
                return;
            }
        }

        if (close != 0) {
            //history_.push({EditActionType::InsertChar, cX_, cY_, std::string(1, key)});

            buffer.insertChar(cX_, cY_, key);
            buffer.insertChar(cX_ + 1, cY_, close);
            cX_++;
            return;
        }

        if (key >= 32 && key <= 126) {
            /*history_.push({
                EditActionType::InsertChar,
                cX_,                           // My hands will hurt forever due to writing code like that...
                cY_,
                std::string(1, key)
            });*/

            buffer.insertChar(cX_, cY_, key); // finally one line again :D
            cX_++;
        }

        return;
    }

    if (key == 'v' && pendingKeys_ == "c") {
        cpSel(buffer, clipboard);
        pendingKeys_.clear();
        return;
    }

    if (key == 'a' && pendingKeys_ == "c") {
        paste(buffer, clipboard);
        pendingKeys_.clear();
        return;
    }

    if (key == 'v' && pendingKeys_ != "x") {
    toggleSelec();
    pendingKeys_.clear();
    return;
}

    if (key == 'V') {
        const auto& lines = buffer.lines();

        if (lines.empty()) {
            return;
        }

        selection_.active = true;
        lineSelec_ = true;
        selection_.startX = 0;
        selection_.startY = cY_;
        selection_.endX = static_cast<int>(lines[cY_].length());
        selection_.endY = cY_;

        cX_ = 0;
        pendingKeys_.clear();
        return;
    }

    if (key == '%') {
        pendingKeys_.clear();
        movMatch(buffer);
        return;
    }

    // key sequences eg qq qQ rr rR xx xa xv cc cv ca gg GG
    if (key == 'q' || key == 'Q' || key == 'r' || key == 'R' || key == 'x' || key == 'c' || key == 'g' || key == 'G' || (pendingKeys_ == "x" && (key == 'a' || key == 'v'))) {
        if (pendingKeys_.empty()) {
            pendingKeys_ = key;
        } else {
            pendingKeys_ += key;
        }
    
        if (pendingKeys_ == "qq") {
            movpWord(buffer);
            pendingKeys_.clear();
        } else if (pendingKeys_ == "qQ") {
            movpWORD(buffer);
            pendingKeys_.clear();
        } else if (pendingKeys_ == "rr") {
            movnWord(buffer);
            pendingKeys_.clear();
        } else if (pendingKeys_ == "rR") {
            movnWORD(buffer);
            pendingKeys_.clear();
        } else if (pendingKeys_ == "gg") {
            cY_ = 0;
            cX_ = 0;
            updSelec();
            pendingKeys_.clear();
        } else if (pendingKeys_ == "GG") {
            const auto& lines = buffer.lines();
        
            if (!lines.empty()) {
                cY_ = static_cast<int>(lines.size()) - 1;
                cX_ = 0;
                updSelec();
            }
        
            pendingKeys_.clear();
        } else if (pendingKeys_ == "xx") {
            chwrd(buffer);
            pendingKeys_.clear();
        } else if (pendingKeys_ == "xa" || pendingKeys_ == "xv") {
            const bool deleteSelection = selection_.active;
        
            if (deleteSelection) {
                int startX = selection_.startX;
                int startY = selection_.startY;
                int endX = selection_.endX;
                int endY = selection_.endY;
            
                if (startY > endY || (startY == endY && startX > endX)) {
                    std::swap(startX, endX);
                    std::swap(startY, endY);
                }
            
                if (lineSelec_) {
                    startX = 0;
                    endX = static_cast<int>(buffer.lines()[endY].length());
                
                    if (endY + 1 < static_cast<int>(buffer.lines().size())) {
                        endY++;
                        endX = 0;
                    }
                }
            
                if (startY == endY) {
                    buffer.deleteRange(startX, endX, startY);
                } else {
                    const std::string left = buffer.lines()[startY].substr(0, startX);
                    const std::string right = buffer.lines()[endY].substr(endX);
                
                    buffer.deleteRange(startX, static_cast<int>(buffer.lines()[startY].length()), startY);
                    buffer.deleteRange(0, endX, endY);
                
                    for (int y = endY; y > startY; y--) buffer.mergeLine(y);
                
                    const int joined = static_cast<int>(buffer.lines()[startY].length());
                    buffer.deleteRange(0, joined, startY);
                    buffer.insertText(0, startY, left + right);
                }
            
                cX_ = startX;
                cY_ = startY;
            } else if (pendingKeys_ == "xa") {
                const auto& lines = buffer.lines();

                if (!lines.empty()) {
                    buffer.deleteRange(0, static_cast<int>(lines[cY_].length()), cY_);
                    cX_ = 0;
                }
            }
        
            selection_ = {};
            lineSelec_ = false;
            mode_ = EditorMode::Type;
            pendingKeys_.clear();
        } else if (pendingKeys_ == "cc") {
            cpLine(buffer, clipboard);
            pendingKeys_.clear();
        }
    
        return;
    }

    pendingKeys_.clear();

    const auto& lines = buffer.lines();

    if (lines.empty()) {
        cX_ = 0;
        cY_ = 0;
        return;
    }

    if (key == 'A') {
        cX_ = 0;
        updSelec();
        return;
    }

    if (key == 'D') {
        cX_ = static_cast<int>(lines[cY_].length());
        updSelec();
        return;
    }

    if (key == 'w' && cY_ > 0) {
        cY_--;
    }

    if (key == 's' && cY_ + 1 < static_cast<int>(lines.size())) {
        cY_++;
    }

    const int lineLength = static_cast<int>(lines[cY_].length());

    if (key == 'a' && cX_ > 0) {
        cX_--;
    }

    if (key == 'd' && cX_ < lineLength) {
        cX_++;
    }

    if (cX_ > lineLength) {
        cX_ = lineLength;
    }

    updSelec();
}

void Editor::enterMoveMode() {
    mode_ = EditorMode::Move;
}

void Editor::reset() {
    lineSelec_ = false;
    mode_ = EditorMode::Move;
    cX_ = 0;
    cY_ = 0;

    selection_ = {};
    pendingKeys_.clear();
    //history_.clear();
}

/*
MASSIVELY BUGGED AND SHIT IMPLEMENTATION o7

void Editor::undo(Buffer& buffer) {
    if (!history_.canUndo()) {
        return;
    }

    const EditAction action = history_.undo();

    if (action.type == EditActionType::InsertChar) {
        buffer.deleteChar(action.x + 1, action.y);

        cX_ = action.x;
        cY_ = action.y;
    }
}

void Editor::redo(Buffer& buffer) {
    if (!history_.canRedo()) {
        return;
    }

    const EditAction action = history_.redo();

    if (action.type == EditActionType::InsertChar) {
        buffer.insertChar(action.x, action.y, action.text[0]);

        cX_ = action.x + 1;
        cY_ = action.y;
    }
}

*/

void Editor::movpWord(const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) {
        return;
    }

    // At the start continue searching from the previous line
    if (cX_ == 0) {
        if (cY_ == 0) {
            return;
        }

        cY_--;
        cX_ = static_cast<int>(lines[cY_].length());
    }

    const std::string& line = lines[cY_];

    // Skip whitespace behind the cursor
    while (cX_ > 0 && std::isspace(static_cast<unsigned char>(line[cX_ - 1]))) {
        cX_--;
    }

    // Move to the beginning of the prev or curr word
    if (cX_ > 0 && iswc(line[cX_ - 1])) {
        while (cX_ > 0 && iswc(line[cX_ - 1])) {
            cX_--;
        }
    } else if (cX_ > 0) {
        cX_--;
    }

    updSelec();
}

void Editor::movnWord(const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) {
        return;
    }

    const std::string& line = lines[cY_];
    const int len = static_cast<int>(line.length());

    // Reaching the end continues at the next line
    if (cX_ >= len) {
        if (cY_ + 1 < static_cast<int>(lines.size())) {
            cY_++;
            cX_ = 0;
        }

        updSelec();
        return;
    }

    // Skip whitespace until the next word
    while (cX_ < len && std::isspace(static_cast<unsigned char>(line[cX_]))) {
        cX_++;
    }

    // Move past the curr word
    if (cX_ < len && iswc(line[cX_])) {
        while (cX_ < len && iswc(line[cX_])) {
            cX_++;
        }
    } else if (cX_ < len) {
        cX_++;
    }

    updSelec();
}

void Editor::movpWORD(const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) return;

    if (cX_ == 0) {
        if (cY_ == 0) return;

        cY_--;
        cX_ = static_cast<int>(lines[cY_].length());
    }

    const std::string& line = lines[cY_];

    while (cX_ > 0 && std::isspace(static_cast<unsigned char>(line[cX_ - 1]))) {
        cX_--;
    }

    while (cX_ > 0 && !std::isspace(static_cast<unsigned char>(line[cX_ - 1]))) {
        cX_--;
    }

    updSelec();
}

void Editor::movnWORD(const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) return;

    const std::string& line = lines[cY_];
    const int len = static_cast<int>(line.length());

    if (cX_ >= len) {
        if (cY_ + 1 < static_cast<int>(lines.size())) {
            cY_++;
            cX_ = 0;
        }

        updSelec();
        return;
    }

    while (cX_ < len && !std::isspace(static_cast<unsigned char>(line[cX_]))) {
        cX_++;
    }

    while (cX_ < len && std::isspace(static_cast<unsigned char>(line[cX_]))) {
        cX_++;
    }

    updSelec();
}

void Editor::movMatch(const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) return;

    const std::string& line = lines[cY_];

    if (cX_ < 0 || cX_ >= static_cast<int>(line.length())) return;

    const char curr = line[cX_];

    char match = 0;
    int dir = 0;

    if (curr == '(') {
        match = ')';
        dir = 1;
    } else if (curr == '[') {
        match = ']';
        dir = 1;
    } else if (curr == '{') {
        match = '}';
        dir = 1;
    } else if (curr == ')') {
        match = '(';
        dir = -1;
    } else if (curr == ']') {
        match = '[';
        dir = -1;
    } else if (curr == '}') {
        match = '{';
        dir = -1;
    } else {
        return;
    }

    int depth = 0;
    int y = cY_;
    int x = cX_;

    while (true) {
        x += dir;

        if (dir > 0) {
            while (y < static_cast<int>(lines.size()) && x >= static_cast<int>(lines[y].length())) {
                y++;
                x = 0;
            }

            if (y >= static_cast<int>(lines.size())) return;
        } else {
            while (y >= 0 && x < 0) {
                y--;

                if (y >= 0) {
                    x = static_cast<int>(lines[y].length()) - 1;
                }
            }

            if (y < 0) return;
        }

        const char character = lines[y][x];

        if (character == curr) {
            depth++;
        } else if (character == match) {
            if (depth == 0) {
                cX_ = x;
                cY_ = y;
                updSelec();
                return;
            }

            depth--;
        }
    }
}

void Editor::toggleSelec() {
    lineSelec_ = false;

    if (selection_.active) {
        selection_.active = false;
        return;
    }

    selection_.active = true;

    selection_.startX = cX_;
    selection_.startY = cY_;
    selection_.endX = cX_;
    selection_.endY = cY_;    
}

void Editor::updSelec() {
    if (!selection_.active) {
        return;
    }

    selection_.endX = cX_;
    selection_.endY = cY_;
}

void Editor::chwrd(Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) {
        return;
    }

    const std::string& line = lines[cY_];
    const int len = static_cast<int>(line.length());

    if (len == 0) {
        mode_ = EditorMode::Type;
        return;
    }

    int start = cX_;

    // find next word on Whitespace
    while (start < len && std::isspace(static_cast<unsigned char>(line[start]))) {
        start++;
    }

    if (start >= len) {
        return;
    }

    // Find beginning of word
    while (start > 0 && !std::isspace(static_cast<unsigned char>(line[start - 1]))) {
        start--;
    }

    int end = start;

    // Find end of word
    while (end < len && !std::isspace(static_cast<unsigned char>(line[end]))) {
        end++;
    }

    buffer.deleteRange(start, end, cY_);

    cX_ = start;
    mode_ = EditorMode::Type;
}

void Editor::cpLine(const Buffer& buffer, const Clipboard& clipboard) {
    const auto& lines = buffer.lines();

    if (cY_ < 0 || cY_ >= static_cast<int>(lines.size())) {
        return;
    }

    clipboard.copy(lines[cY_]);
}


void Editor::cpSel(const Buffer& buffer, const Clipboard& clipboard) {
    if (!selection_.active) return;

    const auto& lines = buffer.lines();

    int startX = selection_.startX;
    int startY = selection_.startY;
    int endX = selection_.endX;
    int endY = selection_.endY;

    if (startY > endY || (startY == endY && startX > endX)) {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }

    std::string text;

    if (lineSelec_) {
        for (int y = startY; y <= endY; y++) {
            text += lines[y];
            text += '\n';
        }

        clipboard.copy(text);
        return;
    }

    if (startY == endY) {
        text = lines[startY].substr(startX, endX - startX);
    } else {
        text = lines[startY].substr(startX);
        text += '\n';

        for (int y = startY + 1; y < endY; y++) {
            text += lines[y];
            text += '\n';
        }

        text += lines[endY].substr(0, endX);
    }

    if (text.empty()) return;

    clipboard.copy(text);
}


void Editor::paste(Buffer& buffer, const Clipboard& clipboard) {
    const std::string text = clipboard.paste();

    if (text.empty()) {
        return;
    }

    buffer.insertText(cX_, cY_, text);

    int newLines = 0;
    int lastLen = 0;

    for (char c : text) {
        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            newLines++;
            lastLen = 0;
        } else {
            lastLen++;
        }
    }

    if (newLines == 0) {
        cX_ += lastLen;
    } else {
        cY_ += newLines;
        cX_ = lastLen;
    }

    selection_.active = false;
}

bool Editor::hasSelec() const {
    return selection_.active;
}

bool Editor::isLineSelec() const {
    return lineSelec_;
}

bool Editor::isTypeMode() const {
    return mode_ == EditorMode::Type;
}

bool Editor::isRunning() const {
    return running_;
}

const Selection& Editor::selection() const {
    return selection_;
}

int Editor::cX() const {
    return cX_;
}

int Editor::cY() const {
    return cY_;
}

void Editor::movTo(int x, int y, const Buffer& buffer) {
    const auto& lines = buffer.lines();

    if (lines.empty()) {
        cX_ = 0;
        cY_ = 0;
        return;
    }

    cY_ = std::clamp(y, 0, static_cast<int>(lines.size()) - 1);
    cX_ = std::clamp(x, 0, static_cast<int>(lines[cY_].length()));
}