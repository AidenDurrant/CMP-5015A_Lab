// simulation.cpp -- the main simulation loop. The loop is GIVEN; the TODOs are
// the Week 7 telemetry (Easy) and flight statistics (Medium).

#include "simulation.h"

#include <iomanip>
#include <iostream>

#include "physics.h"

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot)
{
    FlightStats stats;

    // Thrust is only "on" once; after that the rocket is in free flight. We
    // run until it comes back below ground level or we hit the time limit.
    bool launched = false;
    double nextPrint = 0.0;

    // Belt and braces: even if step() forgets to advance the clock we stop.
    const long maxSteps = static_cast<long>(settings.maxTime / settings.dt) + 1;

    plot.startSeries(rocket.name);

    for (long stepCount = 0; stepCount < maxSteps && rocket.time < settings.maxTime; ++stepCount)
    {
        // TODO (Week 7, Easy): every `settings.printEvery` seconds print one
        // telemetry line with std::cout, std::setw and std::setprecision:
        //     t=  12.00 s  alt=  1234.5 m  range=   56.7 m  speed=  123.4 m/s  fuel=  30.0 kg
        if (settings.printEvery > 0.0 && rocket.time >= nextPrint)
        {
            nextPrint += settings.printEvery;
        }

        physics::step(rocket, settings.dt);
        plot.addPoint(rocket.position.x, rocket.position.y, rocket.time);

        // TODO (Week 7, Medium): update stats.apogee / apogeeTime / maxSpeed /
        // burnoutTime as the flight goes on. Add a plot marker at apogee and at
        // burnout with plot.addMarker(x, y, "apogee").

        if (rocket.position.y > 0.0)
        {
            launched = true;
        }
        if (launched && rocket.position.y <= 0.0)
        {
            break;  // touchdown
        }
    }

    if (!launched)
    {
        std::cout << "The rocket never left the pad. Have you implemented physics::step?\n";
    }

    stats.flightTime = rocket.time;
    stats.range = rocket.position.x;
    return stats;
}

void printStats(const Rocket& rocket, const FlightStats& stats)
{
    // GIVEN: a small example of <iomanip> formatting, c.f. printf's %8.1f.
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "\n--- " << rocket.name << " flight summary ---\n";
    std::cout << "  apogee      " << std::setw(9) << stats.apogee << " m at t = "
              << stats.apogeeTime << " s\n";
    std::cout << "  max speed   " << std::setw(9) << stats.maxSpeed << " m/s\n";
    std::cout << "  burnout     " << std::setw(9) << stats.burnoutTime << " s\n";
    std::cout << "  range       " << std::setw(9) << stats.range << " m\n";
    std::cout << "  flight time " << std::setw(9) << stats.flightTime << " s\n";
}
