#pragma once
// Small helpers shared by the fishing game (SFML 2.6.x).
#include <SFML/Graphics.hpp>
#include <cmath>

namespace util {

constexpr float PI = 3.14159265358979f;

inline float length(sf::Vector2f v) { return std::sqrt(v.x * v.x + v.y * v.y); }

inline sf::Vector2f normalised(sf::Vector2f v) {
    const float l = length(v);
    if (l <= 0.0001f) return sf::Vector2f(0.f, -1.f);
    return sf::Vector2f(v.x / l, v.y / l);
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline sf::Vector2f lerp(sf::Vector2f a, sf::Vector2f b, float t) {
    return sf::Vector2f(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Draws a straight line segment with thickness (a rotated rectangle).
inline void drawThickLine(sf::RenderTarget& target, sf::Vector2f p0, sf::Vector2f p1,
                          float thickness, sf::Color colour) {
    const sf::Vector2f d = p1 - p0;
    const float len = length(d);
    sf::RectangleShape rect(sf::Vector2f(len + 0.5f, thickness));
    rect.setOrigin(0.f, thickness * 0.5f);
    rect.setPosition(p0);
    rect.setRotation(std::atan2(d.y, d.x) * 180.f / PI);
    rect.setFillColor(colour);
    target.draw(rect);
}

// Builds a filled ellipse as a convex shape (so outlines don't get squashed by scaling).
inline sf::ConvexShape makeEllipse(sf::Vector2f centre, sf::Vector2f radii, std::size_t points = 64) {
    sf::ConvexShape shape(points);
    for (std::size_t i = 0; i < points; ++i) {
        const float a = 2.f * PI * static_cast<float>(i) / static_cast<float>(points);
        shape.setPoint(i, sf::Vector2f(centre.x + std::cos(a) * radii.x,
                                       centre.y + std::sin(a) * radii.y));
    }
    return shape;
}

} // namespace util
