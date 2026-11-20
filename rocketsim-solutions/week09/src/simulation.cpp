// simulation.cpp -- Week 9: statistics come from <algorithm> after the loop;
// events are a sorted vector processed as the clock passes them.

#include "simulation.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "engine.h"
#include "force.h"

bool operator<(const FlightStats& a, const FlightStats& b)
{
    return a.apogee < b.apogee;
}

std::ostream& operator<<(std::ostream& out, const FlightStats& stats)
{
    out << std::fixed << std::setprecision(1);
    out << "  apogee      " << std::setw(9) << stats.apogee << " m at t = " << stats.apogeeTime << " s\n";
    out << "  max speed   " << std::setw(9) << stats.maxSpeed << " m/s\n";
    out << "  burnout     " << std::setw(9) << stats.burnoutTime << " s\n";
    out << "  range       " << std::setw(9) << stats.range << " m\n";
    out << "  flight time " << std::setw(9) << stats.flightTime << " s\n";
    out << "  landing     " << std::setw(9) << stats.landingSpeed << " m/s\n";
    return out;
}

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot, TelemetryLog& log)
{
    FlightStats stats;
    bool launched = false;
    double nextPrint = 0.0;
    const long maxSteps = static_cast<long>(settings.maxTime / settings.dt) + 1;

    Force* forces[3] = {new Gravity(), new Thrust(), new Drag(settings.wind)};

    // Events, soonest first. A copy, because we consume it.
    EventList pending = settings.events;
    std::sort(pending.begin(), pending.end(), [](const Event& a, const Event& b) { return a.time < b.time; });
    std::size_t nextEvent = 0;

    plot.startSeries(rocket.name());
    int stageNumber = 1;

    for (long stepCount = 0; stepCount < maxSteps && rocket.time() < settings.maxTime; ++stepCount)
    {
        if (settings.printEvery > 0.0 && rocket.time() >= nextPrint)
        {
            std::cout << rocket << '\n';
            nextPrint += settings.printEvery;
        }

        while (nextEvent < pending.size() && pending[nextEvent].time <= rocket.time())
        {
            const Event& event = pending[nextEvent++];
            event.action(rocket);
            plot.addMarker(rocket.position().x, rocket.position().y, event.label);
        }

        Vec2 total;
        for (const Force* force : forces)
        {
            total += force->apply(rocket);
        }
        rocket.update(total, settings.dt);

        plot.addPoint(rocket.position().x, rocket.position().y, rocket.time());
        const double speed = length(rocket.velocity());
        log.push_back(Sample{rocket.time(), rocket.position().x, rocket.position().y, speed, rocket.fuel()});

        if (rocket.separatedThisStep())
        {
            plot.addMarker(rocket.position().x, rocket.position().y, "stage " + std::to_string(stageNumber++) + " separation");
        }
        if (stats.burnoutTime == 0.0 && !rocket.engineRunning() && rocket.stagesRemaining() == 1)
        {
            stats.burnoutTime = rocket.time();
            plot.addMarker(rocket.position().x, rocket.position().y, "burnout");
        }
        if (settings.parachute && launched && !rocket.parachuteDeployed() && rocket.velocity().y < 0.0)
        {
            rocket.deployParachute();
            plot.addMarker(rocket.position().x, rocket.position().y, "parachute");
        }
        if (rocket.position().y > 0.0)
        {
            launched = true;
        }
        if (launched && rocket.position().y <= 0.0)
        {
            break;
        }
    }

    for (Force* force : forces)
    {
        delete force;
    }

    if (!launched)
    {
        std::cout << "The rocket never left the pad.\n";
    }

    // Easy 2: statistics from the log, not from bookkeeping in the loop.
    if (!log.empty())
    {
        const auto highest = std::max_element(log.begin(), log.end(),
                                              [](const Sample& a, const Sample& b) { return a.y < b.y; });
        stats.apogee = highest->y;
        stats.apogeeTime = highest->time;
        plot.addMarker(highest->x, highest->y, "apogee");
        stats.maxSpeed = std::max_element(log.begin(), log.end(),
                                          [](const Sample& a, const Sample& b) { return a.speed < b.speed; })->speed;
        stats.landingSpeed = log.back().speed;
    }
    stats.flightTime = rocket.time();
    stats.range = rocket.position().x;
    return stats;
}
