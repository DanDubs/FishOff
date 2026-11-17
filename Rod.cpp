#include "Rod.hpp"
#include "Pond.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cmath>

using util::PI;

Rod::Rod(sf::Vector2f base)
    : base_(base), bobber_(base), castStart_(base), castTarget_(base), rng_(std::random_device{}()) {}

float Rod::randomRange(float lo, float hi) {
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(rng_);
}

void Rod::aimAt(sf::Vector2f target) {
    sf::Vector2f d = target - base_;
    if (d.y > -30.f) d.y = -30.f; // always point out over the water, never backwards
    dir_ = util::normalised(d);
}

sf::Vector2f Rod::rodPoint(float t) const {
    const sf::Vector2f perp(-dir_.y, dir_.x);
    return base_ + dir_ * (length_ * t) + perp * (bend_ * t * t);
}

sf::Vector2f Rod::bobberDrawPos() const {
    if (state_ == State::Floating) return bobber_ + sf::Vector2f(0.f, 2.f * std::sin(time_ * 3.f));
    if (state_ == State::Bite) return bobber_ + sf::Vector2f(0.f, 3.f + 4.f * std::fabs(std::sin(time_ * 16.f)));
    return bobber_;
}

void Rod::scheduleBite() {
    biteWait_ = randomRange(2.f, 6.f);
}

void Rod::click(Pond& pond, sf::Vector2f mouse) {
    switch (state_) {
    case State::Idle: {
        castStart_ = tip();
        castTarget_ = pond.clampInside(mouse);
        const float dist = util::length(castTarget_ - castStart_);
        castDuration_ = util::clampf(dist / 700.f, 0.35f, 0.9f);
        castT_ = 0.f;
        bobber_ = castStart_;
        state_ = State::Casting;
        break;
    }
    case State::Floating:
        reelHasFish_ = false;
        state_ = State::Reeling;
        break;
    case State::Bite:
        reelHasFish_ = true;
        ++catches_;
        pond.addRipple(bobber_, 1.f);
        state_ = State::Reeling;
        break;
    case State::Casting:
    case State::Reeling:
        break; // ignore clicks while the line is in motion
    }
}

void Rod::update(float dt, Pond& pond) {
    time_ += dt;

    // Where the rod "wants" to bend to, depending on what is happening on the line.
    float bendTarget = 4.f * std::sin(time_ * 1.3f);
    switch (state_) {
    case State::Casting:
        bendTarget = -35.f * std::sin(PI * std::min(castT_, 1.f));
        break;
    case State::Bite:
        bendTarget = 22.f + 6.f * std::sin(time_ * 35.f);
        break;
    case State::Reeling:
        bendTarget = reelHasFish_ ? 26.f + 5.f * std::sin(time_ * 22.f) : 10.f;
        break;
    default:
        break;
    }
    bend_ += (bendTarget - bend_) * std::min(1.f, dt * 12.f);

    switch (state_) {
    case State::Idle:
        bobber_ = tip();
        break;

    case State::Casting: {
        castT_ += dt / castDuration_;
        const float t = std::min(castT_, 1.f);
        bobber_ = util::lerp(castStart_, castTarget_, t);
        bobber_.y -= 120.f * std::sin(PI * t); // arc through the air
        if (castT_ >= 1.f) {
            bobber_ = castTarget_;
            pond.addRipple(bobber_, 1.f);
            state_ = State::Floating;
            scheduleBite();
        }
        break;
    }

    case State::Floating:
        biteWait_ -= dt;
        if (biteWait_ <= 0.f) {
            state_ = State::Bite;
            biteLeft_ = 1.2f;
            nibbleTimer_ = 0.f;
        }
        break;

    case State::Bite:
        biteLeft_ -= dt;
        nibbleTimer_ -= dt;
        if (nibbleTimer_ <= 0.f) {
            pond.addRipple(bobber_, 0.4f);
            nibbleTimer_ = 0.3f;
        }
        if (biteLeft_ <= 0.f) { // too slow, it spat the hook
            state_ = State::Floating;
            scheduleBite();
        }
        break;

    case State::Reeling: {
        const sf::Vector2f d = tip() - bobber_;
        const float dist = util::length(d);
        const float step = 480.f * dt;
        if (dist <= step + 10.f) {
            state_ = State::Idle;
            bobber_ = tip();
        } else {
            bobber_ += d / dist * step;
        }
        break;
    }
    }
}

