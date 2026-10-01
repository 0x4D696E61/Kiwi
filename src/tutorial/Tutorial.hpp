#pragma once

class Tutorial {
public:
    void open();
    void close();

    void next();
    void prev();

    bool active() const;
    int page() const;
    int pages() const;

private:
    bool active_ = false;
    int page_ = 0;

    static constexpr int pages_ = 8;
};