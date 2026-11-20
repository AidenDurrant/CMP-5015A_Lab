// simulation.h -- Week 9.

#pragma once

#include <ostream>

#include "events.h"
#include "plot.h"
#include "rocket.h"
#include "telemetry.h"
#include "wind.h"

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
    WindProfile wind{{0.0, 2.0}, {1000.0, 8.0}, {5000.0, 20.0}, {15000.0, 35.0}};
    EventList events;
};

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot, TelemetryLog& log);
