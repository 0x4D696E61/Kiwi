#pragma once

#include <atomic>
#include <string>
#include <thread>

class Updater {
public:
    ~Updater();

    bool start(const char* url);
    void stop();

    bool available() const;
    bool mandatory() const;
    void install();

private:
    bool download(const std::string& url, const std::string& path);
    bool verify(const std::string& path, const std::string& signature);
    void installUpdate(const std::string& path);

    void* dll = nullptr;
    bool running = false;

    std::string updateUrl;
    std::string updateSignature;

    std::atomic<bool> updateAvailable{false};
    std::atomic<bool> mandatoryUpdate{false};
    std::jthread worker;
};