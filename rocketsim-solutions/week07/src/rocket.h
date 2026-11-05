// rocket.h -- everything the simulator knows about one rocket.
//
// Week 7: a plain struct, like the C Fraction struct in lecture 7. Every field is
//         public and the physics functions in physics.h operate on it directly.
// Week 8: you will turn this into a class with a constructor, private data and
//         const member functions.

#pragma once

#include <string>

#include "vec2.h"

struct Rocket
{
    // --- Design parameters (fixed for the whole flight) ---------------------
    std::string name = "Starter-1";
    double dryMass = 50.0;          // kg: structure + payload, never changes
    double thrust = 5000.0;         // N:  engine force while fuel remains
    double burnRate = 10.0;         // kg/s: how fast fuel is consumed
    double dragCoeff = 0.5;         // Cd, dimensionless
    double area = 0.05;             // m^2 frontal area used for drag
    double launchAngleDeg = 85.0;   // degrees from horizontal, 90 = straight up

    // --- State (changes every time step) ------------------------------------
    double fuelMass = 150.0;        // kg: remaining fuel
    Vec2 position;                  // m:   x = downrange, y = altitude
    Vec2 velocity;                  // m/s
    double time = 0.0;              // s:   mission elapsed time
};
