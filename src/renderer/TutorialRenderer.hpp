#pragma once

class Screen;
class Tutorial;

class TutorialRenderer {
public:
    void render(Screen& screen, const Tutorial& tutorial);
};