#include "Clipboard.hpp"

#include <windows.h>

bool Clipboard::copy(const std::string& text) const {
    if (text.empty()) {
        return false;
    }

    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);

    if (size <= 0) {
        return false;
    }

    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, (size + 1) * sizeof(wchar_t));

    if (!memory) {
        return false;
    }

    wchar_t* data = static_cast<wchar_t*>(GlobalLock(memory));

    if (!data) {
        GlobalFree(memory);
        return false;
    }

    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), data, size);
    data[size] = L'\0';

    GlobalUnlock(memory);

    if (!OpenClipboard(nullptr)) {
        GlobalFree(memory);
        return false;
    }

    EmptyClipboard();

    if (!SetClipboardData(CF_UNICODETEXT, memory)) {
        GlobalFree(memory);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

std::string Clipboard::paste() const {
    if (!OpenClipboard(nullptr)) {
        return "";
    }

    HANDLE memory = GetClipboardData(CF_UNICODETEXT);

    if (!memory) {
        CloseClipboard();
        return "";
    }

    const wchar_t* data = static_cast<const wchar_t*>(GlobalLock(memory));

    if (!data) {
        CloseClipboard();
        return "";
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, data, -1, nullptr, 0, nullptr, nullptr);

    if (size <= 0) {
        GlobalUnlock(memory);
        CloseClipboard();
        return "";
    }

    std::string text(size, '\0');

    WideCharToMultiByte(CP_UTF8, 0, data, -1, text.data(), size, nullptr, nullptr);

    GlobalUnlock(memory);
    CloseClipboard();

    text.pop_back();
    return text;
}