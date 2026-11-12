// engine.cpp -- Week 8.

#include "engine.h"

#include "rocket.h"

SolidMotor::SolidMotor(double thrust, double burnRate) : thrust_(thrust), burnRate_(burnRate) {}

double SolidMotor::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? thrust_ : 0.0;
}

double SolidMotor::burnRate(const Rocket& rocket) const
{
    return rocket.engineRunning() ? burnRate_ : 0.0;
}

std::string SolidMotor::name() const
{
    return "solid motor";
}

LiquidEngine::LiquidEngine(double thrust, double burnRate) : thrust_(thrust), burnRate_(burnRate) {}

double LiquidEngine::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? thrust_ * throttle_ : 0.0;
}

double LiquidEngine::burnRate(const Rocket& rocket) const
{
    return rocket.engineRunning() ? burnRate_ * throttle_ : 0.0;
}

std::string LiquidEngine::name() const
{
    return "liquid engine";
}

void LiquidEngine::setThrottle(double fraction)
{
    if (fraction < 0.0)
    {
        fraction = 0.0;
    }
    if (fraction > 1.0)
    {
        fraction = 1.0;
    }
    throttle_ = fraction;
}
