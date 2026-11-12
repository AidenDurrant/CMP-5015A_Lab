// rocket.h -- Week 8: Rocket is a class. State is private, read through const
// getters, and only update() changes it. The engine is a polymorphic object
// the rocket points to but (this week) does not own.

#pragma once

#include <ostream>
#include <string>

#include "rocket_spec.h"
#include "vec2.h"

class Engine;

class Rocket
{
public:
    Rocket(const RocketSpec& spec, Engine* engine);
    Rocket(const Rocket& other);
    Rocket& operator=(const Rocket& other) = default;
    ~Rocket();

    const std::string& name() const { return name_; }
    double dryMass() const { return dryMass_; }
    double fuel() const { return fuelMass_; }
    double mass() const { return dryMass_ + fuelMass_; }
    double dragCoeff() const { return dragCoeff_; }
    double area() const { return area_; }
    double launchAngle() const { return launchAngleDeg_; }
    const Vec2& position() const { return position_; }
    const Vec2& velocity() const { return velocity_; }
    double time() const { return time_; }
    bool engineRunning() const { return fuelMass_ > 0.0; }
    Engine& engine() const { return *engine_; }

    // Euler step: a = F/m, v += a dt, p += v dt, burn fuel, advance the clock.
    void update(const Vec2& totalForce, double dt);

    // How many Rocket objects exist right now (incremented by every
    // constructor, decremented by the destructor).
    static int liveCount();

    friend std::ostream& operator<<(std::ostream& out, const Rocket& rocket);

private:
    std::string name_;
    double dryMass_;
    double fuelMass_;
    double dragCoeff_;
    double area_;
    double launchAngleDeg_;
    Vec2 position_;
    Vec2 velocity_;
    double time_ = 0.0;
    Engine* engine_;

    static int liveCount_;
};
