#pragma once
#include <SFML/Graphics.hpp>
#include <random>

class Pond;

// The fishing rod, line, bobber and the little state machine that drives them.
//
//   Idle     -> click -> Casting -> (lands) -> Floating
//   Floating -> (random wait) -> Bite -> click in time -> Reeling (with a fish)
//   Bite     -> too slow -> Floating again (the fish got away)
//   Floating -> click -> Reeling (empty hook)
//   Reeling  -> (bobber reaches the rod tip) -> Idle
class Rod {
public:
    enum class State { Idle, Casting, Floating, Bite, Reeling };

    explicit Rod(sf::Vector2f base);

    void aimAt(sf::Vector2f target);                 // point the rod towards the mouse
    void click(Pond& pond, sf::Vector2f mouse);      // cast / hook / reel depending on state
    void update(float dt, Pond& pond);
    void draw(sf::RenderTarget& target) const;

    State state() const { return state_; }
    int catches() const { return catches_; }

private:
    sf::Vector2f rodPoint(float t) const;            // point along the (bent) rod, t in [0,1]
    sf::Vector2f tip() const { return rodPoint(1.f); }
    sf::Vector2f bobberDrawPos() const;
    void scheduleBite();

    void drawRod(sf::RenderTarget& target) const;
    void drawLine(sf::RenderTarget& target) const;
    void drawBobber(sf::RenderTarget& target) const;
    void drawFish(sf::RenderTarget& target) const;

    float randomRange(float lo, float hi);

    // Rod geometry
    sf::Vector2f base_;
    sf::Vector2f dir_{0.f, -1.f};
    float length_ = 150.f;
    float bend_ = 0.f;

    // State
    State state_ = State::Idle;
    float time_ = 0.f;

    // Bobber / casting
    sf::Vector2f bobber_;
    sf::Vector2f castStart_;
    sf::Vector2f castTarget_;
    float castT_ = 0.f;
    float castDuration_ = 0.6f;

    // Fishing
    float biteWait_ = 0.f;
    float biteLeft_ = 0.f;
    float nibbleTimer_ = 0.f;
    bool reelHasFish_ = false;
    int catches_ = 0;

    std::mt19937 rng_;
};
