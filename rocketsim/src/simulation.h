// simulation.h -- runs a rocket from launch to landing and records the result.

#pragma once

#include "plot.h"
#include "rocket.h"

// Summary numbers for one flight. Filled in by simulate() (Week 7, Medium).
struct FlightStats
{
    double apogee = 0.0;        // m, highest altitude reached
    double apogeeTime = 0.0;    // s
    double maxSpeed = 0.0;      // m/s
    double burnoutTime = 0.0;   // s, when the fuel ran out (0 if it never did)
    double range = 0.0;         // m, downrange distance at touchdown
    double flightTime = 0.0;    // s, total time in the air
};

// Simulation settings. `dt` is the integration step, `maxTime` a safety limit,
// `printEvery` how often (in seconds) a telemetry line is printed (0 = never).
struct SimSettings
{
    double dt = 0.01;
    double maxTime = 300.0;
    double printEvery = 1.0;
};

// Fly the rocket until it lands (altitude < 0) or maxTime is reached.
// Every step is added to `plot`. Returns the summary statistics.
FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot);

// Print the statistics in a readable form.
void printStats(const Rocket& rocket, const FlightStats& stats);
