// physics.h -- the forces acting on the rocket and the time-step update.
//
// WEEK 7 (Easy): implement every function marked TODO in physics.cpp.
// The declarations below are the interface; do not change them this week.
// All of them take the Rocket by reference: `const Rocket&` when the function
// only reads it, plain `Rocket&` when it changes it. Compare with the C version
// in lecture 4, where you would have written `Rocket*` and `->`.

#pragma once

#include "rocket.h"
#include "vec2.h"

namespace physics
{
    // Constants. `constexpr` is the C++ way to write lecture 6's #define.
    constexpr double GRAVITY = 9.81;             // m/s^2
    constexpr double SEA_LEVEL_DENSITY = 1.225;  // kg/m^3
    constexpr double SCALE_HEIGHT = 8500.0;      // m, atmosphere thins with e^(-h/H)

    // Air density at a given altitude (m). GIVEN.
    double airDensity(double altitude);

    // Dry mass plus remaining fuel.
    double totalMass(const Rocket& rocket);

    // True while the engine is producing thrust.
    bool engineRunning(const Rocket& rocket);

    // Force of gravity on the rocket (N). Always points straight down.
    Vec2 gravityForce(const Rocket& rocket);

    // Engine thrust (N). Zero when the fuel has run out. Points along the
    // launch angle (Week 7 Medium changes this to follow the velocity).
    Vec2 thrustForce(const Rocket& rocket);

    // Aerodynamic drag (N). Opposes the velocity; zero when the rocket is still.
    //     |F| = 0.5 * rho(altitude) * Cd * A * |v|^2
    Vec2 dragForce(const Rocket& rocket);

    // Advance the rocket by one time step of dt seconds using Euler's method:
    //   1. sum the forces, 2. a = F / m, 3. v += a * dt, 4. p += v * dt,
    //   5. burn fuel, 6. advance time.
    void step(Rocket& rocket, double dt);
}
