#include "engine.h"

#include "rocket.h"

double SolidMotor::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? rocket.activeStage().thrust : 0.0;
}

double SolidMotor::burnRate(const Rocket& rocket) const
{
    return rocket.engineRunning() ? rocket.activeStage().burnRate : 0.0;
}

double LiquidEngine::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? rocket.activeStage().thrust * throttle_ : 0.0;
}

double LiquidEngine::burnRate(const Rocket& rocket) const
{
    return rocket.engineRunning() ? rocket.activeStage().burnRate * throttle_ : 0.0;
}

void LiquidEngine::setThrottle(double fraction)
{
    throttle_ = fraction < 0.0 ? 0.0 : (fraction > 1.0 ? 1.0 : fraction);
}
