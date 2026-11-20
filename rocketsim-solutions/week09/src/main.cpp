// main.cpp -- Week 9 model solution.
//
//   rocketsim [--launch FILE]... [--quiet] [--no-window] [--out NAME] [--events]
//
// Several --launch files make a fleet: all fly on one plot and are ranked.
// --events schedules a throttle-down at 20 s and a fairing jettison at 30 s.

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "engine.h"
#include "plot.h"
#include "rocket.h"
#include "simulation.h"
#include "telemetry.h"

int main(int argc, char* argv[])
{
    std::vector<std::string> launchFiles;
    std::string outName = "flight";
    bool showWindow = true;
    bool quiet = false;
    bool withEvents = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--quiet")                        quiet = true;
        else if (arg == "--no-window")               showWindow = false;
        else if (arg == "--events")                  withEvents = true;
        else if (arg == "--launch" && i + 1 < argc)  launchFiles.push_back(argv[++i]);
        else if (arg == "--out" && i + 1 < argc)     outName = argv[++i];
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    // Medium 3: a fleet keyed by name. Rocket has no default constructor, so
    // fleet[name] would not compile; emplace it is.
    std::map<std::string, RocketSpec> specs;
    std::map<std::string, SimSettings> settingsByName;
    if (launchFiles.empty())
    {
        specs.emplace("Starter-1", RocketSpec{});
        settingsByName.emplace("Starter-1", SimSettings{});
    }
    for (const std::string& file : launchFiles)
    {
        RocketSpec spec;
        SimSettings settings;
        if (loadLaunchFile(file, spec, settings))
        {
            specs.emplace(spec.name, spec);
            settingsByName.emplace(spec.name, settings);
        }
    }

    std::map<std::string, LiquidEngine> engines;   // one engine object per rocket
    std::map<std::string, Rocket> fleet;
    for (const auto& [name, spec] : specs)
    {
        engines.emplace(name, LiquidEngine{});
        fleet.emplace(name, Rocket(spec, &engines.at(name)));
    }

    Plot plot(fleet.size() == 1 ? "Flight path of " + fleet.begin()->first : "Fleet comparison");
    std::vector<std::pair<std::string, FlightStats>> results;
    TelemetryLog lastLog;

    for (auto& [name, rocket] : fleet)
    {
        SimSettings settings = settingsByName.at(name);
        if (quiet)
        {
            settings.printEvery = 0.0;
        }
        if (withEvents)   // Hard 1: lambdas capturing by value
        {
            const double throttle = 0.5;
            settings.events.push_back({20.0, "throttle 50%", [throttle](Rocket& r) { r.engine().setThrottle(throttle); }});
            settings.events.push_back({30.0, "fairing off", [](Rocket& r) { r.jettison(5.0); }});
        }
        std::cout << "Launching " << name << "...\n";
        TelemetryLog log;
        const FlightStats stats = simulate(rocket, settings, plot, log);
        std::cout << "\n--- " << name << " flight summary ---\n" << stats;
        printLogSummary(log, settings.dt);
        results.emplace_back(name, stats);
        lastLog = log;
    }

    // Medium 3: ranking by apogee with one sort call.
    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.second.apogee > b.second.apogee; });
    std::cout << "\nRanking by apogee:\n";
    int place = 1;
    for (const auto& [name, stats] : results)
    {
        std::cout << "  " << place++ << ". " << name << "  " << stats.apogee << " m\n";
    }

    if (writeCsv(lastLog, outName + ".csv"))
    {
        std::cout << "Wrote " << outName << ".csv with " << lastLog.size() << " samples\n";
    }
    plot.saveSvg(outName + ".svg");
    plot.printAscii();
    if (showWindow)
    {
        plot.show();
    }
    return 0;
}
