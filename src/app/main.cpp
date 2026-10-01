#include "Application.hpp"

#include "../updater/Updater.hpp"

int main(int argc, char* argv[]) {
    Updater updater;
    updater.start("https://raw.githubusercontent.com/0x4D696E61/Kiwi/main/appcast.xml");

    Application app(argc, argv, updater);

    return app.run();
}