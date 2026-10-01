#include "Updater.hpp"

#include <windows.h>

bool Updater::start(const char* url) {
    if (running || !url || !*url) return false;

    HMODULE lib = LoadLibraryW(L"WinSparkle.dll");
    if (!lib) return false;

    using SetUrl = void (*)(const char*);
    using Init = void (*)();
    using Cleanup = void (*)();

    auto setUrl = reinterpret_cast<SetUrl>(GetProcAddress(lib, "win_sparkle_set_appcast_url"));
    auto init = reinterpret_cast<Init>(GetProcAddress(lib, "win_sparkle_init"));
    auto cleanup = reinterpret_cast<Cleanup>(GetProcAddress(lib, "win_sparkle_cleanup"));

    if (!setUrl || !init || !cleanup) {
        FreeLibrary(lib);
        return false;
    }

    setUrl(url);
    init();

    dll = lib;
    running = true;

    return true;
}

void Updater::stop() {
    if (!running) return;

    HMODULE lib = static_cast<HMODULE>(dll);

    using Cleanup = void (*)();

    auto cleanup = reinterpret_cast<Cleanup>(GetProcAddress(lib, "win_sparkle_cleanup"));

    if (cleanup) cleanup();

    FreeLibrary(lib);

    dll = nullptr;
    running = false;
}

Updater::~Updater() {
    stop();
}