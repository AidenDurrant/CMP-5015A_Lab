// engine.h -- Week 8: an abstract Engine and two concrete kinds. The rocket
// asks its engine for thrust and burn rate every step and never knows which
// kind it has: that is dynamic binding (lecture 10's vtable, now in C++).

#pragma once

#include <string>

class Rocket;

class Engine
{
public:
    virtual ~Engine() = default;   // deleting through Engine* must run the right destructor

    virtual double thrust(const Rocket& rocket) const = 0;
    virtual double burnRate(const Rocket& rocket) const = 0;
    virtual std::string name() const = 0;

    // Default behaviour: most engines cannot be throttled, so ignore it.
    virtual void setThrottle(double /*fraction*/) {}
};

// Burns at a fixed rate until the fuel is gone: the week 7 behaviour.
class SolidMotor : public Engine
{
public:
    SolidMotor(double thrust, double burnRate);
    double thrust(const Rocket& rocket) const override;
    double burnRate(const Rocket& rocket) const override;
    std::string name() const override;

private:
    double thrust_;
    double burnRate_;
};

// Same, but thrust and burn rate scale with a throttle setting in [0, 1].
class LiquidEngine : public Engine
{
public:
    LiquidEngine(double thrust, double burnRate);
    double thrust(const Rocket& rocket) const override;
    double burnRate(const Rocket& rocket) const override;
    std::string name() const override;
    void setThrottle(double fraction) override;
    double throttle() const { return throttle_; }

private:
    double thrust_;
    double burnRate_;
    double throttle_ = 1.0;
};
