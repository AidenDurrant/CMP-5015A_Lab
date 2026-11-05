// physics.cpp -- Week 7 model solution (Easy + Medium gravity turn).

#include "physics.h"

#include <cmath>

namespace physics
{
    double airDensity(double altitude)
    {
        if (altitude < 0.0)
        {
            altitude = 0.0;
        }
        return SEA_LEVEL_DENSITY * std::exp(-altitude / SCALE_HEIGHT);
    }

    double totalMass(const Rocket& rocket)
    {
        return rocket.dryMass + rocket.fuelMass;
    }

    bool engineRunning(const Rocket& rocket)
    {
        return rocket.fuelMass > 0.0;
    }

    Vec2 gravityForce(const Rocket& rocket)
    {
        Vec2 force;
        force.y = -totalMass(rocket) * GRAVITY;
        return force;
    }

    Vec2 thrustForce(const Rocket& rocket)
    {
        if (!engineRunning(rocket))
        {
            return Vec2{};
        }
        // Medium: gravity turn -- once moving, thrust along the velocity.
        Vec2 direction = unitFromAngle(rocket.launchAngleDeg);
        if (rocket.position.y > 300.0)
        {
            direction = normalised(rocket.velocity);
        }
        return scale(direction, rocket.thrust);
    }

    Vec2 dragForce(const Rocket& rocket)
    {
        const double speed = length(rocket.velocity);
        if (speed == 0.0)
        {
            return Vec2{};
        }
        const double rho = airDensity(rocket.position.y);
        const double magnitude = 0.5 * rho * rocket.dragCoeff * rocket.area * speed * speed;
        return scale(normalised(rocket.velocity), -magnitude);
    }

    void step(Rocket& rocket, double dt)
    {
        Vec2 force = add(gravityForce(rocket), thrustForce(rocket));
        force = add(force, dragForce(rocket));

        const Vec2 acceleration = scale(force, 1.0 / totalMass(rocket));
        rocket.velocity = add(rocket.velocity, scale(acceleration, dt));
        rocket.position = add(rocket.position, scale(rocket.velocity, dt));

        if (engineRunning(rocket))
        {
            rocket.fuelMass -= rocket.burnRate * dt;
            if (rocket.fuelMass < 0.0)
            {
                rocket.fuelMass = 0.0;
            }
        }
        rocket.time += dt;
    }
}
