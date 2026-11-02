// physics.cpp -- WEEK 7 (Easy): fill in every TODO.
//
// Each stub currently does nothing useful, which is why the starter program
// shows a rocket sitting on the launch pad. Work down the file in order; the
// simulation starts to move as soon as gravityForce, thrustForce and step work.

#include "physics.h"

#include <cmath>

namespace physics
{
    double airDensity(double altitude)
    {
        // GIVEN. Exponential atmosphere: density halves roughly every 5.9 km.
        if (altitude < 0.0)
        {
            altitude = 0.0;
        }
        return SEA_LEVEL_DENSITY * std::exp(-altitude / SCALE_HEIGHT);
    }

    double totalMass(const Rocket& rocket)
    {
        // TODO (Week 7, Easy): dry mass + remaining fuel.
        (void)rocket;
        return 1.0;
    }

    bool engineRunning(const Rocket& rocket)
    {
        // TODO (Week 7, Easy): true while there is fuel left to burn.
        (void)rocket;
        return false;
    }

    Vec2 gravityForce(const Rocket& rocket)
    {
        // TODO (Week 7, Easy): weight = m * g, pointing in -y.
        (void)rocket;
        return Vec2{};
    }

    Vec2 thrustForce(const Rocket& rocket)
    {
        // TODO (Week 7, Easy): if the engine is running, `thrust` newtons along
        // the launch angle (see unitFromAngle in vec2.h); otherwise zero.
        (void)rocket;
        return Vec2{};
    }

    Vec2 dragForce(const Rocket& rocket)
    {
        // TODO (Week 7, Easy): 0.5 * rho * Cd * A * |v|^2, directed against v.
        (void)rocket;
        return Vec2{};
    }

    void step(Rocket& rocket, double dt)
    {
        // TODO (Week 7, Easy): Euler integration.
        //   Vec2 force = gravity + thrust + drag
        //   Vec2 acceleration = force / mass
        //   velocity += acceleration * dt
        //   position += velocity * dt
        //   if the engine is running, fuel -= burnRate * dt (never below zero)
        //   time += dt
        (void)rocket;
        (void)dt;
    }
}
