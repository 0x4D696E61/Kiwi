#pragma once

class Updater {
public:
    ~Updater();

    bool start( const char* url);
    void stop();

private:
    void* dll = nullptr;
    bool running = false;
};