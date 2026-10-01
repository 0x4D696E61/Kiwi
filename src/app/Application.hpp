#pragma once

class Updater;

class Application {
public:
    Application(int argc, char* argv[], Updater& updater);

    int run();

private:
    int argc_;
    char** argv_;
    Updater& updater_;
};