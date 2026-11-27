// config.cpp -- Week 9: known keys live in a std::set; "stage" lines build
// the stage list. Still the week 7 string-surgery parser underneath.

#include "config.h"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

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

    const std::set<std::string> KNOWN_KEYS = {"name",     "dry_mass",  "fuel_mass",    "thrust",
                                              "burn_rate", "drag_coeff", "area",        "launch_angle",
                                              "dt",       "max_time",  "payload_mass", "stage",
                                              "parachute", "engine"};
}

bool loadLaunchFile(const std::string& path, RocketSpec& spec, SimSettings& settings)
{
    std::ifstream in(path);
    if (!in)
    {
        std::cerr << "Could not open launch file '" << path << "'\n";
        return false;
    }

    std::vector<Stage> stages;   // filled by "stage =" lines, if any
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

        if (KNOWN_KEYS.count(key) == 0)
        {
            std::cerr << path << ":" << lineNumber << ": unknown key '" << key << "'\n";
            continue;
        }
        if (key == "name")
        {
            spec.name = value;
        }
        else if (key == "stage")
        {
            std::istringstream fields(value);
            Stage stage;
            fields >> stage.dryMass >> stage.fuelMass >> stage.thrust >> stage.burnRate;
            stages.push_back(stage);
        }
        else if (key == "engine")
        {
            spec.engineType = value;
        }
        else if (key == "parachute")
        {
            settings.parachute = (value == "yes" || value == "1" || value == "true");
        }
        else
        {
            const double number = std::stod(value);
            Stage& single = spec.stages.front();   // the legacy single-stage keys
            if (key == "dry_mass")          single.dryMass = number;
            else if (key == "fuel_mass")    single.fuelMass = number;
            else if (key == "thrust")       single.thrust = number;
            else if (key == "burn_rate")    single.burnRate = number;
            else if (key == "drag_coeff")   spec.dragCoeff = number;
            else if (key == "area")         spec.area = number;
            else if (key == "launch_angle") spec.launchAngleDeg = number;
            else if (key == "payload_mass") spec.payloadMass = number;
            else if (key == "dt")           settings.dt = number;
            else if (key == "max_time")     settings.maxTime = number;
        }
    }
    if (!stages.empty())
    {
        spec.stages = stages;
    }
    return true;
}
