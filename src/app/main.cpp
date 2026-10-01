#include "Application.hpp"

#include "../updater/Updater.hpp"

int main(int argc, char* argv[]) {
    Updater updater;
    updater.start("");

    Application app(argc, argv);

    return app.run();
}