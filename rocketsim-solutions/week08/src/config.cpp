// config.cpp -- Week 7 (Hard) parser, now filling a RocketSpec. Week 11
// replaces the string surgery with >> and exceptions.

#include "config.h"

#include <fstream>
#include <iostream>

namespace
{
    std::string trim(const std::string& text)
    {
        const std::string whitespace = " \t\r\n";
        const std::size_t first = text.find_first_not_of(whitespace);
        if (first == std::string::npos)
        {
            return "";
        }
        const std::size_t last = text.find_last_not_of(whitespace);
        return text.substr(first, last - first + 1);
    }
}

bool loadLaunchFile(const std::string& path, RocketSpec& spec, SimSettings& settings)
{
    std::ifstream in(path);
    if (!in)
    {
        std::cerr << "Could not open launch file '" << path << "'\n";
        return false;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(in, line))
    {
        ++lineNumber;
        const std::size_t hash = line.find('#');
        if (hash != std::string::npos)
        {
            line = line.substr(0, hash);
        }
        line = trim(line);
        if (line.empty())
        {
            continue;
        }
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos)
        {
            std::cerr << path << ":" << lineNumber << ": expected key = value\n";
            continue;
        }
        const std::string key = trim(line.substr(0, equals));
        const std::string value = trim(line.substr(equals + 1));

        if (key == "name")
        {
            spec.name = value;
            continue;
        }
        const double number = std::stod(value);
        if (key == "dry_mass")          spec.dryMass = number;
        else if (key == "fuel_mass")    spec.fuelMass = number;
        else if (key == "thrust")       spec.thrust = number;
        else if (key == "burn_rate")    spec.burnRate = number;
        else if (key == "drag_coeff")   spec.dragCoeff = number;
        else if (key == "area")         spec.area = number;
        else if (key == "launch_angle") spec.launchAngleDeg = number;
        else if (key == "dt")           settings.dt = number;
        else if (key == "max_time")     settings.maxTime = number;
        else if (key == "wind")         settings.wind = Vec2{number, 0.0};
        else
        {
            std::cerr << path << ":" << lineNumber << ": unknown key '" << key << "'\n";
        }
    }
    return true;
}
