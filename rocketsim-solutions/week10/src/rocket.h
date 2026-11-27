// rocket.h -- Week 10: the Rocket owns its Engine through a unique_ptr, so
// it is destroyed with the rocket and the Rocket itself cannot be copied.

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "rocket_spec.h"
#include "vec2.h"

class Engine;

class Rocket
{
public:
    explicit Rocket(const RocketSpec& spec);

    // A Rocket owns its engine, so copying it would need a deep copy of a
    // polymorphic object. Say so explicitly instead of letting the compiler
    // produce a confusing error.
    Rocket(const Rocket&) = delete;
    Rocket& operator=(const Rocket&) = delete;
    Rocket(Rocket&&) noexcept;
    Rocket& operator=(Rocket&&) noexcept;
    ~Rocket();   // defined in rocket.cpp, where Engine is a complete type

    const std::string& name() const { return name_; }
    double mass() const;                        // payload + every remaining stage
    double fuel() const;                        // fuel in the active stage
    const Stage& activeStage() const { return stages_.front(); }
    std::size_t stagesRemaining() const { return stages_.size(); }
    double dragCoeff() const { return dragCoeff_; }
    double area() const { return area_ * dragAreaMultiplier_; }
    double launchAngle() const { return launchAngleDeg_; }
    const Vec2& position() const { return position_; }
    const Vec2& velocity() const { return velocity_; }
    double time() const { return time_; }
    bool engineRunning() const { return fuel() > 0.0; }
    bool parachuteDeployed() const { return dragAreaMultiplier_ > 1.0; }
    Engine& engine() const { return *engine_; }

    void update(const Vec2& totalForce, double dt);

    // True exactly once, on the step in which an empty stage was dropped.
    bool separatedThisStep() const { return separated_; }

    // Week 9 (Hard): things events can do to a rocket.
    void deployParachute(double areaMultiplier = 20.0) { dragAreaMultiplier_ = areaMultiplier; }
    void jettison(double massKg);   // drop part of the payload (a fairing)

    friend std::ostream& operator<<(std::ostream& out, const Rocket& rocket);

private:
    std::string name_;
    double payloadMass_;
    std::vector<Stage> stages_;
    double dragCoeff_;
    double area_;
    double dragAreaMultiplier_ = 1.0;
    double launchAngleDeg_;
    Vec2 position_;
    Vec2 velocity_;
    double time_ = 0.0;
    bool separated_ = false;
    std::unique_ptr<Engine> engine_;
};
