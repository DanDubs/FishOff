#pragma once
#include <SFML/Graphics.hpp>

#include "Pond.hpp"
#include "Rod.hpp"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void drawScenery();

    sf::RenderWindow window_;
    Pond pond_;
    Rod rod_;
    sf::Vector2f mouse_;
    int shownCatches_ = -1;
};
