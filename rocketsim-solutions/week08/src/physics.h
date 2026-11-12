// physics.h -- constants and the atmosphere model. The week 7 force functions
// have moved into the Force classes (force.h).

#pragma once

namespace physics
{
    constexpr double GRAVITY = 9.81;             // m/s^2
    constexpr double SEA_LEVEL_DENSITY = 1.225;  // kg/m^3
    constexpr double SCALE_HEIGHT = 8500.0;      // m
    constexpr double PITCH_OVER_ALTITUDE = 300.0; // m, gravity turn starts here

    double airDensity(double altitude);
}