void Rod::draw(sf::RenderTarget& target) const {
    drawLine(target);
    drawRod(target);
    if (state_ != State::Idle) drawBobber(target);
    drawFish(target);
}

void Rod::drawRod(sf::RenderTarget& target) const {
    const int segments = 14;
    for (int i = 0; i < segments; ++i) {
        const float t0 = static_cast<float>(i) / segments;
        const float t1 = static_cast<float>(i + 1) / segments;
        const float thickness = util::lerp(7.f, 2.f, t0);
        const sf::Color colour = i < 3 ? sf::Color(80, 50, 25) : sf::Color(150, 100, 50);
        util::drawThickLine(target, rodPoint(t0), rodPoint(t1), thickness, colour);
    }
    // Reel
    sf::CircleShape reel(8.f);
    reel.setOrigin(8.f, 8.f);
    reel.setPosition(rodPoint(0.14f));
    reel.setFillColor(sf::Color(60, 60, 70));
    reel.setOutlineColor(sf::Color(170, 170, 180));
    reel.setOutlineThickness(2.f);
    target.draw(reel);
}

void Rod::drawLine(sf::RenderTarget& target) const {
    sf::Vector2f end;
    float sag = 0.f;
    switch (state_) {
    case State::Idle:     end = tip() + sf::Vector2f(3.f, 38.f); sag = 0.f; break;
    case State::Casting:  end = bobber_;                         sag = 6.f; break;
    case State::Floating: end = bobberDrawPos();                 sag = 18.f; break;
    case State::Bite:     end = bobberDrawPos();                 sag = 4.f; break;
    case State::Reeling:  end = bobber_;                         sag = 3.f; break;
    }

    const int points = 24;
    sf::VertexArray line(sf::LineStrip, points + 1);
    const sf::Vector2f start = tip();
    for (int i = 0; i <= points; ++i) {
        const float t = static_cast<float>(i) / points;
        sf::Vector2f p = util::lerp(start, end, t);
        p.y += sag * std::sin(PI * t);
        line[i].position = p;
        line[i].color = sf::Color(235, 235, 235);
    }
    target.draw(line);

    if (state_ == State::Idle) { // bare hook
        sf::CircleShape hook(3.f);
        hook.setOrigin(3.f, 3.f);
        hook.setPosition(end);
        hook.setFillColor(sf::Color(190, 190, 200));
        target.draw(hook);
    }
}

void Rod::drawBobber(sf::RenderTarget& target) const {
    const sf::Vector2f p = bobberDrawPos();
    const float radius = state_ == State::Bite ? 6.f : 7.f;

    sf::CircleShape body(radius);
    body.setOrigin(radius, radius);
    body.setPosition(p);
    body.setFillColor(sf::Color(245, 245, 245));
    body.setOutlineColor(sf::Color(60, 60, 60));
    body.setOutlineThickness(1.f);
    target.draw(body);

    sf::CircleShape cap(radius * 0.6f);
    cap.setOrigin(radius * 0.6f, radius * 0.6f);
    cap.setPosition(p + sf::Vector2f(0.f, -radius * 0.35f));
    cap.setFillColor(sf::Color(215, 40, 40));
    target.draw(cap);
}

void Rod::drawFish(sf::RenderTarget& target) const {
    if (state_ != State::Reeling || !reelHasFish_) return;

    const sf::Vector2f toTip = tip() - bobber_;
    const float angle = std::atan2(toTip.y, toTip.x) * 180.f / PI;
    sf::Transform transform;
    transform.translate(bobber_);
    transform.rotate(angle);
    const sf::RenderStates states(transform);

    sf::ConvexShape body = util::makeEllipse(sf::Vector2f(0.f, 0.f), sf::Vector2f(17.f, 8.f), 20);
    body.setFillColor(sf::Color(235, 140, 50));
    body.setOutlineColor(sf::Color(160, 80, 20));
    body.setOutlineThickness(1.f);
    target.draw(body, states);

    sf::ConvexShape tail(3);
    tail.setPoint(0, sf::Vector2f(-14.f, 0.f));
    tail.setPoint(1, sf::Vector2f(-28.f, -9.f));
    tail.setPoint(2, sf::Vector2f(-28.f, 9.f));
    tail.setFillColor(sf::Color(220, 110, 40));
    target.draw(tail, states);

    sf::CircleShape eye(1.8f);
    eye.setOrigin(1.8f, 1.8f);
    eye.setPosition(10.f, -2.f);
    eye.setFillColor(sf::Color::Black);
    target.draw(eye, states);
}
