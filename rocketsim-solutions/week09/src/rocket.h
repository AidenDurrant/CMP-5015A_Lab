// rocket.h -- Week 9: a Rocket is a payload plus a std::vector<Stage>. The
// bottom stage burns first and is dropped when empty.

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "rocket_spec.h"
#include "vec2.h"

class Engine;

class Rocket
{
public:
    Rocket(const RocketSpec& spec, Engine* engine);

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

    static int liveCount();

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
    Engine* engine_;
};
