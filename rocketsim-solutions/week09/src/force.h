// force.h -- Week 8 hierarchy. Week 9: Drag reads the wind from a profile.

#pragma once

#include <string>

#include "rocket.h"
#include "vec2.h"
#include "wind.h"

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

class Thrust : public Force
{
public:
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "thrust"; }
};

class Drag : public Force
{
public:
    explicit Drag(const WindProfile& profile);
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "drag"; }

private:
    const WindProfile& profile_;   // owned by SimSettings, which outlives the flight
};
