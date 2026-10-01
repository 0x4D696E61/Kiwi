#include "Tutorial.hpp"

void Tutorial::open() {
    active_ = true;
    page_ = 0;
}

void Tutorial::close() {
    active_ = false;
}

void Tutorial::next() {
    if (page_ < pages_ - 1) page_++;
}

void Tutorial::prev() {
    if (page_ > 0) page_--;
}

bool Tutorial::active() const {
    return active_;
}

int Tutorial::page() const {
    return page_;
}

int Tutorial::pages() const {
    return pages_;
}