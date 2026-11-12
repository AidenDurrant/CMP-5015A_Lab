// rocket_spec.h -- the plain numbers that describe a rocket design, as read
// from a launch file. The Rocket class is built from one of these. Keeping
// the "data on disk" separate from the "object in flight" is what makes
// serialisation easy in week 11.

#pragma once

#include <string>

struct RocketSpec
{
    std::string name = "Starter-1";
    double dryMass = 50.0;
    double fuelMass = 150.0;
    double thrust = 5000.0;
    double burnRate = 10.0;
    double dragCoeff = 0.5;
    double area = 0.05;
    double launchAngleDeg = 85.0;
};
