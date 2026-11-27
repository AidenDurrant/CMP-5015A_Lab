#include "engine.h"

#include "rocket.h"
#include "util.h"

#include <functional>
#include <map>
#include <stdexcept>

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
    throttle_ = clamp(fraction, 0.0, 1.0);
}

std::unique_ptr<Engine> makeEngine(const std::string& type)
{
    // One lambda per engine type. They use nothing from the surrounding
    // function, so they need no captures. find, not operator[], so an unknown
    // name is not inserted as an empty std::function.
    static const std::map<std::string, std::function<std::unique_ptr<Engine>()>> makers = {
        {"solid", [] { return std::make_unique<SolidMotor>(); }},
        {"liquid", [] { return std::make_unique<LiquidEngine>(); }},
    };
    const auto found = makers.find(type);
    if (found == makers.end())
    {
        throw std::invalid_argument("unknown engine type '" + type + "'");
    }
    return found->second();
}
