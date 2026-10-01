#pragma once

#include <string>

class CmdBar {
public:
    void start();
    void cancel();
    void handleKey(char key);
    
    bool active() const;
    const std::string& text() const;

private:
    bool active_ = false;
    std::string text_;
};