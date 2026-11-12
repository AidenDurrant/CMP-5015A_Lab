// simulation.h -- runs a rocket from launch to landing and records the result.

#pragma once

#include <ostream>

#include "plot.h"
#include "rocket.h"
#include "telemetry.h"
#include "vec2.h"

struct FlightStats
{
    double apogee = 0.0;
    double apogeeTime = 0.0;
    double maxSpeed = 0.0;
    double burnoutTime = 0.0;
    double range = 0.0;
    double flightTime = 0.0;
};

// Week 8 (Hard): ordering by apogee, so flights can be sorted (week 9).
bool operator<(const FlightStats& a, const FlightStats& b);
std::ostream& operator<<(std::ostream& out, const FlightStats& stats);

struct SimSettings
{
    double dt = 0.01;
    double maxTime = 300.0;
    double printEvery = 1.0;
    Vec2 wind;   // m/s, +x = tailwind for a rocket launched towards +x
};

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot, TelemetryLog& log);
