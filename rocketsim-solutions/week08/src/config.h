// config.h -- reading a launch file into a RocketSpec and SimSettings.

#pragma once

#include <string>

#include "rocket_spec.h"
#include "simulation.h"

// Fill `spec` and `settings` from the key = value file at `path`. Returns
// false (and leaves the arguments untouched) if the file cannot be opened.
bool loadLaunchFile(const std::string& path, RocketSpec& spec, SimSettings& settings);
