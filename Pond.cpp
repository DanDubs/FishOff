#include "Pond.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cmath>

Pond::Pond(sf::Vector2f centre, sf::Vector2f radii) : centre_(centre), radii_(radii) {}

void Pond::update(float dt) {
    time_ += dt;
    for (auto& r : ripples_) r.age += dt;
    ripples_.erase(std::remove_if(ripples_.begin(), ripples_.end(),
                                  [](const Ripple& r) { return r.age >= r.maxAge; }),
                   ripples_.end());
}

void Pond::addRipple(sf::Vector2f pos, float strength) {
    Ripple r;
    r.pos = pos;
    r.age = 0.f;
    r.maxAge = 0.9f + 0.9f * strength;
    r.maxRadius = 14.f + 40.f * strength;
    ripples_.push_back(r);
}

bool Pond::contains(sf::Vector2f p) const {
    const float dx = (p.x - centre_.x) / radii_.x;
    const float dy = (p.y - centre_.y) / radii_.y;
    return dx * dx + dy * dy <= 1.f;
}

sf::Vector2f Pond::clampInside(sf::Vector2f p, float maxFraction) const {
    const float dx = (p.x - centre_.x) / radii_.x;
    const float dy = (p.y - centre_.y) / radii_.y;
    const float k = std::sqrt(dx * dx + dy * dy);
    if (k <= maxFraction) return p;
    const float s = maxFraction / k;
    return sf::Vector2f(centre_.x + dx * s * radii_.x, centre_.y + dy * s * radii_.y);
}

void Pond::draw(sf::RenderTarget& target) const {
    // Muddy bank, then the water, then a darker "deep" area in the middle.
    sf::ConvexShape bank = util::makeEllipse(centre_, radii_ * 1.08f);
    bank.setFillColor(sf::Color(122, 100, 68));
    target.draw(bank);

    sf::ConvexShape shallows = util::makeEllipse(centre_, radii_);
    shallows.setFillColor(sf::Color(70, 150, 185));
    target.draw(shallows);

    sf::ConvexShape deep = util::makeEllipse(centre_, radii_ * 0.72f);
    deep.setFillColor(sf::Color(40, 110, 160));
    target.draw(deep);

    // Drifting highlights so the water looks alive.
    for (int i = 0; i < 24; ++i) {
        const float a = static_cast<float>(i) * 2.4f;
        const float r = 0.25f + 0.6f * static_cast<float>((i * 37) % 100) / 100.f;
        const float drift = std::sin(time_ * 0.6f + static_cast<float>(i)) * 7.f;
        const sf::Vector2f p(centre_.x + std::cos(a) * radii_.x * r + drift,
                             centre_.y + std::sin(a) * radii_.y * r);
        const float alpha = 45.f + 35.f * std::sin(time_ * 1.2f + static_cast<float>(i) * 1.7f);
        util::drawThickLine(target, p, p + sf::Vector2f(18.f, 0.f), 2.f,
                            sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
    }

    // Lily pads (fixed positions relative to the pond so they scale with it).
    const float padA[] = {0.6f, 2.2f, 3.9f, 5.1f, 1.4f};
    const float padR[] = {0.62f, 0.55f, 0.7f, 0.45f, 0.78f};
    for (int i = 0; i < 5; ++i) {
        const sf::Vector2f p(centre_.x + std::cos(padA[i]) * radii_.x * padR[i],
                             centre_.y + std::sin(padA[i]) * radii_.y * padR[i] +
                                 std::sin(time_ * 0.8f + static_cast<float>(i)) * 1.5f);
        sf::ConvexShape pad = util::makeEllipse(p, sf::Vector2f(15.f, 9.f), 20);
        pad.setFillColor(sf::Color(60, 140, 70));
        pad.setOutlineColor(sf::Color(40, 105, 55));
        pad.setOutlineThickness(1.5f);
        target.draw(pad);
    }

    // Ripples: squashed rings (the view is slightly top-down, so circles look like ellipses).
    for (const auto& rp : ripples_) {
        const float t = rp.age / rp.maxAge;
        for (int ring = 0; ring < 2; ++ring) {
            const float scale = ring == 0 ? 1.f : 0.6f;
            const float rad = rp.maxRadius * t * scale;
            if (rad < 1.f) continue;
            sf::ConvexShape ellipse = util::makeEllipse(rp.pos, sf::Vector2f(rad, rad * 0.55f), 32);
            ellipse.setFillColor(sf::Color::Transparent);
            ellipse.setOutlineThickness(2.f);
            const float alpha = 170.f * (1.f - t);
            ellipse.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
            target.draw(ellipse);
        }
    }
}
