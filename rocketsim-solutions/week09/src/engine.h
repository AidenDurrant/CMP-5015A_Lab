// engine.h -- Week 8 hierarchy. Week 9: thrust and burn rate come from the
// rocket's active stage, so an engine object is just a "kind of engine".

#pragma once

#include <string>

class Rocket;

class Engine
{
public:
    virtual ~Engine() = default;
    virtual double thrust(const Rocket& rocket) const = 0;
    virtual double burnRate(const Rocket& rocket) const = 0;
    virtual std::string name() const = 0;
    virtual void setThrottle(double /*fraction*/) {}
};

class SolidMotor : public Engine
{
public:
    double thrust(const Rocket& rocket) const override;
    double burnRate(const Rocket& rocket) const override;
    std::string name() const override { return "solid motor"; }
};

class LiquidEngine : public Engine
{
public:
    double thrust(const Rocket& rocket) const override;
    double burnRate(const Rocket& rocket) const override;
    std::string name() const override { return "liquid engine"; }
    void setThrottle(double fraction) override;
    double throttle() const { return throttle_; }

private:
    double throttle_ = 1.0;
};
