#pragma once

class Application {
public:
    Application(int argc, char* argv[]);

    int run();

private:
    int argc_;
    char** argv_;
};