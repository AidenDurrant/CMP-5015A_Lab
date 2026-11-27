// main.cpp -- Week 10 model solution.
//
//   rocketsim [--launch FILE]... [--quiet] [--no-window] [--out NAME] [--events]
//
// The fleet is a vector<shared_ptr<Rocket>> shared with MissionControl
// (weak_ptr); every Drag force shares one Atmosphere.

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "engine.h"
#include "mission_control.h"
#include "plot.h"
#include "rocket.h"
#include "simulation.h"
#include "telemetry.h"
#include "util.h"

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

    // One atmosphere for everyone.
    auto atmosphere = std::make_shared<Atmosphere>();
    std::cout << "Atmosphere use_count before the fleet: " << atmosphere.use_count() << '\n';

    std::vector<std::pair<std::shared_ptr<Rocket>, SimSettings>> fleet;
    MissionControl control;
    auto addRocket = [&](const RocketSpec& spec, SimSettings settings) {
        settings.atmosphere = atmosphere;
        settings.printEvery = quiet ? 0.0 : settings.printEvery;
        auto rocket = std::make_shared<Rocket>(spec);
        control.track(rocket);
        fleet.emplace_back(rocket, settings);
    };
    if (launchFiles.empty())
    {
        addRocket(RocketSpec{}, SimSettings{});
    }
    for (const std::string& file : launchFiles)
    {
        RocketSpec spec;
        SimSettings settings;
        if (loadLaunchFile(file, spec, settings))
        {
            addRocket(spec, settings);
        }
    }
    std::cout << "Atmosphere use_count with " << fleet.size() << " rockets: " << atmosphere.use_count() << '\n';

    Plot plot(fleet.size() == 1 ? "Flight path of " + fleet.front().first->name() : "Fleet comparison");
    std::vector<std::pair<std::string, FlightStats>> results;
    TelemetryLog lastLog;

    for (auto& [rocket, settings] : fleet)
    {
        if (withEvents)
        {
            const double throttle = clamp(0.5, 0.0, 1.0);
            settings.events.push_back({20.0, "throttle 50%", [throttle](Rocket& r) { r.engine().setThrottle(throttle); }});
            settings.events.push_back({30.0, "fairing off", [](Rocket& r) { r.jettison(5.0); }});
        }
        std::cout << "Launching " << rocket->name() << " (" << rocket->engine().name() << ")...\n";
        TelemetryLog log;
        const FlightStats stats = simulate(*rocket, settings, plot, log);
        std::cout << "\n--- " << rocket->name() << " flight summary ---\n" << stats;
        printLogSummary(log, settings.dt);
        results.emplace_back(rocket->name(), stats);
        lastLog = log;
    }

    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.second.apogee > b.second.apogee; });
    std::cout << "\nRanking by apogee:\n";
    int place = 1;
    for (const auto& [name, stats] : results)
    {
        std::cout << "  " << place++ << ". " << name << "  " << stats.apogee << " m\n";
    }

    std::cout << "Mission control: " << control.alive() << " of " << control.tracked() << " rockets alive\n";
    fleet.clear();   // the shared_ptrs go; the weak_ptrs notice
    std::cout << "Mission control after clearing the fleet: " << control.alive() << " alive\n";
    std::cout << "Atmosphere use_count now: " << atmosphere.use_count() << '\n';

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
