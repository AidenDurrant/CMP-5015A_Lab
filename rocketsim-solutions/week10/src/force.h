// force.h -- Week 8 hierarchy. Week 10: Drag shares the Atmosphere.

#pragma once

#include <memory>
#include <string>

#include "rocket.h"
#include "vec2.h"
#include "atmosphere.h"

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
    explicit Drag(std::shared_ptr<const Atmosphere> atmosphere);
    Vec2 apply(const Rocket& rocket) const override;
    std::string name() const override { return "drag"; }

private:
    std::shared_ptr<const Atmosphere> atmosphere_;   // shared, never dangles
};
