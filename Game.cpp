#include "Game.hpp"

#include <iostream>
#include <string>

Game::Game()
    : window_(sf::VideoMode(900, 650), "Fishing Game"),
      pond_(sf::Vector2f(450.f, 255.f), sf::Vector2f(340.f, 185.f)),
      rod_(sf::Vector2f(450.f, 590.f)),
      mouse_(450.f, 200.f) {
    window_.setFramerateLimit(60);
    std::cout << "Fishing Game\n"
              << "  Move the mouse to aim the rod.\n"
              << "  Left click: cast the line onto the pond.\n"
              << "  When the bobber dips, click quickly to hook the fish!\n"
              << "  Click while it is just floating to reel the line back in.\n"
              << "  Esc: quit.\n";
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // avoid huge jumps after a window drag
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            window_.close();
        } else if (event.type == sf::Event::MouseButtonPressed &&
                   event.mouseButton.button == sf::Mouse::Left) {
            rod_.click(pond_, window_.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y)));
        }
    }
}

void Game::update(float dt) {
    mouse_ = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    rod_.aimAt(mouse_);
    rod_.update(dt, pond_);
    pond_.update(dt);

    if (rod_.catches() != shownCatches_) {
        shownCatches_ = rod_.catches();
        window_.setTitle("Fishing Game - Fish caught: " + std::to_string(shownCatches_));
    }
}

void Game::drawScenery() {
    // Grass is the clear colour; add a dirt path and a wooden dock for the angler to stand on.
    sf::RectangleShape path(sf::Vector2f(900.f, 110.f));
    path.setPosition(0.f, 540.f);
    path.setFillColor(sf::Color(130, 110, 78));
    window_.draw(path);

    sf::RectangleShape dock(sf::Vector2f(150.f, 110.f));
    dock.setPosition(375.f, 540.f);
    dock.setFillColor(sf::Color(140, 100, 55));
    dock.setOutlineColor(sf::Color(90, 60, 30));
    dock.setOutlineThickness(3.f);
    window_.draw(dock);

    for (int i = 1; i < 6; ++i) { // plank lines
        sf::RectangleShape plank(sf::Vector2f(150.f, 2.f));
        plank.setPosition(375.f, 540.f + static_cast<float>(i) * 18.f);
        plank.setFillColor(sf::Color(90, 60, 30));
        window_.draw(plank);
    }
}

void Game::render() {
    window_.clear(sf::Color(96, 150, 80));
    pond_.draw(window_);
    drawScenery();
    rod_.draw(window_);
    window_.display();
}
