// simulation.cpp -- Week 8. The forces are polymorphic objects held through
// raw pointers (hard tier); week 10 replaces the pointers with unique_ptr.

#include "simulation.h"

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
    return out;
}

FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot, TelemetryLog& log)
{
    FlightStats stats;
    bool launched = false;
    double nextPrint = 0.0;
    double apogeeX = 0.0;
    const long maxSteps = static_cast<long>(settings.maxTime / settings.dt) + 1;

    // Hard tier: a fixed array of polymorphic forces. Created here, deleted
    // at the end; week 10 makes the ownership automatic.
    Force* forces[3] = {new Gravity(), new Thrust(), new Drag(settings.wind)};

    plot.startSeries(rocket.name());

    for (long stepCount = 0; stepCount < maxSteps && rocket.time() < settings.maxTime; ++stepCount)
    {
        if (settings.printEvery > 0.0 && rocket.time() >= nextPrint)
        {
            std::cout << rocket << '\n';
            nextPrint += settings.printEvery;
        }

        Vec2 total;
        for (const Force* force : forces)
        {
            total += force->apply(rocket);
        }
        rocket.update(total, settings.dt);

        plot.addPoint(rocket.position().x, rocket.position().y, rocket.time());
        const double speed = length(rocket.velocity());
        log.append(Sample{rocket.time(), rocket.position().x, rocket.position().y, speed, rocket.fuel()});

        // Medium tier: throttle rule. Solid motors ignore it (base-class default).
        rocket.engine().setThrottle(speed > 300.0 ? 0.6 : 1.0);

        if (rocket.position().y > stats.apogee)
        {
            stats.apogee = rocket.position().y;
            stats.apogeeTime = rocket.time();
            apogeeX = rocket.position().x;
        }
        if (speed > stats.maxSpeed)
        {
            stats.maxSpeed = speed;
        }
        if (stats.burnoutTime == 0.0 && !rocket.engineRunning())
        {
            stats.burnoutTime = rocket.time();
            plot.addMarker(rocket.position().x, rocket.position().y, "burnout");
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
    plot.addMarker(apogeeX, stats.apogee, "apogee");
    stats.flightTime = rocket.time();
    stats.range = rocket.position().x;
    return stats;
}
