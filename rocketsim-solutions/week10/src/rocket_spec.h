// rocket_spec.h -- the plain numbers describing a rocket design.

#pragma once

#include <string>
#include <vector>

struct Stage
{
    double dryMass = 50.0;
    double fuelMass = 150.0;
    double thrust = 5000.0;
    double burnRate = 10.0;
};

struct RocketSpec
{
    std::string name = "Starter-1";
    std::string engineType = "solid";    // Week 10: "solid" or "liquid", see makeEngine
    double payloadMass = 0.0;
    std::vector<Stage> stages{Stage{}};
    double dragCoeff = 0.5;
    double area = 0.05;
    double launchAngleDeg = 85.0;
};
