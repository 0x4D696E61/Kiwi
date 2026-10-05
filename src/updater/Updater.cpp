#include "Updater.hpp"
#include "Version.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <wininet.h>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

struct UpdateInfo {
    std::string version;
    std::string url;
    std::string sig;

    bool mandatory = false;
};

static UpdateInfo fetchUpdate(const char* url) {
    HINTERNET session = InternetOpenA("Kiwi Updater", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);

    if (!session) return {};

    DWORD timeout = 5000;
    InternetSetOptionA(session, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(session, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    HINTERNET request = InternetOpenUrlA( session, url, nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI, 0);

    if (!request) {
        InternetCloseHandle(session);
        return {};
    }

    DWORD status = 0;
    DWORD length = sizeof(status);

    const bool valid = HttpQueryInfoA(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &length, nullptr) && status == 200;

    std::string xml;
    bool success = valid;

    if (valid) {
        char buffer[4096];
        DWORD bytes = 0;

        while (true) {
            if (!InternetReadFile(request, buffer, sizeof(buffer), &bytes)) {
                success = false;
                break;
            }

            if (bytes == 0) break;

            xml.append(buffer, bytes);

            if (xml.size() > 262144) {
                success = false;
                break;
            }
        }
    }

    InternetCloseHandle(request);
    InternetCloseHandle(session);

    if (!success) return {};

    const std::regex vPat(R"rx(sparkle:version\s*=\s*"([0-9]+(?:\.[0-9]+)*)")rx");
    const std::regex urlPat(R"rx(url\s*=\s*"([^"]+)")rx");
    const std::regex sigPat(R"rx(sparkle:edSignature\s*=\s*"([^"]+)")rx");

    std::smatch vMatch;
    std::smatch urlMatch;
    std::smatch sigMatch;

    if (!std::regex_search(xml, vMatch, vPat)) return {};
    if (!std::regex_search(xml, urlMatch, urlPat)) return {};
    if (!std::regex_search(xml, sigMatch, sigPat)) return {};

    UpdateInfo info;
    info.version = vMatch[1].str();
    info.url = urlMatch[1].str();
    info.sig = sigMatch[1].str();
    info.mandatory = xml.find("<sparkle:criticalUpdate") != std::string::npos;

    return info;
}

static std::vector<unsigned long> versionParts(const std::string& version) {
    std::vector<unsigned long> parts;
    std::stringstream stream(version);
    std::string part;

    while (std::getline(stream, part, '.')) {
        if (part.empty() || !std::all_of(part.begin(), part.end(), [](unsigned char c) {
            return std::isdigit(c);
        })) return {};

        try {
            parts.push_back(std::stoul(part));
        } catch (...) {
            return {};
        }
    }

    return parts;
}

static bool newerVersion(const std::string& latest) {
    auto current = versionParts(kiwiversion);
    auto next = versionParts(latest);

    if (current.empty() || next.empty()) return false;

    const size_t count = std::max(current.size(), next.size());

    current.resize(count, 0);
    next.resize(count, 0);

    return next > current;
}

bool Updater::start(const char* url) {
    if (running || !url || !*url) return false;

    HMODULE lib = LoadLibraryW(L"WinSparkle.dll");

    if (!lib) return false;

    using SetUrl = void (*)(const char*);
    using SetKey = void (*)(const char*);
    using SetDetails = void (*)(const wchar_t*, const wchar_t*, const wchar_t*);
    using SetAuto = void (*)(int);
    using Init = void (*)();

    auto setUrl = reinterpret_cast<SetUrl>(GetProcAddress(lib, "win_sparkle_set_appcast_url"));
    auto setKey = reinterpret_cast<SetKey>(GetProcAddress(lib, "win_sparkle_set_eddsa_public_key"));
    auto setDetails = reinterpret_cast<SetDetails>(GetProcAddress(lib, "win_sparkle_set_app_details"));
    auto setAuto = reinterpret_cast<SetAuto>(GetProcAddress(lib, "win_sparkle_set_automatic_check_for_updates"));
    auto init = reinterpret_cast<Init>(GetProcAddress(lib, "win_sparkle_init"));

    if (!setUrl || !setKey || !setDetails || !setAuto || !init) {
        FreeLibrary(lib);
        return false;
    }

    std::string version = kiwiversion;
    std::wstring wideVersion(version.begin(), version.end());

    setDetails(L"0x4D696E61", L"Kiwi", wideVersion.c_str());
    setKey("uXpNiUW2a3dw/fcrLM2K3Z22cwdyRHCVFcR9xead7H8=");
    setUrl(url);

    setAuto(0);
    init();

    dll = lib;
    running = true;
    updateAvailable.store(false);
    mandatoryUpdate.store(false);

    worker = std::jthread([this, feed = std::string(url)](std::stop_token stop) {
        const UpdateInfo info = fetchUpdate(feed.c_str());

        if (!stop.stop_requested() && newerVersion(info.version)) {
            updateUrl = info.url;
            updateSignature = info.sig;
            mandatoryUpdate.store(info.mandatory);
            updateAvailable.store(true);
        }
    });

    return true;
}

bool Updater::available() const {
    return updateAvailable.load();
}

bool Updater::mandatory() const {
    return mandatoryUpdate.load();
}

bool Updater::download(const std::string& url, const std::string& path) {
    HINTERNET session = InternetOpenA("Kiwi Updater", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);

    if (!session) return false;

    HINTERNET req = InternetOpenUrlA(session, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI, 0);

    if (!req) {
        InternetCloseHandle(session);
        return false;
    }

    HANDLE file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(req);
        InternetCloseHandle(session);
        return false;
    }

    char buffer[16384];
    DWORD bytes = 0;
    bool suc = true;

    while (true) {
        if (!InternetReadFile(req, buffer, sizeof(buffer), &bytes)) {
            suc = false;
            break;
        }

        if (bytes == 0) break;

        DWORD written = 0;

        if (!WriteFile(file, buffer, bytes, &written, nullptr) || written != bytes) {
            suc = false;
            break;
        }
    }

    CloseHandle(file);
    InternetCloseHandle(req);
    InternetCloseHandle(session);

    if (!suc) DeleteFileA(path.c_str());

    return suc;
}

bool Updater::verify(const std::string& path, const std::string& sig) {
    if (sig.empty()) return false;

    char exePath[MAX_PATH]{};

    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH)) return false;

    std::string folder = exePath;
    const size_t slash = folder.find_last_of("\\/");

    if (slash == std::string::npos) return false;

    folder.resize(slash + 1);

    const std::string tool = folder + "winsparkle-tool.exe";
    const std::string key = "uXpNiUW2a3dw/fcrLM2K3Z22cwdyRHCVFcR9xead7H8=";
    std::string command = "\"" + tool + "\" verify --public-key \"" + key + "\" --signature \"" + sig + "\" \"" + path + "\"";

    std::vector<char> commandLine(command.begin(), command.end());
    commandLine.push_back('\0');

    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);

    PROCESS_INFORMATION process{};

    if (!CreateProcessA(nullptr, commandLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) return false;

    WaitForSingleObject(process.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    return exitCode == 0;
}

void Updater::installUpdate(const std::string& path) {
    char exePath[MAX_PATH]{};

    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH)) return;

    const std::string command = "/C timeout /T 1 /NOBREAK >NUL & start \"\" /wait \"" + path + "\" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART & \"" + std::string(exePath) + "\"";

    HINSTANCE res = ShellExecuteA(nullptr, "open", "cmd.exe", command.c_str(), nullptr, SW_HIDE);

    if (reinterpret_cast<INT_PTR>(res) <= 32) return;

    ExitProcess(0);
}

void Updater::install() {
    if (!running || !available() || updateUrl.empty() || updateSignature.empty()) return;

    char tempPath[MAX_PATH]{};

    if (!GetTempPathA(MAX_PATH, tempPath)) return;

    const std::string path = std::string(tempPath) + "Kiwi-Update.exe";

    if (!download(updateUrl, path)) return;

    if (!verify(path, updateSignature)) {
        DeleteFileA(path.c_str());
        return;
    }

    installUpdate(path);
}

void Updater::stop() {
    if (!running) return;

    if (worker.joinable()) {
        worker.request_stop();
        worker.join();
    }

    HMODULE lib = static_cast<HMODULE>(dll);

    using Cleanup = void (*)();

    auto cleanup = reinterpret_cast<Cleanup>(GetProcAddress(lib, "win_sparkle_cleanup"));

    if (cleanup) cleanup();

    FreeLibrary(lib);

    dll = nullptr;
    running = false;
    updateAvailable.store(false);
    mandatoryUpdate.store(false);
}

Updater::~Updater() {
    stop();
}