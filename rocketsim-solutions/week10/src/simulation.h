// simulation.h -- Week 10.

#pragma once

#include <memory>
#include <ostream>

#include "events.h"
#include "plot.h"
#include "rocket.h"
#include "telemetry.h"
#include "atmosphere.h"

struct FlightStats
{
    double apogee = 0.0;
    double apogeeTime = 0.0;
    double maxSpeed = 0.0;
    double burnoutTime = 0.0;
    double range = 0.0;
    double flightTime = 0.0;
    double landingSpeed = 0.0;
};

bool operator<(const FlightStats& a, const FlightStats& b);
std::ostream& operator<<(std::ostream& out, const FlightStats& stats);

struct SimSettings
{
    double dt = 0.01;
    double maxTime = 300.0;
    double printEvery = 1.0;
    bool parachute = false;   // deploy at apogee (Hard tier)
    std::shared_ptr<Atmosphere> atmosphere = std::make_shared<Atmosphere>();
    EventList events;
};

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot, TelemetryLog& log);
