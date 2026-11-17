#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// An elliptical pond with a bank, lily pads, shimmering water and ripples.
class Pond {
public:
    Pond(sf::Vector2f centre, sf::Vector2f radii);

    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    // Spawns an expanding ripple. strength scales its size (1 = splash, 0.4 = tiny nibble).
    void addRipple(sf::Vector2f pos, float strength = 1.f);

    bool contains(sf::Vector2f p) const;

    // Returns p if it is comfortably inside the water, otherwise the nearest point that is.
    sf::Vector2f clampInside(sf::Vector2f p, float maxFraction = 0.88f) const;

private:
    struct Ripple {
        sf::Vector2f pos;
        float age;
        float maxAge;
        float maxRadius;
    };

    sf::Vector2f centre_;
    sf::Vector2f radii_;
    float time_ = 0.f;
    std::vector<Ripple> ripples_;
};
