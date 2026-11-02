// config.h -- reading a launch file (see launch.txt in the project root).
//
// WEEK 7 (Hard): implement loadLaunchFile in config.cpp using std::ifstream,
// std::getline and the std::string functions from lecture A (find, substr).
// WEEK 11 replaces this hand-written parser with >> and exceptions.

#pragma once

#include <string>

#include "rocket.h"
#include "simulation.h"

// Fill `rocket` and `settings` from the key = value file at `path`.
// Returns false (and leaves the arguments untouched) if the file cannot be
// opened. Unknown keys should be reported on std::cerr and skipped.
bool loadLaunchFile(const std::string& path, Rocket& rocket, SimSettings& settings);
