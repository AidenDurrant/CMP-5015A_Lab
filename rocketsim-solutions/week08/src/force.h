// force.h -- Week 8 (Hard): every force is an object with the same interface,
// so simulate() can sum a list of them without knowing what they are.

#pragma once

#include <string>

#include "rocket.h"
#include "vec2.h"

class Force
{
public:
    virtual ~Force() = default;
    virtual Vec2 apply(const Rocket& rocket) const = 0;
    virtual std::string name() const = 0;
};

class Gravity : public Force
{
public:
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "gravity"; }
};

// Thrust along the launch angle until PITCH_OVER_ALTITUDE, then along the
// velocity (a gravity turn).
class Thrust : public Force
{
public:
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "thrust"; }
};

// Drag on the velocity relative to the air, so a wind shifts the landing.
class Drag : public Force
{
public:
    explicit Drag(const Vec2& wind = Vec2{});
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "drag"; }

private:
    Vec2 wind_;
};
