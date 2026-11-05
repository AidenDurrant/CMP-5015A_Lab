// main.cpp -- rocketsim entry point. GIVEN.
//
//   rocketsim                  fly the default rocket, write flight.svg
//   rocketsim --launch FILE    fly the rocket described in FILE (Week 7, Hard)
//   rocketsim --demo           plot a canned parabola to check the plotter works
//   rocketsim --quiet          no per-step telemetry
//   rocketsim --no-window      skip the live window (write the SVG only)
//
// You will change this file as the weeks go on; it is deliberately short.

#include <cmath>
#include <iostream>
#include <string>

#include "config.h"
#include "plot.h"
#include "rocket.h"
#include "simulation.h"
#include "telemetry.h"

namespace
{
    // A ballistic parabola with no drag or thrust, so you can see what a
    // correct plot looks like before any physics has been written.
    void runDemo()
    {
        Plot plot("rocketsim demo: 120 m/s at 60 degrees, no drag");
        plot.startSeries("demo");
        const double g = 9.81;
        const double v0 = 120.0;
        const double angle = 60.0 * 3.14159265358979323846 / 180.0;
        const double vx = v0 * std::cos(angle);
        const double vy = v0 * std::sin(angle);
        const double flightTime = 2.0 * vy / g;
        for (double t = 0.0; t <= flightTime; t += 0.05)
        {
            plot.addPoint(vx * t, vy * t - 0.5 * g * t * t);
        }
        plot.addMarker(vx * flightTime / 2.0, vy * vy / (2.0 * g), "apogee");
        plot.saveSvg("flight.svg");
        plot.printAscii();
        std::cout << "Wrote flight.svg (open it in a browser)\n";
        plot.show();
    }
}

int main(int argc, char* argv[])
{
    Rocket rocket;          // default rocket, see rocket.h
    SimSettings settings;   // default settings, see simulation.h
    std::string outName = "flight";   // Week 7 (Hard): --out NAME
    bool showWindow = true;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--demo")
        {
            runDemo();
            return 0;
        }
        else if (arg == "--quiet")
        {
            settings.printEvery = 0.0;
        }
        else if (arg == "--no-window")
        {
            showWindow = false;
        }
        else if (arg == "--launch" && i + 1 < argc)
        {
            loadLaunchFile(argv[++i], rocket, settings);
        }
        else if (arg == "--out" && i + 1 < argc)
        {
            outName = argv[++i];
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    std::cout << "Launching " << rocket.name << "...\n";

    Plot plot("Flight path of " + rocket.name);
    TelemetryLog log = createLog(1024);
    const FlightStats stats = simulate(rocket, settings, plot, log);

    printStats(rocket, stats);
    plot.printAscii();

    if (writeCsv(log, outName + ".csv"))
    {
        std::cout << "Wrote " << outName << ".csv with " << log.size << " samples\n";
    }
    destroyLog(log);

    if (plot.saveSvg(outName + ".svg"))
    {
        std::cout << "Wrote " << outName << ".svg with " << plot.pointCount()
                  << " points (open it in a browser)\n";
    }
    else
    {
        std::cerr << "Could not write " << outName << ".svg\n";
        return 1;
    }
    if (showWindow)
    {
        plot.show();   // returns when the window is closed
    }
    return 0;
}
