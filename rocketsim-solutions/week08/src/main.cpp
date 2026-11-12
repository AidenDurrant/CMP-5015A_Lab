// main.cpp -- Week 8 model solution.
//
//   rocketsim [--launch FILE] [--quiet] [--no-window] [--out NAME] [--compare]
//   --compare flies the same design with a solid motor and a liquid engine
//   on one plot (medium tier).

#include <iostream>
#include <string>

#include "config.h"
#include "engine.h"
#include "plot.h"
#include "rocket.h"
#include "simulation.h"
#include "telemetry.h"

int main(int argc, char* argv[])
{
    RocketSpec spec;
    SimSettings settings;
    std::string outName = "flight";
    bool showWindow = true;
    bool compare = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--quiet")                            settings.printEvery = 0.0;
        else if (arg == "--no-window")                   showWindow = false;
        else if (arg == "--compare")                     compare = true;
        else if (arg == "--launch" && i + 1 < argc)      loadLaunchFile(argv[++i], spec, settings);
        else if (arg == "--out" && i + 1 < argc)         outName = argv[++i];
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    std::cout << "Live rockets at start: " << Rocket::liveCount() << '\n';
    Plot plot("Flight path of " + spec.name);
    TelemetryLog log;

    {
        SolidMotor motor(spec.thrust, spec.burnRate);
        Rocket rocket(spec, &motor);
        std::cout << "Launching " << rocket.name() << " (" << motor.name() << ")...\n";
        const FlightStats stats = simulate(rocket, settings, plot, log);
        std::cout << "\n--- " << rocket.name() << " flight summary ---\n" << stats;

        if (compare)
        {
            LiquidEngine engine(spec.thrust, spec.burnRate);
            RocketSpec liquidSpec = spec;
            liquidSpec.name += "-liquid";
            Rocket rocket2(liquidSpec, &engine);
            TelemetryLog log2;
            const FlightStats stats2 = simulate(rocket2, settings, plot, log2);
            std::cout << "\n--- " << rocket2.name() << " flight summary ---\n" << stats2;
            std::cout << (stats < stats2 ? "The liquid engine flew higher.\n"
                                         : "The solid motor flew higher.\n");
        }

        // Medium tier demonstration: this copy used to crash the program.
        TelemetryLog copy = log;
        std::cout << "Deep copies made so far: " << TelemetryLog::copyCount()
                  << " (copy has " << copy.size() << " samples)\n";
    }   // rockets destroyed here

    if (writeCsv(log, outName + ".csv"))
    {
        std::cout << "Wrote " << outName << ".csv with " << log.size() << " samples\n";
    }
    if (plot.saveSvg(outName + ".svg"))
    {
        std::cout << "Wrote " << outName << ".svg\n";
    }
    plot.printAscii();
    std::cout << "Live rockets at end: " << Rocket::liveCount() << '\n';

    if (showWindow)
    {
        plot.show();
    }
    return 0;
}
